// Compile the production implementation against deterministic platform substitutes.
#include "host_stubs/runtime.h"
#include <assert.h>
#include <thread>
#include <set>
#include "../src/fsd_tcp.cpp"
#if AUTO_SERVER_CONNECT
#include "../src/server_discovery.cpp"
#endif
#include "../src/log_transport.cpp"
#include "log_core.h"

SerialStub Serial;
WiFiStub WiFi;
namespace fallback_test { unsigned starts = 0; std::string wire; }
namespace udp {
bool begin(const char *) { ++fallback_test::starts; return true; }
void end() {}
bool send_bytes(const uint8_t *data, size_t size) {
    fallback_test::wire.append(reinterpret_cast<const char *>(data), size);
    return true;
}
}
namespace fake {
uint32_t now = 0;
bool wifi = true, peer = true, connect_ok = true;
unsigned connects = 0;
std::deque<int> writes;
std::string wire;
std::function<void()> on_delay;
void (*task)(void *) = nullptr;
}

// Reset the deterministic environment between independent scenarios.
static void reset() {
    using namespace tcp_logging;
    if (queue) { delete queue; queue = nullptr; }
    state.store(State::stopped);
    stop_requested.store(false);
    accepting.store(false);
    session.store(0);
    dropped_offline = 0; dropped_full = 0; dropped_long = 0;
    discarded = 0; sent = 0; failures = 0;
    fallback_test::starts = 0; fallback_test::wire.clear();
    fake::now = 0; fake::connects = 0;
    fake::wifi = true; fake::peer = true; fake::connect_ok = true;
    fake::writes.clear(); fake::wire.clear(); fake::on_delay = nullptr;
    Serial.output.clear();
#if AUTO_SERVER_CONNECT
    discovery_fake::respond = true; discovery_fake::ambiguous = false; discovery_fake::requests = 0;
#endif
}

// Exercise production queue admission, drop counters and atomic complete-call copies.
static void queue_and_concurrency() {
    using namespace tcp_logging;
    reset();
    const uint8_t line[] = "offline";
    assert(!send_bytes(line, sizeof(line)-1));
    assert(dropped_offline == 1);
    assert(begin("TEST"));
    accepting = true;
    session = 4;
    std::vector<std::thread> threads;
    for (unsigned t = 0; t < 4; ++t) {
        threads.emplace_back([t]() {
            for (unsigned i = 0; i < 4; ++i) {
                const std::string text = std::to_string(t) + ":" + std::to_string(i);
                assert(tcp_logging::send_bytes(reinterpret_cast<const uint8_t *>(text.data()), text.size()));
            }
        });
    }
    for (auto &thread : threads) thread.join();
    assert(!send_bytes(line, sizeof(line)-1));
    assert(dropped_full == 1);
    Message m;
    std::set<std::string> records;
    while (xQueueReceive(queue, &m, 0)) {
        assert(m.session == 4);
        records.emplace(reinterpret_cast<const char *>(m.bytes), m.size);
    }
    assert(records.size() == 16);
    for (unsigned t = 0; t < 4; ++t)
        for (unsigned i = 0; i < 4; ++i)
            assert(records.count(std::to_string(t) + ":" + std::to_string(i) + "\n"));
    end();
    assert(!send_bytes(line, sizeof(line)-1));
    assert(!begin("TOO-EARLY"));
    fake::task(nullptr);
    assert(state == State::stopped);
    assert(begin("RESTART"));
    end();
    fake::task(nullptr);
}

// Exercise real send-loop decisions with short writes, FIN, EPIPE and backpressure.
static void socket_failures() {
    using namespace tcp_logging;
    reset();
    WiFiClient client;
    client.connect(IPAddress(), 10514, 1000);
    Message m{};
    const uint8_t utf8[] = {'A', 0xc3, 0xbc, 'B'};
    assert(frame(utf8, sizeof(utf8), m));
    fake::writes = {2, -EAGAIN, 1, 2};
    assert(transmit(client, m));
    assert(fake::wire == std::string(reinterpret_cast<const char *>(m.bytes), m.size));
    fake::wire.clear();
    fake::writes = {2, -EPIPE};
    assert(!transmit(client, m));
    assert(fake::wire.size() == 2);
    fake::wire.clear();
    assert(transmit(client, m)); // A fresh call starts at zero, never at the old offset.
    assert(fake::wire.size() == m.size);
    fake::writes.assign(300, -EAGAIN);
    const uint32_t started = fake::now;
    assert(!transmit(client, m));
    assert(fake::now - started == kSendDeadlineMs);
    fake::writes.clear();
    fake::peer = false;
    assert(!transmit(client, m));
    fake::peer = true;
    fake::wifi = false;
    assert(!transmit(client, m));
}

