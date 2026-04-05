#include <Arduino.h>

#include "log_core.h"
#include "log_client.h"
#include "log_csv.h"
#include "udp/fsd_udp.h"

namespace {

uint32_t last_log_ms = 0;
uint32_t counter = 0;
bool door_open = false;
bool heater_on = false;
bool fan12_on = true;
bool fan230_on = false;
bool fan230_slow_on = false;
bool motor_on = false;
bool lamp_on = true;

int diag_state_to_int() {
    return heater_on ? 2 : 1;
}

const char *diag_state_to_str() {
    return heater_on ? "HEATING" : "IDLE";
}

void emit_demo_logs(uint32_t now_ms) {
    const int raw_hot = 12450 + static_cast<int>((counter % 7U) * 32U);
    const long hot_mV = 1460L + static_cast<long>((counter % 9U) * 11U);
    const long hot_ohm = 41200L - static_cast<long>((counter % 6U) * 180U);
    const long hot_dC = 612L + static_cast<long>((counter % 10U) * 3U);

    const int raw_ch = 10120 + static_cast<int>((counter % 11U) * 27U);
    const long cha_mV = 1320L + static_cast<long>((counter % 8U) * 9U);
    const long cha_ohm = 8350L - static_cast<long>((counter % 5U) * 75U);
    const long cha_dC = 438L + static_cast<long>((counter % 12U) * 4U);

    const float temp_hotspot_c = hot_dC / 10.0f;
    const float temp_chamber_c = cha_dC / 10.0f;

    CSV_LOG_CLIENT_LOGIC(
        fan12_on ? 1 : 0,
        fan230_on ? 1 : 0,
        fan230_slow_on ? 1 : 0,
        motor_on ? 1 : 0,
        heater_on ? 1 : 0,
        lamp_on ? 1 : 0,
        door_open ? 1 : 0,
        static_cast<unsigned>(diag_state_to_int()));

    CSV_LOG_CLIENT_TEMP(
        static_cast<long>(raw_hot),
        hot_mV,
        hot_dC,
        static_cast<long>(raw_ch),
        cha_mV,
        cha_dC,
        diag_state_to_int(),
        heater_on ? 1L : 0L,
        door_open ? 1L : 0L);

    CLIENT_INFO(
        "[DIAG] Door=%s state=%s rawHot=%d hot_mV=%ld hot_ohm=%ld Thot=%.2fC "
        "rawCh=%d cha_mV=%ld cha_ohm=%ld Tch=%.2fC heater=%s\n",
        door_open ? "OPEN" : "CLOSED",
        diag_state_to_str(),
        raw_hot,
        hot_mV,
        hot_ohm,
        temp_hotspot_c,
        raw_ch,
        cha_mV,
        cha_ohm,
        temp_chamber_c,
        heater_on ? "ON" : "OFF");

    CLIENT_DBG("[CLIENTCOMM][T14] STATUS demo tick=%lu uptime_ms=%lu free_heap=%lu\n",
               static_cast<unsigned long>(counter),
               static_cast<unsigned long>(now_ms),
               static_cast<unsigned long>(ESP.getFreeHeap()));

    if (door_open) {
        CLIENT_WARN("[DOOR] state=%s\n", "OPEN");
    }

    if (temp_hotspot_c >= 63.0f) {
        CLIENT_ERR("[SAFETY] ABS OVER-TEMP! cur=%.1f >= %.1f -> HEATER OFF\n",
                   temp_hotspot_c,
                   63.0f);
    }
}

}

void setup() {
    Serial.begin(115200);
    delay(500);

    INFO("Booting esp-udp-logging example\n");
    CLIENT_INFO("----------------------------------------------\n");
    CLIENT_INFO("- ESP32 UDP logging demo with CSV output    ---\n");
    CLIENT_INFO("----------------------------------------------\n");
    CLIENT_INFO("This demo is made to test Serial + UDP output together\n");
    CLIENT_INFO("Use it with udp-viewer on your desktop side\n");

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

        heater_on = ((counter % 4U) == 1U) || ((counter % 4U) == 2U);
        door_open = ((counter % 9U) == 0U);
        fan230_on = heater_on;
        fan230_slow_on = !heater_on && ((counter % 3U) == 0U);
        motor_on = ((counter % 6U) == 0U);
        lamp_on = !door_open;

        INFO("Structured info log, counter=%lu uptime_ms=%lu state=%s\n",
             static_cast<unsigned long>(counter),
             static_cast<unsigned long>(now),
             diag_state_to_str());
        DBG("Debug log example, free_heap=%lu door=%s heater=%s\n",
            static_cast<unsigned long>(ESP.getFreeHeap()),
            door_open ? "OPEN" : "CLOSED",
            heater_on ? "ON" : "OFF");

        emit_demo_logs(now);

        if ((counter % 5U) == 0U) {
            WARN("Periodic warning example, counter=%lu\n", static_cast<unsigned long>(counter));
        }
    }
}
