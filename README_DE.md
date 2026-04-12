# esp-udp-logging

Wenn du das UDP-Logging direkt mit einem Desktop-Viewer testen willst, schau dir zuerst das Begleitprojekt an:

[udp-viewer](https://github.com/mrRobot62/udp-viewer.git)

**Genau dafuer ist dieses Demo-Repo gedacht: Du flashst es auf einen ESP32, laesst den Controller UDP-Logpakete senden und kannst das Ergebnis direkt im `udp-viewer` pruefen.**

Wenn du einfach nur ein kleines ESP32-Beispiel suchst fuer strukturiertes Logging auf `Serial` und optionales UDP-Logging ueber WiFi, dann passt dieses Projekt ziemlich gut.

Ich habe das Repo bewusst kompakt gehalten. Die Basis kommt aus meinem groesseren `FilamentSilicatDryer_480x480` Projekt, aber hier ist alles so weit abgespeckt, dass man das Prinzip schnell versteht, ohne sich erst durch eine komplette Anwendung zu wuehlen.

Dieses Beispiel zeigt im Wesentlichen zwei Dinge:

- wie du strukturierte Logmeldungen wie `INFO(...)`, `WARN(...)` oder `ERR(...)` schreiben kannst
- wie dieselben Logzeilen auch per UDP ueber WiFi verschickt werden koennen

Heisst praktisch: Die Ausgabe landet im seriellen Monitor und, wenn WiFi verfuegbar ist, zusaetzlich bei deinem UDP-Empfaenger.

## Warum es dieses Repo gibt

Man will nicht immer auf ein grosses Produktivprojekt verweisen, nur um ein kleines Thema zu erklaeren.

Dieses Repo ist als kleine Referenz gedacht, die du weitergeben kannst, wenn jemand fragt:

- "Wie machst du strukturiertes Logging auf einem ESP32?"
- "Wie spiegelst du Serial-Logs auf UDP?"
- "Hast du ein minimales WiFi-Logging-Beispiel?"
- "Hast du ein kleines CSV-Logging-Beispiel fuer den udp-viewer?"

Genau dafuer ist das hier da.

## Hinweise

- Das Projekt will kein vollwertiges, wiederverwendbares Library-Paket sein.
- Es ist in erster Linie ein kleines Demo-Projekt.
- Der Code ist absichtlich nah am Stil des Ursprungsprojekts gehalten, damit der Uebergang leicht faellt.
- Der WiFi-Verbindungsaufbau blockiert beim Start kurz. Fuer ein Demo ist das voellig okay.

## Was in diesem Projekt drin ist

Das Projekt ist absichtlich klein und nicht mehr als ein Beispiel-Demo.

- `src/main.cpp`
  Das ist die Beispielanwendung. Sie startet `Serial`, initialisiert UDP-Logging und erzeugt im Loop ein paar Demo-Logmeldungen.

- `include/log_core.h`
  Das ist der zentrale Logging-Helfer. Hier sind Makros wie `INFO`, `DBG`, `WARN` und `ERR` definiert.

- `include/udp/fsd_udp.h`
  Kleines oeffentliches Header-Interface fuer den UDP-Helfer.

- `src/fsd_udp.cpp`
  Die WiFi- und UDP-Implementierung. Sie verbindet den ESP32 mit deinem WLAN und verschickt Logzeilen als UDP-Pakete.

- `include/wifi_secrets.h.example`
  Eine Vorlage fuer deine lokalen WiFi-Zugangsdaten.

- `platformio.ini`
  PlatformIO-Konfiguration mit Ziel-IP, Ziel-Port und dem Compile-Flag fuer aktiviertes WiFi-Logging. Inzwischen gibt es dort ein Environment fuer klassischen ESP32 und eines fuer ESP32-S3.

## Wie das Logging aussieht

Die Logs bekommen einen Prefix, damit du direkt siehst, was fuer eine Meldung es ist.

**Beispiel:**

```text
[MAIN/INFO] Booting esp-udp-logging example
[MAIN/INFO] UDP logging ready
[MAIN/DEBUG] Debug log example, free_heap=231456
[MAIN/WARN] Periodic warning example, counter=5
```

- `[xxxxx]` - hier kannst du selbst festlegen, welche Information du an der Stelle haben willst
- `[MAIN/INFO]` - dieser Text laesst sich im UDP-LOG-VIEWER auch gut als Filter verwenden

Das Ganze soll kein riesiges Logging-Framework sein. Es ist eher eine einfache und praktische Loesung fuer Embedded-Debugging.

## CSV-Logging Beispiele

Die Demo in [`src/main.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/main.cpp) sendet nicht nur normale Text-Logs. Sie schickt auch zwei CSV-artige Logzeilen raus, die du fuer Plotting oder fuer Zustandsanzeigen im `udp-viewer` verwenden kannst.

- Format: `[xxxxx]` ist dein Filter-Kriterium fuer den UDP-LOG-VIEWER, die einzelnen Werte werden per Semikolon getrennt
- `[CSV_CLIENT_PLOT]` : Beispiel fuer interessante Messwerte aus deiner Client-Software
- `[CSV_HOST_PLOT]` : Beispiel fuer interessante Messwerte aus deiner Host-Software
- `[CSV_LONG_TERM_PLOT]` : Beispiel fuer Werte, die du fuer Langzeitanalyse loggen willst
- `[feel_free]` : die Namen der Filter kannst du natuerlich frei gestalten :-)

### Beispiel 1: Temperatur-CSV

Diese Zeile wird mit `CSV_LOG_CLIENT_TEMP(...)` erzeugt. In dieser Demo repraesentieren die Werte Messungen von einem Client-ESP32, der seine Daten per UDP sendet.

<img width="1582" height="658" alt="udp_log_viewer_plot_measure1" src="https://github.com/user-attachments/assets/1238b9ae-ddcc-4558-8290-7f842e4d69fd" />

```text
[CSV_CLIENT_PLOT];12482;1471;615;10147;1329;442;2;1;0
```

Bedeutung der Werte:

- `12482` = Hotspot Raw-ADC-Wert
- `1471` = Hotspot in Millivolt
- `615` = Hotspot-Temperatur in Zehntel-Grad C, also `61.5C`
- `10147` = Chamber Raw-ADC-Wert
- `1329` = Chamber in Millivolt
- `442` = Chamber-Temperatur in Zehntel-Grad C, also `44.2C`
- `2` = Demo-State, zum Beispiel `HEATING`
- `1` = Heater an
- `0` = Door geschlossen

Dieses Format ist an das Temperatur-CSV-Logging angelehnt, das ich auch im groesseren Dryer-Projekt verwende.

### Beispiel 2: Logic-CSV

Diese Zeile wird mit `CSV_LOG_CLIENT_LOGIC(...)` erzeugt. In dieser Demo versuchen wir GPIO-Zustaende bzw. HIGH/LOW-Signale zu loggen und sie im UDP-LOG-VIEWER sichtbar zu machen.

<img width="1769" height="782" alt="udp_log_viewer_logic_signal2" src="https://github.com/user-attachments/assets/810f19b4-f61e-4a76-adb0-5886ad821c76" />

```text
[CSV_CLIENT_LOGIC];1;1;0;0;1;1;0;2
```

Bedeutung der Werte:

- `1` = fan12V an
- `1` = fan230V an
- `0` = fan230V slow aus
- `0` = Motor aus
- `1` = Heater an
- `1` = Lamp an
- `0` = Door geschlossen
- `2` = Demo-State als Integer

Mit der aktuellen Demo kannst du also beides gleichzeitig testen:

- klassisch lesbare Logtexte
- strukturierte CSV-Daten fuer Viewer oder Plotting

## UDP-LOG-VIEWER konfigurieren

Wenn du diese Demo zusammen mit dem UDP-LOG-VIEWER verwenden willst, solltest du die Graphen dort passend konfigurieren.

### Konfiguration PLOT-Visualizer

<img width="1372" height="860" alt="image" src="https://github.com/user-attachments/assets/0a014dca-baef-4f3f-ad80-e2f46e11d0d2" />

### Konfiguration LOGIC-Visualizer

<img width="1372" height="860" alt="image" src="https://github.com/user-attachments/assets/ed71a0da-6213-422e-b73c-e7ccb7067a82" />

## Wie es funktioniert

Der grundsaetzliche Ablauf ist dieser:

1. `Serial.begin(115200)` startet die serielle Ausgabe.
2. `udp::begin("CLIENT")` versucht, WiFi zu verbinden und UDP vorzubereiten.
3. `INFO(...)`, `WARN(...)`, `DBG(...)` formatieren die Zeile genau einmal.
4. Die Zeile wird auf `Serial` geschrieben.
5. Wenn UDP-Logging aktiviert ist und WiFi verbunden ist, wird dieselbe Zeile auch per UDP verschickt.

Du brauchst also keine getrennten Log-Aufrufe fuer Serial und Netzwerk. Ein Log-Aufruf reicht.

## Setup

Bevor du das Projekt baust, mach das einmal:

1. Kopiere `include/wifi_secrets.h.example` nach `include/wifi_secrets.h`
2. Trage deinen WiFi-Namen und dein Passwort ein
3. Oeffne `platformio.ini`
4. Passe die Ziel-IP fuer deinen UDP-Log-Empfaenger an
5. Passe bei Bedarf den UDP-Port an

Deine lokale `wifi_secrets.h` ist absichtlich per Git ignoriert, damit echte Zugangsdaten nicht im Repo landen.

## Beispiel `wifi_secrets.h`

```cpp
#pragma once

#define WIFI_SSID "DEIN_WIFI_NAME"
#define WIFI_PASS "DEIN_WIFI_PASSWORT"
```

## PlatformIO-Konfiguration

In `platformio.ini` findest du diese wichtigen Einstellungen:

```ini
-DWIFI_LOGGING_ENABLE=1
-DWIFI_LOGGING_UDP_IP="192.168.0.249"
-DWIFI_LOGGING_UDP_PORT=10514
```

Das bedeutet:

- `WIFI_LOGGING_ENABLE=1`
  WiFi- bzw. UDP-Logging wird mit einkompiliert.

- `WIFI_LOGGING_UDP_IP`
  Das ist die IP deines PCs oder deines Logging-Servers.

- `WIFI_LOGGING_UDP_PORT`
  Das ist der UDP-Port, auf dem dein Empfaenger lauscht.

Ein wichtiges Detail noch: In [`src/fsd_udp.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/fsd_udp.cpp) ist auch eine Default-Ziel-IP plus Port hinterlegt.

Wenn du also gar nichts machst, gibt es trotzdem einen eingebauten Fallback. Im normalen Einsatz werden diese Werte aber ueber die Compiler-Defines aus der `platformio.ini` ueberschrieben.

Wenn du WiFi-Logging deaktivierst, funktioniert das serielle Logging trotzdem weiter. Das macht die Demo flexibel.

## Build und Upload

Die ueblichen PlatformIO-Kommandos:

```bash
pio run
pio run -t upload
pio device monitor
```

Wenn dein Board angeschlossen ist und der Upload-Port stimmt, reicht das normalerweise schon.

Wenn du gezielt fuer ein bestimmtes Board bauen willst, nutze direkt den Environment-Namen:

```bash
pio run -e esp32dev
pio run -e esp32-s3-devkitc-1
```

Aktuelle Targets in diesem Repo:

- `esp32dev`
- `esp32-s3-devkitc-1`

## Wie du die UDP-Logs empfaengst

Du kannst jeden beliebigen UDP-Empfaenger verwenden.

Zum schnellen Testen am Rechner funktionieren z. B. `nc`, `ncat`, `socat`, Wireshark oder auch ein eigenes kleines Python-Skript. Oder du nutzt direkt meinen [udp-viewer](https://github.com/mrRobot62/udp-viewer.git), geschrieben in Python.

**Ein einfaches Beispiel mit netcat:**  
(nur um kurz zu pruefen, ob dein PC ueberhaupt UDP-Messages bekommt - fuer Windows habe ich gerade keinen guten Tipp :-/ )

```bash
nc -ul 10514
```

Je nach System brauchst du vielleicht `ncat` statt `nc`, aber die Idee ist dieselbe: einfach einen UDP-Listener auf dem Port aus `platformio.ini` starten.

# Bereit fuer deinen UDP-LOG-VIEWER

Wenn du Nachrichten auf deinem PC bekommst, ist es Zeit fuer den UDP-LOG-VIEWER.
Spiel ein bisschen mit deinem Embedded-System, veraendere Werte und schau dir an, wie sich das im UDP-LOG-VIEWER verhaelt.

Viel Spass damit .....

# Relevante Dateien

Wenn du direkt in die wichtigen Dateien schauen willst:

- [`src/main.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/main.cpp)
- [`include/log_core.h`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/include/log_core.h)
- [`include/udp/fsd_udp.h`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/include/udp/fsd_udp.h)
- [`src/fsd_udp.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/fsd_udp.cpp)
- [`platformio.ini`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/platformio.ini)
- [`CHANGELOG.md`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/CHANGELOG.md)

## Gib mir einen Stern

Wenn dir meine Projekte gefallen, gib ihnen gern einen Stern.

## Englische Version

Wenn du dieselbe Readme auf Englisch lesen willst, schau in [`README.md`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/README.md).