// Verify formatter overflow cannot turn into a malformed TCP line.
static void formatting() {
    using namespace tcp_logging;
    reset();
    assert(begin("FORMAT"));
    accepting = true;
    const std::string oversized(600, 'x');
    RAW("%s", oversized.c_str());
    assert(dropped_long == 1);
    assert(Serial.output.size() == 511);
    Message m;
    assert(!xQueueReceive(queue, &m, 0));
    INFO("hello\r\n");
    assert(xQueueReceive(queue, &m, 0));
    assert(std::string(reinterpret_cast<char *>(m.bytes), m.size) == "[MAIN/INFO] hello\n");
    end();
    fake::task(nullptr);
}

// Exercise the full production worker through peer loss, reconnect and clean stop.
static void worker_reconnect() {
    using namespace tcp_logging;
    reset();
    assert(begin("WORKER"));
    unsigned phase = 0;
    fake::on_delay = [&]() {
        assert(fake::now < 20000);
        if (state == State::online && phase == 0) {
            const uint8_t old[] = "old\n";
            assert(send_bytes(old, sizeof(old)-1));
            fake::writes = {2, -EPIPE};
            phase = 1;
        } else if (state == State::online && fake::connects >= 2 && phase == 1) {
            const uint8_t fresh[] = "new\n";
            assert(send_bytes(fresh, sizeof(fresh)-1));
            phase = 2;
        } else if (sent == 1) {
            end();
        }
    };
    fake::task(nullptr);
    assert(state == State::stopped && fake::connects == 2);
    assert(discarded == 1 && sent == 1);
    assert(fake::wire == "olnew\n"); // Two sessions: old tail was never retransmitted.
    assert(!accepting);
}

// Ensure unavailable servers use backoff rather than a busy retry loop.
static void refused_server() {
    using namespace tcp_logging;
    reset();
    fake::connect_ok = false;
    assert(begin("REFUSED"));
    fake::on_delay = [&]() {
        assert(fake::now < 10000);
        if (fake::connects == 3) end();
    };
    fake::task(nullptr);
    assert(fake::connects == 3 && fake::now >= 3000);
    assert(state == State::stopped);
}

// Exhaust the configured attempt limit and keep subsequent logs on the UDP worker.
static void udp_fallback() {
    using namespace tcp_logging;
    reset();
    fake::connect_ok = false;
    assert(begin("FALLBACK"));
    bool queued = false;
    fake::on_delay = [&]() {
        assert(fake::now < 200000);
        if (state == State::udp_fallback && !queued) {
            const uint8_t text[] = "fallback";
            assert(send_bytes(text, sizeof(text)-1));
            queued = true;
        } else if (!fallback_test::wire.empty()) end();
    };
    fake::task(nullptr);
    assert(fake::connects == TCP_CONNECT_MAX_ATTEMPTS);
    assert(fallback_test::starts == 1);
    assert(fallback_test::wire == "fallback\n");
    assert(state == State::stopped);
}

#if AUTO_SERVER_CONNECT
// Both missing servers and ambiguous replies exhaust the same bounded retry budget.
static void discovery_failure_fallback(bool ambiguous) {
    using namespace tcp_logging;
    reset();
    discovery_fake::respond = ambiguous;
    discovery_fake::ambiguous = ambiguous;
    assert(begin("DISCOVERY-FAILURE"));
    fake::on_delay = [&]() {
        assert(fake::now < 200000);
        if (state == State::udp_fallback) end();
    };
    fake::task(nullptr);
    assert(fake::connects == 0);
    assert(discovery_fake::requests == 2 * TCP_CONNECT_MAX_ATTEMPTS);
    assert(fallback_test::starts == 1);
}
#endif

int main() {
    queue_and_concurrency();
    socket_failures();
    formatting();
    worker_reconnect();
    refused_server();
    udp_fallback();
#if AUTO_SERVER_CONNECT
    discovery_failure_fallback(false);
    discovery_failure_fallback(true);
#endif
    reset();
    puts("TCP runtime: 6 scenario groups passed (simulated platform/network)");
}
