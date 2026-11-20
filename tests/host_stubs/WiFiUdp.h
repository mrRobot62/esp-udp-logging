#pragma once
#include "runtime.h"
namespace discovery_fake {
static bool respond = true;
static bool ambiguous = false;
static unsigned requests = 0;
}
class WiFiUDP {
    std::string request;
    std::deque<std::string> replies;
public:
    int begin(uint16_t) { return 1; }
    int beginPacket(IPAddress, uint16_t) { request.clear(); return 1; }
    size_t write(const uint8_t *data, size_t size) {
        request.append(reinterpret_cast<const char *>(data), size);
        return size;
    }
    int endPacket() {
        ++discovery_fake::requests;
        if (discovery_fake::respond) {
            const std::string nonce = request.substr(15, 16);
            replies.push_back("ULV_SERVER 1 " + nonce + " viewer-test 10514\n");
            if (discovery_fake::ambiguous)
                replies.push_back("ULV_SERVER 1 " + nonce + " other-viewer 10514\n");
        }
        return 1;
    }
    int parsePacket() const { return replies.empty() ? 0 : replies.front().size(); }
    IPAddress remoteIP() const { return IPAddress(192,168,0,249); }
    int read(uint8_t *data, size_t size) {
        if (replies.empty()) return 0;
        const size_t n = size < replies.front().size() ? size : replies.front().size();
        memcpy(data, replies.front().data(), n);
        return static_cast<int>(n);
    }
    void clear() { if (!replies.empty()) replies.pop_front(); }
    void stop() {}
};
