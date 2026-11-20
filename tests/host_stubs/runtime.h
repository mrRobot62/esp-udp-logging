#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace fake {
extern uint32_t now;
extern bool wifi, peer, connect_ok;
extern unsigned connects;
extern std::deque<int> writes;
extern std::string wire;
extern std::function<void()> on_delay;
extern void (*task)(void *);
}

inline uint32_t millis() { return fake::now; }
struct SerialStub {
    std::string output;
    template <typename... Args> void printf(const char *, Args...) {}
    void println(const char *) {}
    size_t write(const uint8_t *p, size_t n) {
        output.append(reinterpret_cast<const char *>(p), n);
        return n;
    }
};
extern SerialStub Serial;
constexpr int IPv4 = 0, WIFI_STA = 1, WL_CONNECTED = 3;
class IPAddress {
    uint8_t bytes_[4] = {192, 168, 0, 249};
public:
    IPAddress() = default;
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : bytes_{a,b,c,d} {}
    int type() const { return IPv4; }
    uint8_t operator[](size_t i) const { return bytes_[i]; }
    bool fromString(const char *) { return true; }
};
struct WiFiStub {
    int status() const { return fake::wifi ? WL_CONNECTED : 0; }
    void mode(int) {}
    void setAutoReconnect(bool) {}
    void setSleep(bool) {}
    void begin(const char *, const char *) {}
    int hostByName(const char *, IPAddress &) { return 1; }
    IPAddress localIP() const { return IPAddress(192,168,0,42); }
    IPAddress subnetMask() const { return IPAddress(255,255,255,0); }
};
extern WiFiStub WiFi;
class WiFiClient {
    int fd_ = -1;
public:
    int fd() const { return fd_; }
    int connect(IPAddress, uint16_t, int32_t) {
        ++fake::connects;
        fake::peer = fake::connect_ok;
        fd_ = fake::connect_ok ? 7 : -1;
        return fake::connect_ok;
    }
    void stop() { fd_ = -1; }
    void setNoDelay(bool) {}
};

constexpr int pdTRUE = 1, pdPASS = 1;
using TickType_t = uint32_t;
inline TickType_t pdMS_TO_TICKS(uint32_t ms) { return ms; }
inline void vTaskDelay(TickType_t ticks) {
    fake::now += ticks;
    if (fake::on_delay) fake::on_delay();
}
inline void vTaskDelete(void *) {}
inline int xTaskCreate(void (*task)(void *), const char *, unsigned, void *, int, void *) {
    fake::task = task;
    return pdPASS;
}
struct HostQueue {
    size_t capacity, item_size;
    std::mutex mutex;
    std::deque<std::vector<uint8_t>> items;
    HostQueue(size_t n, size_t size) : capacity(n), item_size(size) {}
};
using QueueHandle_t = HostQueue *;
inline QueueHandle_t xQueueCreate(size_t count, size_t size) { return new HostQueue(count,size); }
inline int xQueueSend(QueueHandle_t q, const void *item, TickType_t) {
    std::lock_guard<std::mutex> lock(q->mutex);
    if (q->items.size() == q->capacity) return 0;
    const auto p = static_cast<const uint8_t *>(item);
    q->items.emplace_back(p, p + q->item_size);
    return pdTRUE;
}
inline int xQueueReceive(QueueHandle_t q, void *item, TickType_t wait) {
    {
        std::lock_guard<std::mutex> lock(q->mutex);
        if (!q->items.empty()) {
            memcpy(item, q->items.front().data(), q->item_size);
            q->items.pop_front();
            return pdTRUE;
        }
    }
    if (wait) vTaskDelay(wait);
    return 0;
}

constexpr int MSG_PEEK = 1, MSG_DONTWAIT = 2;
inline int recv(int fd, void *, size_t, int) {
    if (fd < 0) { errno = EBADF; return -1; }
    if (!fake::peer) return 0;
    errno = EAGAIN;
    return -1;
}
inline int send(int fd, const void *data, size_t size, int) {
    if (fd < 0) { errno = EBADF; return -1; }
    int n = static_cast<int>(size);
    if (!fake::writes.empty()) {
        n = fake::writes.front();
        fake::writes.pop_front();
    }
    if (n < 0) { errno = -n; return -1; }
    if (static_cast<size_t>(n) > size) n = static_cast<int>(size);
    fake::wire.append(static_cast<const char *>(data), n);
    return n;
}
