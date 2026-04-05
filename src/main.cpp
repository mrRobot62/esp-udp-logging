#include <Arduino.h>

#include "log_core.h"
#include "udp/fsd_udp.h"

namespace {

uint32_t last_log_ms = 0;
uint32_t counter = 0;

}

void setup() {
    Serial.begin(115200);
    delay(500);

    INFO("Booting esp-udp-logging example\n");

    const bool udp_ok = udp::begin("CLIENT");
    if (udp_ok) {
        INFO("UDP logging ready\n");
    } else {
        WARN("UDP logging disabled or unavailable\n");
    }

    udp::diag_print();
}

void loop() {
    const uint32_t now = millis();
    if ((now - last_log_ms) >= 2000U) {
        last_log_ms = now;
        ++counter;

        INFO("Structured info log, counter=%lu uptime_ms=%lu\n",
             static_cast<unsigned long>(counter),
             static_cast<unsigned long>(now));
        DBG("Debug log example, free_heap=%lu\n", static_cast<unsigned long>(ESP.getFreeHeap()));

        if ((counter % 5U) == 0U) {
            WARN("Periodic warning example, counter=%lu\n", static_cast<unsigned long>(counter));
        }
    }
}
