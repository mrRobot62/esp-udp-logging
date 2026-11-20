#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace tcp_logging {
constexpr size_t kPayloadLimit = 511;
constexpr size_t kQueueDepth = 16;
constexpr uint32_t kSendDeadlineMs = 2000;

// Fixed-size queue item: one logging call, with its final LF and session identity.
struct Message {
    uint32_t session;
    uint16_t size;
    uint8_t bytes[kPayloadLimit + 1];
};

// Preserve interior newlines and UTF-8 bytes; normalize trailing LF/CRLF to one LF.
inline bool frame(const uint8_t *data, size_t len, Message &out) {
    if (!data || len == 0 || len > kPayloadLimit) {
        return false;
    }
    while (len && (data[len - 1] == '\n' || data[len - 1] == '\r')) {
        --len;
    }
    memcpy(out.bytes, data, len);
    out.bytes[len] = '\n';
    out.size = static_cast<uint16_t>(len + 1);
    return true;
}

// Track partial writes within one connection; a new record always resets the offset.
class PendingWrite {
public:
    // Start a bounded send attempt, using rollover-safe elapsed time comparisons.
    void start(size_t size, uint32_t now) {
        size_ = size;
        offset_ = 0;
        started_ = now;
    }
    // Accept only a valid number of bytes actually handed to the socket.
    bool advance(size_t count) {
        if (count > remaining()) return false;
        offset_ += count;
        return true;
    }
    // Return the next byte position within the current record.
    size_t offset() const { return offset_; }
    // Return the number of bytes still owed to this connection.
    size_t remaining() const { return size_ - offset_; }
    // Test the absolute send deadline even if the peer makes slow progress.
    bool expired(uint32_t now) const {
        return static_cast<uint32_t>(now - started_) >= kSendDeadlineMs;
    }
private:
    size_t size_ = 0;
    size_t offset_ = 0;
    uint32_t started_ = 0;
};

// Bounded reconnect delay. Reset only after a stable connection, not every accept.
class Backoff {
public:
    // Return the current delay and increase the next one up to ten seconds.
    uint32_t next() {
        const uint32_t result = delay_;
        delay_ = delay_ < 5000 ? delay_ * 2 : 10000;
        return result;
    }
    // Restart the retry sequence after sustained connectivity.
    void reset() { delay_ = 1000; }
private:
    uint32_t delay_ = 1000;
};
}
