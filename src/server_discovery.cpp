#include "log_transport_config.h"
#if AUTO_SERVER_CONNECT && defined(WIFI_LOGGING_USE_TCP) && defined(WIFI_LOGGING_ENABLE) && WIFI_LOGGING_ENABLE == 1
#include "tcp/server_discovery.h"
#include "tcp/discovery_protocol.h"
#include <WiFiUdp.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace tcp_discovery {
bool find_server(IPAddress &ip, uint16_t &port, const std::atomic<bool> &stop) {
    WiFiUDP udp;
    if (!udp.begin(0)) {
        Serial.println("[TCP discovery] UDP socket unavailable");
        return false;
    }
    char nonce[17];
    snprintf(nonce, sizeof(nonce), "%08lx%08lx",
             static_cast<unsigned long>(esp_random()), static_cast<unsigned long>(esp_random()));
    char request[64];
    const int size = snprintf(request, sizeof(request), "ULV_DISCOVER 1 %s\n", nonce);
    const IPAddress local = WiFi.localIP(), mask = WiFi.subnetMask();
    IPAddress broadcast(local[0] | static_cast<uint8_t>(~mask[0]),
                        local[1] | static_cast<uint8_t>(~mask[1]),
                        local[2] | static_cast<uint8_t>(~mask[2]),
                        local[3] | static_cast<uint8_t>(~mask[3]));
    Selection selection;
    const uint32_t start = millis();
    unsigned requests = 0;
    while (!stop.load() && WiFi.status() == WL_CONNECTED &&
           static_cast<uint32_t>(millis() - start) < 700) {
        const uint32_t elapsed = static_cast<uint32_t>(millis() - start);
        if (requests == 0 || (requests == 1 && elapsed >= 350)) {
            if (udp.beginPacket(broadcast, WIFI_LOGGING_DISCOVERY_PORT)) {
                udp.write(reinterpret_cast<const uint8_t *>(request), size);
                udp.endPacket();
            }
            ++requests;
        }
        const int received = udp.parsePacket();
        if (received > 0) {
            char data[kDatagramLimit + 1];
            const IPAddress source = udp.remoteIP();
            const int count = udp.read(reinterpret_cast<uint8_t *>(data), sizeof(data));
            if (received <= static_cast<int>(kDatagramLimit) && count == received) {
                const uint32_t address = (uint32_t(source[0]) << 24) | (uint32_t(source[1]) << 16) |
                                         (uint32_t(source[2]) << 8) | source[3];
                selection.consider(data, count, address, nonce, WIFI_LOGGING_SERVER_ID);
            }
            udp.clear();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    udp.stop();
    if (stop.load() || WiFi.status() != WL_CONNECTED) return false;
    if (!selection.ready()) {
        Serial.println(selection.ambiguous ? "[TCP discovery] multiple servers; set WIFI_LOGGING_SERVER_ID" :
                                             "[TCP discovery] no matching server found");
        return false;
    }
    ip = IPAddress(selection.ip >> 24, selection.ip >> 16, selection.ip >> 8, selection.ip);
    port = selection.port;
    Serial.printf("[TCP discovery] selected server %s\n", selection.id);
    return true;
}
}
#endif
