# esp-udp-logging

Kleines PlatformIO-Beispiel fuer ESP32, das zwei Dinge zeigt:

- strukturiertes Logging auf `Serial`
- optionales Spiegeln derselben Logzeilen per UDP ueber WiFi

## Inhalt

- `src/main.cpp`: minimales Beispiel mit `INFO`, `DBG`, `WARN`
- `include/log_core.h`: printf-basierte Logging-Makros
- `include/udp/fsd_udp.h` und `src/fsd_udp.cpp`: UDP/WiFi-Helfer
- `include/wifi_secrets.h.example`: Vorlage fuer lokale WLAN-Zugangsdaten

## Einrichtung

1. `include/wifi_secrets.h.example` nach `include/wifi_secrets.h` kopieren.
2. SSID und Passwort eintragen.
3. In [`platformio.ini`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/platformio.ini) Ziel-IP und UDP-Port anpassen.
4. Flashen und seriellen Monitor starten.

## Beispiel-Logs

`INFO(...)`, `DBG(...)`, `WARN(...)` schreiben auf `Serial`. Wenn `udp::begin("CLIENT")` erfolgreich ist und `WIFI_LOGGING_ENABLE=1` gesetzt ist, gehen dieselben Zeilen auch als UDP-Pakete an den konfigurierten Server.

## Build

```bash
pio run
pio run -t upload
pio device monitor
```
