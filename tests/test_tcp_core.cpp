#include "tcp/log_tcp_core.h"
#include <assert.h>
#include <string.h>

// Check framing without requiring an ESP32 or network connection.
static void framing() {
    using namespace tcp_logging;
    Message m{};
    const char *inputs[] = {"hello", "hello\n", "hello\r\n", "hello\n\n"};
    for (const char *s : inputs) {
        assert(frame(reinterpret_cast<const uint8_t *>(s), strlen(s), m));
        assert(m.size == 6 && memcmp(m.bytes, "hello\n", 6) == 0);
    }
    const char utf8[] = "Gr\xc3\xbc\xc3\x9f" "e\nsecond\r\n";
    assert(frame(reinterpret_cast<const uint8_t *>(utf8), strlen(utf8), m));
    const char expected[] = "Gr\xc3\xbc\xc3\x9f" "e\nsecond\n";
    assert(m.size == strlen(expected));
    assert(memcmp(m.bytes, expected, m.size) == 0);
    uint8_t maximum[kPayloadLimit + 1];
    memset(maximum, 'a', sizeof(maximum));
    assert(frame(maximum, kPayloadLimit, m));
    assert(m.size == kPayloadLimit + 1 && m.bytes[kPayloadLimit] == '\n');
    assert(!frame(maximum, sizeof(maximum), m));
    assert(!frame(nullptr, 1, m));
    assert(!frame(maximum, 0, m));
}

// Verify short writes, no-progress timeout, clock rollover and fresh-session offset.
static void partial_writes() {
    using namespace tcp_logging;
    PendingWrite p;
    p.start(7, 100);
    assert(p.advance(2) && p.offset() == 2 && p.remaining() == 5);
    assert(p.advance(0) && p.remaining() == 5);
    assert(!p.advance(6));
    assert(!p.expired(2099) && p.expired(2100));
    p.start(4, 0xfffffff0U);
    assert(p.offset() == 0 && p.remaining() == 4);
    assert(!p.expired(0x10U));
    assert(p.expired(0x7c0U));
    assert(p.advance(1) && p.advance(3) && p.remaining() == 0);
}

// Verify exponential retry limits and recovery after a stable connection.
static void reconnect() {
    tcp_logging::Backoff b;
    const unsigned expected[] = {1000, 2000, 4000, 8000, 10000, 10000};
    for (unsigned delay : expected) assert(b.next() == delay);
    b.reset();
    assert(b.next() == 1000);
}

// Run deterministic host-side checks; hardware and integration tests remain separate.
int main() {
    framing();
    partial_writes();
    reconnect();
}
