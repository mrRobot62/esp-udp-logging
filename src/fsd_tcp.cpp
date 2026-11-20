#include "log_transport_config.h"
#include "tcp/fsd_tcp.h"

#if defined(WIFI_LOGGING_USE_TCP) && defined(WIFI_LOGGING_ENABLE) && WIFI_LOGGING_ENABLE == 1
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <atomic>
#include <errno.h>
#include <lwip/sockets.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "wifi_secrets.h"
#include "tcp/log_tcp_core.h"
#include "udp/fsd_udp.h"
#if AUTO_SERVER_CONNECT
#include "tcp/server_discovery.h"
#include "tcp/discovery_protocol.h"
#endif

namespace tcp_logging {
namespace {
enum class State : uint8_t { stopped, starting, wifi, connecting, online, backoff, stopping, udp_fallback };
std::atomic<State> state{State::stopped};
std::atomic<bool> stop_requested{false};
std::atomic<bool> accepting{false};
std::atomic<uint32_t> session{0};
std::atomic<uint32_t> dropped_offline{0}, dropped_full{0}, dropped_long{0};
std::atomic<uint32_t> discarded{0}, sent{0}, failures{0};
QueueHandle_t queue = nullptr;
char role_name[32] = "UNK";
constexpr uint32_t kConnectMs = 1000;

// Sleep in short slices so shutdown also interrupts the reconnect backoff.
bool wait_interruptible(uint32_t ms) {
    const uint32_t start = millis();
    while (!stop_requested.load() && static_cast<uint32_t>(millis() - start) < ms) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return !stop_requested.load();
}

// Reject unspecified, multicast, limited broadcast and the local subnet broadcast.
bool valid_target(const IPAddress &ip) {
    if (ip.type() != IPv4 || ip[0] == 0 || ip[0] >= 224) return false;
    const IPAddress local = WiFi.localIP();
    const IPAddress mask = WiFi.subnetMask();
    bool broadcast = true;
    for (size_t i = 0; i < 4; ++i) {
        if (ip[i] != static_cast<uint8_t>(local[i] | static_cast<uint8_t>(~mask[i]))) {
            broadcast = false;
        }
    }
    return !broadcast;
}

// Inspect FIN/reset directly; connected() alone can miss orderly peer shutdown.
bool peer_alive(WiFiClient &client) {
    uint8_t byte;
    const int result = ::recv(client.fd(), &byte, 1, MSG_PEEK | MSG_DONTWAIT);
    if (result > 0) {
        // The protocol defines no inbound application messages; consume bounded data.
        uint8_t ignored[64];
        return ::recv(client.fd(), ignored, sizeof(ignored), MSG_DONTWAIT) > 0;
    }
    return result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
}

// Drop records of the old connection; session tags also catch racing producers.
void disconnect(WiFiClient &client) {
    accepting.store(false);
    session.fetch_add(1);
    client.stop();
    Message ignored;
    while (xQueueReceive(queue, &ignored, 0) == pdTRUE) ++discarded;
}

// Send only on this socket, with a strict application-level deadline and no replay.
bool transmit(WiFiClient &client, const Message &message) {
    PendingWrite pending;
    pending.start(message.size, millis());
    while (pending.remaining()) {
        if (stop_requested.load() || WiFi.status() != WL_CONNECTED ||
            pending.expired(millis()) || !peer_alive(client)) return false;
        const int n = ::send(client.fd(), message.bytes + pending.offset(),
                             pending.remaining(), MSG_DONTWAIT);
        if (n > 0) {
            if (!pending.advance(static_cast<size_t>(n))) return false;
        } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            return false;
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    return true;
}

// Keep fallback sends in the same worker, preserving queue bounds and task isolation.
void run_udp_fallback() {
    Serial.println("[TCP] attempt limit reached; switching to configured UDP target until logging restart");
    while (!stop_requested.load()) {
        if (WiFi.status() != WL_CONNECTED) {
            state.store(State::wifi);
            if (!wait_interruptible(1000)) break;
            continue;
        }
        if (!udp::begin(role_name)) {
            if (!wait_interruptible(1000)) break;
            continue;
        }
        const uint32_t current_session = session.fetch_add(1) + 1;
        state.store(State::udp_fallback);
        accepting.store(true);
        while (!stop_requested.load() && WiFi.status() == WL_CONNECTED) {
            Message message;
            if (xQueueReceive(queue, &message, pdMS_TO_TICKS(20)) == pdTRUE) {
                if (message.session != current_session) ++discarded;
                else if (udp::send_bytes(message.bytes, message.size)) ++sent;
                else { ++discarded; ++failures; break; }
            }
        }
        accepting.store(false);
        session.fetch_add(1);
        Message ignored;
        while (xQueueReceive(queue, &ignored, 0) == pdTRUE) ++discarded;
        udp::end();
        if (!stop_requested.load()) wait_interruptible(1000);
    }
    accepting.store(false);
    udp::end();
}

// Own all WiFi initialization, DNS, TCP state and socket I/O outside the caller task.
void run_worker() {
    WiFiClient client;
    Backoff retry;
    unsigned failed_attempts = 0;
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);
    if (WiFi.status() != WL_CONNECTED) WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (!stop_requested.load()) {
        if (WiFi.status() != WL_CONNECTED) {
            state.store(State::wifi);
            if (!wait_interruptible(1000)) break;
            continue;
        }
        state.store(State::connecting);
        IPAddress ip;
        uint16_t target_port = 0;
#if AUTO_SERVER_CONNECT
        const bool resolved = tcp_discovery::find_server(ip, target_port, stop_requested);
#else
        // DNS may take longer than the TCP connect timeout, but never blocks loop().
        target_port = WIFI_LOGGING_TCP_PORT;
        const bool resolved = ip.fromString(WIFI_LOGGING_TCP_HOST) ||
                              WiFi.hostByName(WIFI_LOGGING_TCP_HOST, ip) == 1;
#endif
        bool connected = false;
        if (!stop_requested.load() && resolved && valid_target(ip)) {
            connected = client.connect(ip, target_port, kConnectMs) == 1;
        }
        if (connected && !stop_requested.load()) {
            client.setNoDelay(true);
            const uint32_t current_session = session.fetch_add(1) + 1;
            const uint32_t connected_at = millis();
            state.store(State::online);
            accepting.store(true);
            Serial.printf("[TCP][%s] connected to %u.%u.%u.%u:%u\n", role_name,
                          ip[0], ip[1], ip[2], ip[3], static_cast<unsigned>(target_port));
            while (!stop_requested.load() && WiFi.status() == WL_CONNECTED && peer_alive(client)) {
                Message message;
                if (xQueueReceive(queue, &message, pdMS_TO_TICKS(20)) == pdTRUE) {
                    if (message.session != current_session) {
                        ++discarded;
                    } else if (!transmit(client, message)) {
                        ++discarded;
                        break;
                    } else {
                        ++sent; // Accepted by the local socket, not acknowledged by the viewer.
                    }
                }
                if (static_cast<uint32_t>(millis() - connected_at) >= 10000) {
                    retry.reset();
                    failed_attempts = 0;
                }
            }
        }
        disconnect(client);
        if (stop_requested.load()) break;
        ++failures;
        if (++failed_attempts >= TCP_CONNECT_MAX_ATTEMPTS) {
            run_udp_fallback();
            break;
        }
        state.store(State::backoff);
        const uint32_t delay_ms = retry.next();
        Serial.printf("[TCP] unavailable/disconnected; retry in %lu ms\n",
                      static_cast<unsigned long>(delay_ms));
        if (!wait_interruptible(delay_ms)) break;
    }
    disconnect(client);
}

// Let C++ release worker-local resources before FreeRTOS deletes the task.
void worker(void *) {
    run_worker();
    state.store(State::stopped);
    vTaskDelete(nullptr);
}
}

bool begin(const char *role) {
    State expected = State::stopped;
    if (!state.compare_exchange_strong(expected, State::starting)) {
        return !stop_requested.load();
    }
#if AUTO_SERVER_CONNECT
    const bool target_configured = !WIFI_LOGGING_SERVER_ID[0] || tcp_discovery::valid_id(WIFI_LOGGING_SERVER_ID);
#else
    const bool target_configured = WIFI_LOGGING_TCP_HOST[0] != '\0';
#endif
    if (WIFI_SSID[0] == '\0' || !target_configured) {
        Serial.println("[TCP] missing WiFi SSID or invalid TCP host/server ID");
        state.store(State::stopped);
        return false;
    }
    // Allocate once; retaining this bounded queue makes concurrent producers safe at stop.
    if (!queue) queue = xQueueCreate(kQueueDepth, sizeof(Message));
    if (!queue) {
        Serial.println("[TCP] queue allocation failed");
        state.store(State::stopped);
        return false;
    }
    snprintf(role_name, sizeof(role_name), "%s", role && *role ? role : "UNK");
    stop_requested.store(false);
    if (xTaskCreate(worker, "tcp-log", 6144, nullptr, 1, nullptr) != pdPASS) {
        Serial.println("[TCP] worker allocation failed");
        state.store(State::stopped);
        return false;
    }
    return true;
}

void end() {
    accepting.store(false);
    stop_requested.store(true);
}

bool send_bytes(const uint8_t *data, size_t len) {
    if (!data || !len) return false;
    if (len > kPayloadLimit) { ++dropped_long; return false; }
    const uint32_t current_session = session.load();
    if (!accepting.load() || stop_requested.load()) { ++dropped_offline; return false; }
    Message message;
    if (!frame(data, len, message)) return false;
    message.session = current_session;
    if (xQueueSend(queue, &message, 0) != pdTRUE) { ++dropped_full; return false; }
    return true;
}

void note_oversize() { ++dropped_long; }

void diag_print() {
    static const char *names[] = {"stopped", "starting", "waiting-wifi", "connecting",
                                  "connected", "backoff", "stopping", "udp-fallback"};
    const State current = state.load();
    const char *label = stop_requested.load() && current != State::stopped ?
                        "stopping" : names[static_cast<unsigned>(current)];
    Serial.printf("[TCP] state=%s socket_sent=%lu offline=%lu full=%lu oversize=%lu discarded=%lu failures=%lu\n",
                  label, static_cast<unsigned long>(sent.load()),
                  static_cast<unsigned long>(dropped_offline.load()),
                  static_cast<unsigned long>(dropped_full.load()),
                  static_cast<unsigned long>(dropped_long.load()),
                  static_cast<unsigned long>(discarded.load()),
                  static_cast<unsigned long>(failures.load()));
}
}
#else
namespace tcp_logging {
bool begin(const char *) { return false; }
void end() {}
bool send_bytes(const uint8_t *, size_t) { return false; }
void note_oversize() {}
void diag_print() {}
}
#endif
