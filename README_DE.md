# esp-udp-logging

Wenn du das UDP-Logging direkt mit einem Desktop-Viewer testen willst, schau dir am besten zuerst das passende Begleitprojekt an:

[udp-viewer](https://github.com/mrRobot62/udp-viewer.git)

Genau dafuer ist dieses Demo hier auch gedacht: Du flashst das Projekt auf einen ESP32, der ESP32 sendet UDP-Logpakete raus, und im `udp-viewer` kannst du das Ergebnis direkt anschauen.

Wenn du einfach ein kleines ESP32-Beispiel suchst, mit dem du strukturiert auf `Serial` loggen und dieselben Meldungen optional per UDP ueber WiFi rausschicken kannst, dann ist dieses Repo genau dafuer gedacht.

Ich habe das Projekt bewusst klein gehalten. Die Basis kommt aus meinem groesseren `FilamentSilicatDryer_480x480` Projekt, aber hier ist alles so weit reduziert, dass man das Prinzip schnell versteht, ohne sich erst durch die komplette Anwendungslogik zu arbeiten.

Das Beispiel zeigt dir im Kern zwei Sachen:

- wie du mit Makros wie `INFO(...)`, `WARN(...)`, `ERR(...)` sauber loggen kannst
- wie dieselben Logzeilen parallel als UDP-Pakete verschickt werden koennen

Heisst also ganz praktisch: Die Ausgabe landet im seriellen Monitor und, wenn WiFi klappt, auch noch bei deinem UDP-Empfaenger.

## Was im Repo drin ist

Das Projekt ist absichtlich ueberschaubar.

- `src/main.cpp`
  Das eigentliche Mini-Beispiel. Hier wird `Serial` gestartet, UDP initialisiert und es werden im Loop ein paar Demo-Logs erzeugt.

- `include/log_core.h`
  Das ist der zentrale Logging-Baustein. Hier sind `INFO`, `DBG`, `WARN`, `ERR` definiert.

- `include/udp/fsd_udp.h`
  Das kleine Interface fuer den UDP-Teil.

- `src/fsd_udp.cpp`
  Hier steckt die WiFi- und UDP-Implementierung drin. Also Verbindung aufbauen, UDP vorbereiten und Daten verschicken.

- `include/wifi_secrets.h.example`
  Vorlage fuer deine lokalen WLAN-Zugangsdaten.

- `platformio.ini`
  PlatformIO-Konfiguration mit Ziel-IP, Ziel-Port und dem Schalter fuer WiFi-Logging.

## Wie die Logs aussehen

Die Logzeilen haben einen Prefix, damit du direkt siehst, was fuer eine Meldung das ist.

Beispiel:

```text
[MAIN/INFO] Booting esp-udp-logging example
[MAIN/INFO] UDP logging ready
[MAIN/DEBUG] Debug log example, free_heap=231456
[MAIN/WARN] Periodic warning example, counter=5
```

Das soll kein riesiges Logging-Framework sein. Es ist eher eine einfache, praktische Loesung fuer Embedded-Debugging.

## Wie das Ganze funktioniert

Der Ablauf ist ziemlich simpel:

1. `Serial.begin(115200)` startet die serielle Ausgabe.
2. `udp::begin("CLIENT")` versucht, das WLAN aufzubauen und UDP vorzubereiten.
3. `INFO(...)`, `WARN(...)`, `DBG(...)` formatieren die Zeile genau einmal.
4. Die Zeile geht auf `Serial`.
5. Falls UDP aktiv ist und WiFi steht, geht dieselbe Zeile auch noch als UDP-Paket raus.

Du brauchst also nicht getrennt fuer Serial und Netzwerk zu loggen. Ein Log-Aufruf reicht.

## Einrichtung

Bevor du das Projekt baust, mach einmal kurz Folgendes:

1. `include/wifi_secrets.h.example` nach `include/wifi_secrets.h` kopieren
2. WLAN-Name und Passwort eintragen
3. `platformio.ini` oeffnen
4. Ziel-IP fuer deinen UDP-Empfaenger anpassen
5. Falls noetig den UDP-Port anpassen

Die lokale `wifi_secrets.h` ist absichtlich in `.gitignore`, damit echte Zugangsdaten nicht versehentlich im Repo landen.

## Beispiel fuer `wifi_secrets.h`

```cpp
#pragma once

#define WIFI_SSID "DEIN_WIFI_NAME"
#define WIFI_PASS "DEIN_WIFI_PASSWORT"
```

## Wichtige Stellen in der `platformio.ini`

Dort findest du diese Eintraege:

```ini
-DWIFI_LOGGING_ENABLE=1
-DWIFI_LOGGING_UDP_IP="192.168.0.249"
-DWIFI_LOGGING_UDP_PORT=10514
```

Kurz gesagt:

- `WIFI_LOGGING_ENABLE=1`
  WiFi- bzw. UDP-Logging wird mit einkompiliert.

- `WIFI_LOGGING_UDP_IP`
  Das ist die IP von deinem Rechner oder deinem Logging-Server.

- `WIFI_LOGGING_UDP_PORT`
  Das ist der UDP-Port, auf dem dein Empfaenger lauscht.

Ein wichtiges Detail noch: In [`src/fsd_udp.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/fsd_udp.cpp) ist ebenfalls eine Default-IP und ein Default-Port hinterlegt.

Wenn du also gar nichts an den Build-Flags aenderst, gibt es trotzdem einen eingebauten Fallback. Im normalen Einsatz werden diese Default-Werte aber ueber die Compiler-Direktiven aus der `platformio.ini` ueberschrieben.

Wenn du WiFi-Logging abschaltest, funktioniert das serielle Logging trotzdem weiter. Das macht das Beispiel etwas flexibler.

## Build und Upload

Die normalen PlatformIO-Kommandos sind:

```bash
pio run
pio run -t upload
pio device monitor
```

Wenn dein Board angeschlossen ist und der Port passt, reicht das im Normalfall schon.

## Wie du die UDP-Logs empfaengst

Du kannst jeden beliebigen UDP-Empfaenger verwenden.

Zum schnellen Testen auf dem Rechner gehen zum Beispiel `nc`, `ncat`, `socat`, Wireshark oder auch ein kleines eigenes Python-Skript.

Ein ganz einfacher Start waere zum Beispiel:

```bash
nc -ul 10514
```

Je nach System brauchst du vielleicht `ncat` statt `nc`, aber die Idee bleibt gleich: einfach einen UDP-Listener auf dem konfigurierten Port starten.

## Warum es dieses Repo gibt

Man will nicht jedes Mal auf ein grosses Produktivprojekt verweisen, nur um kurz zu zeigen, wie Logging oder UDP-Mirroring funktioniert.

Dieses Repo ist genau dafuer gedacht. Du kannst es weitergeben, wenn dich jemand fragt:

- "Wie machst du strukturiertes Logging auf dem ESP32?"
- "Wie spiegelst du Serial-Logs auf UDP?"
- "Hast du ein kleines WiFi-Logging-Beispiel?"

Genau dafuer ist das hier da.

## Noch ein paar Hinweise

- Das Projekt will kein vollwertiges Library-Paket sein.
- Es ist in erster Linie ein kleines Beispielprojekt.
- Der Stil ist absichtlich nah am Ursprungsprojekt gehalten, damit der Uebergang spaeter leicht faellt.
- Der WLAN-Verbindungsaufbau blockiert beim Start kurz. Fuer ein Demo ist das voellig okay.
- Wenn kein WiFi da ist, laeuft das Beispiel trotzdem weiter und `Serial`-Logging bleibt nutzbar.

## Direkt zu den relevanten Dateien

- [`src/main.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/main.cpp)
- [`include/log_core.h`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/include/log_core.h)
- [`include/udp/fsd_udp.h`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/include/udp/fsd_udp.h)
- [`src/fsd_udp.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/fsd_udp.cpp)
- [`platformio.ini`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/platformio.ini)

## English version

Die englische Version findest du in [`README.md`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/README.md).
