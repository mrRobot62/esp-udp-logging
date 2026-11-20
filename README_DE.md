# esp-logging-demo — Serial-, UDP- und TCP-Logging

Version **0.2.0**. Repository: `esp-udp-logging`.

Wenn du das Netzwerk-Logging direkt mit einem Desktop-Viewer testen willst, schau dir zuerst das Begleitprojekt an:

[udp-viewer](https://github.com/mrRobot62/udp-viewer.git)

**Genau dafuer ist dieses Demo-Repo gedacht: Du flashst es auf einen ESP32, laesst den Controller UDP-Datagramme oder TCP-Logzeilen senden und kannst das Ergebnis direkt im `udp-viewer` pruefen.**

Wenn du einfach nur ein kleines ESP32-Beispiel suchst fuer strukturiertes Logging auf `Serial` und optionales UDP- oder TCP-Logging über WiFi, dann passt dieses Projekt ziemlich gut.

Ich habe das Repo bewusst kompakt gehalten. Die Basis kommt aus meinem groesseren `FilamentSilicatDryer_480x480` Projekt, aber hier ist alles so weit abgespeckt, dass man das Prinzip schnell versteht, ohne sich erst durch eine komplette Anwendung zu wuehlen.

Dieses Beispiel zeigt im Wesentlichen zwei Dinge:

- wie du strukturierte Logmeldungen wie `INFO(...)`, `WARN(...)` oder `ERR(...)` schreiben kannst
- wie dieselben Logging-Aufrufe UDP-Datagramme oder TCP-Zeilen über WiFi übertragen

Heisst praktisch: Die Ausgabe landet im seriellen Monitor und, wenn WiFi verfuegbar ist, zusätzlich bei deinem Netzwerk-Empfänger. UDP bleibt Standard; TCP wird vor dem Kompilieren ausgewählt.

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
- Bei UDP kann die WLAN-Initialisierung den Start bis zu acht Sekunden blockieren. TCP initialisiert und sendet in einem Hintergrund-Worker.
- Der ESP32 ist der TCP-Client, der Desktop-Viewer der TCP-Server.
- Bestehende `INFO`-, `DBG`-, `WARN`-, `ERR`-, `CLIENT_*`-, `RAW`- und CSV-Aufrufe funktionieren mit beiden Transportarten.
- Firmware-Builds und automatisierte Tests sind erfolgreich. Tests mit echter ESP32-/WLAN-Hardware sowie plattformspezifische Discovery-Prüfungen unter Windows stehen noch aus.

## Was in diesem Projekt drin ist

Das Projekt ist absichtlich klein und nicht mehr als ein Beispiel-Demo.

- `src/main.cpp`
  Das ist die Beispielanwendung. Sie startet `Serial`, initialisiert den ausgewählten Logging-Transport und erzeugt im Loop ein paar Demo-Logmeldungen.

- `include/log_core.h`
  Das ist der zentrale Logging-Helfer. Hier sind Makros wie `INFO`, `DBG`, `WARN` und `ERR` definiert.

- `include/udp/fsd_udp.h`
  Kleines oeffentliches Header-Interface fuer den UDP-Helfer.

- `src/fsd_udp.cpp`
  Die WiFi- und UDP-Implementierung. Sie verbindet den ESP32 mit deinem WLAN und verschickt Logzeilen als UDP-Pakete.

- `include/wifi_secrets.h.example`
  Eine Vorlage fuer deine lokalen WiFi-Zugangsdaten.

- `platformio.ini`
  Build-Environments für ESP32 und ESP32-S3, jeweils mit UDP, manuellem TCP und automatischer TCP-Serversuche.

- `include/log_transport.h` / `src/log_transport.cpp`
  Gemeinsame Transportschnittstelle mit Auswahl zur Compile-Zeit.

- `src/fsd_tcp.cpp`
  TCP-Worker, begrenzte Queue, Wiederverbindung und UDP-Fallback.

- `src/server_discovery.cpp` / `include/tcp/discovery_protocol.h`
  UDP-basierte Serversuche und Prüfung der Antworten.

## Wie das Logging aussieht

Die Logs bekommen einen Prefix, damit du direkt siehst, was fuer eine Meldung es ist.

**Beispiel:**

```text
[MAIN/INFO] Booting esp-logging-demo 0.2.0
[MAIN/DEBUG] Debug log example, free_heap=231456
[MAIN/WARN] Periodic warning example, counter=5
```

- `[xxxxx]` - hier kannst du selbst festlegen, welche Information du an der Stelle haben willst
- `[MAIN/INFO]` - dieser Text laesst sich im UDP-LOG-VIEWER auch gut als Filter verwenden

Das Ganze soll kein riesiges Logging-Framework sein. Es ist eher eine einfache und praktische Loesung fuer Embedded-Debugging.

## CSV-Logging Beispiele

Die Demo in [`src/main.cpp`](src/main.cpp) sendet nicht nur normale Text-Logs. Sie schickt auch zwei CSV-artige Logzeilen raus, die du fuer Plotting oder fuer Zustandsanzeigen im `udp-viewer` verwenden kannst.

- Format: `[xxxxx]` ist dein Filter-Kriterium fuer den UDP-LOG-VIEWER, die einzelnen Werte werden per Semikolon getrennt
- `[CSV_CLIENT_PLOT]` : Beispiel fuer interessante Messwerte aus deiner Client-Software
- `[CSV_HOST_PLOT]` : Beispiel fuer interessante Messwerte aus deiner Host-Software
- `[CSV_LONG_TERM_PLOT]` : Beispiel fuer Werte, die du fuer Langzeitanalyse loggen willst
- `[feel_free]` : die Namen der Filter kannst du natuerlich frei gestalten :-)

### Beispiel 1: Temperatur-CSV

Diese Zeile wird mit `CSV_LOG_CLIENT_TEMP(...)` erzeugt. In dieser Demo repraesentieren die Werte Messungen von einem Client-ESP32, der seine Daten per UDP oder TCP sendet.

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
2. `logging_transport::begin("CLIENT")` initialisiert den ausgewählten Transport. Bei TCP bedeutet Erfolg, dass der Worker gestartet wurde; die Verbindung zum Server entsteht anschließend asynchron.
3. `INFO(...)`, `WARN(...)`, `DBG(...)` formatieren die Zeile genau einmal.
4. Die Zeile wird auf `Serial` geschrieben.
5. Wenn Netzwerk-Logging aktiviert ist, übernimmt der gewählte Transport die Nachricht. TCP reiht vollständige Aufrufe nur bei bestehender Verbindung ein; frühe Boot- und Offline-Meldungen bleiben auf Serial verfügbar.

Du brauchst also keine getrennten Log-Aufrufe fuer Serial und Netzwerk. Ein Log-Aufruf reicht.

## Setup

Bevor du das Projekt baust, mach das einmal:

1. Kopiere `include/wifi_secrets.h.example` nach `include/wifi_secrets.h`
2. Trage deinen WiFi-Namen und dein Passwort ein
3. Oeffne `platformio.ini`
4. Wähle ein Environment für UDP, manuelles TCP oder automatische TCP-Serversuche.
5. Konfiguriere Empfängeradresse und Ports. Auch bei TCP muss das UDP-Ziel stimmen, da es beim Fallback verwendet wird.
6. Starte den passenden Empfänger im Viewer, bevor du den ESP32 flashst und startest.

Deine lokale `wifi_secrets.h` ist absichtlich per Git ignoriert, damit echte Zugangsdaten nicht im Repo landen.

## Beispiel `wifi_secrets.h`

```cpp
#pragma once

#define WIFI_SSID "DEIN_WIFI_NAME"
#define WIFI_PASS "DEIN_WIFI_PASSWORT"
```

## Vor dem Kompilieren zwischen UDP und TCP wechseln

Wähle das passende PlatformIO-Environment. Die Logging-Aufrufe im Anwendungscode
bleiben gleich.

| Board | UDP (Standard) | TCP mit festem Server | TCP mit automatischer Serversuche |
| --- | --- | --- | --- |
| ESP32 | `esp32dev` | `esp32dev-tcp` | `esp32dev-tcp-auto` |
| ESP32-S3 | `esp32-s3-devkitc-1` | `esp32-s3-devkitc-1-tcp` | `esp32-s3-devkitc-1-tcp-auto` |

In der PlatformIO-IDE: **Project Tasks → Environment → Build / Upload**.
Im Terminal wählst du das gewünschte Environment ausdrücklich aus:

```sh
# Klassischer ESP32: einen dieser Builds auswählen
pio run -e esp32dev
pio run -e esp32dev-tcp
pio run -e esp32dev-tcp-auto

# Beispiel: automatische TCP-Serversuche für ESP32-S3 bauen und flashen
pio run -e esp32-s3-devkitc-1-tcp-auto -t upload
pio device monitor -b 115200
```

Ein Wechsel des Environments oder seiner Flags wird nach erneutem Kompilieren
und Flashen wirksam. `pio run` ohne `-e` baut **alle sechs Varianten**. Beim Upload
immer `-e` angeben. Eine laufende TCP-Firmware kann über den unten beschriebenen
Fallback zu UDP wechseln; ein Menü zur Protokollauswahl auf dem ESP32 gibt es nicht.

### UDP

Im gemeinsamen Abschnitt `[env]` der `platformio.ini` steht das UDP-Ziel:

```ini
-DWIFI_LOGGING_ENABLE=1
-DWIFI_LOGGING_UDP_IP=\"192.168.0.249\"
-DWIFI_LOGGING_UDP_PORT=10514
```

Verwende `esp32dev` oder `esp32-s3-devkitc-1`. Ohne ausdrückliches Transport-Flag
wird UDP gewählt. Alternativ ist `-DWIFI_LOGGING_USE_UDP=1` möglich; dieses Flag darf
nicht gemeinsam mit `WIFI_LOGGING_USE_TCP` gesetzt werden. Ersetze die Beispiel-IP
durch die Adresse deines Empfängers. Fehlen die UDP-Zieloptionen, verwendet die
Implementierung `192.168.0.112:10514`.

### TCP mit konfiguriertem Server

Die Environments `*-tcp` ergänzen die gemeinsamen Einstellungen um:

```ini
-DWIFI_LOGGING_USE_TCP=1
-DAUTO_SERVER_CONNECT=0
-DWIFI_LOGGING_TCP_HOST=\"192.168.0.249\"
-DWIFI_LOGGING_TCP_PORT=10514
-DTCP_CONNECT_MAX_ATTEMPTS=10
```

Der ESP32 verbindet sich mit dieser IPv4-Adresse oder diesem Hostnamen. Als
TCP-Ziel sind `0.0.0.0` und Broadcast-Adressen ungeeignet. Fehlt
`AUTO_SERVER_CONNECT`, gilt `0`. Manuelles TCP benötigt `WIFI_LOGGING_TCP_HOST`.
Der TCP-Port ist in den Environments ausdrücklich angegeben; ohne Angabe bleibt
für bestehende Konfigurationen der Standard `10514` gültig.

Im Viewer **TCP** auswählen, lokale Bind-Adresse und TCP-Port eintragen und
**CONNECT** anklicken. `0.0.0.0` als Bind-Adresse des Viewers bedeutet „auf allen
lokalen Schnittstellen lauschen“. Der ESP32 benötigt trotzdem eine konkrete
Zieladresse.

### TCP mit automatischer Serversuche

Die Environments `*-tcp-auto` verwenden:

```ini
-DWIFI_LOGGING_USE_TCP=1
-DAUTO_SERVER_CONNECT=1
-DWIFI_LOGGING_DISCOVERY_PORT=10515
-DTCP_CONNECT_MAX_ATTEMPTS=10
```

Hier sind keine feste TCP-Adresse und kein fester TCP-Zielport erforderlich.
Im Viewer mit Discovery-Unterstützung **TCP** auswählen, **Auto server discovery**
aktivieren, denselben Discovery-UDP-Port einstellen und **CONNECT** anklicken.
Der TCP-Port des Viewers darf vom Discovery-Port abweichen; die Serverantwort
liefert den TCP-Port an den ESP32.

Die Suche erfolgt über **UDP-Broadcast**, die anschließende Logübertragung über
TCP. Der ESP32 sendet zwei Suchanfragen innerhalb eines Zeitfensters von 700 ms.
Antworten müssen zur Protokollversion und zur zufälligen Kennung der aktuellen
Anfrage passen. Die Absender-IP der Antwort wird zur TCP-Zieladresse. Ein Viewer
antwortet nur, wenn sein TCP-Listener läuft und noch kein Sender verbunden ist.

Antworten mehrere unterschiedliche passende Endpunkte, wird keiner beliebig
ausgewählt. Um einen bestimmten Viewer festzulegen, kopiere dessen **Server ID**
in die `build_flags` des ausgewählten Environments:

```ini
-DWIFI_LOGGING_SERVER_ID=\"hier-die-server-id-des-viewers-eintragen\"
```

Ohne dieses optionale Flag muss genau ein antwortender Endpunkt gefunden werden.
Der Viewer speichert seine erzeugte Kennung über Neustarts hinweg. Discovery bleibt
normalerweise im lokalen Subnetz; Routing, Firewalls und WLAN-Client-Isolation
können die Suche verhindern. Am Empfänger müssen Discovery-UDP-Port und TCP-Port
erreichbar sein. Discovery identifiziert Endpunkte, authentifiziert sie aber nicht.

### TCP-Versuchslimit und UDP-Fallback

`TCP_CONNECT_MAX_ATTEMPTS` gilt für **manuelles und automatisch ermitteltes TCP**:

- Standard: `10`; erlaubt: `1`–`65535`. `0` führt zu einem Compilerfehler.
- Eine erfolglose Suche oder ein fehlgeschlagener TCP-Verbindungszyklus zählt als
  Versuch. Sofort wieder geschlossene Verbindungen zählen ebenfalls zum Limit.
- Die Wartezeiten steigen auf 1, 2, 4, 8 und höchstens 10 Sekunden. Bleibt eine
  TCP-Verbindung 10 Sekunden aktiv, werden Fehlerzähler und Wartezeit zurückgesetzt.
- Das reine Warten auf WLAN verbraucht keine TCP-Versuche. Suche, DNS-Auflösung und
  Verbindungsaufbau kommen zu den Wartezeiten hinzu. Das Limit ist eine Anzahl,
  keine feste Zeitspanne.
- Beim Erreichen des Limits meldet der Worker den Wechsel auf Serial. Anschließende
  Logs gehen an `WIFI_LOGGING_UDP_IP` / `WIFI_LOGGING_UDP_PORT`.
- UDP bleibt aktiv, bis das Logging gestoppt und neu gestartet wird oder der ESP32
  neu startet. Während des UDP-Fallbacks wird TCP nicht weiter gesucht.
- Wartende TCP-Nachrichten und Reste teilweise versendeter Nachrichten werden beim
  Wechsel verworfen.

**Am Fallback-Ziel muss ein UDP-Empfänger lauschen.** Ein Viewer im TCP-Modus
empfängt nicht automatisch UDP-Logs. Verwende einen separaten UDP-Empfänger oder
schalte den Viewer zum Testen des Fallbacks auf UDP um. Der Discovery-UDP-Port ist
nicht der UDP-Zielport für Logs.

### Konfigurationsreferenz

| Flag | Standard | Zweck |
| --- | --- | --- |
| `WIFI_LOGGING_ENABLE` | In den gemeinsamen Projektflags auf `1` gesetzt | Netzwerk-Logging aktivieren; bei `0` bleibt Serial aktiv |
| `WIFI_LOGGING_USE_TCP` | Nicht gesetzt | Mit `1` TCP auswählen |
| `WIFI_LOGGING_USE_UDP` | Implizit `1`, wenn kein Transport gewählt ist | UDP ausdrücklich auswählen; nicht mit TCP kombinieren |
| `AUTO_SERVER_CONNECT` | `0` | `0`: manuelles TCP-Ziel; `1`: UDP-Suche vor TCP; benötigt TCP |
| `WIFI_LOGGING_TCP_HOST` | Kein Standard | Bei manuellem TCP erforderlich; IPv4-Adresse oder Hostname |
| `WIFI_LOGGING_TCP_PORT` | `10514` | TCP-Zielport bei manueller Konfiguration |
| `WIFI_LOGGING_DISCOVERY_PORT` | `10515` | Discovery-UDP-Port; muss zum Viewer passen |
| `WIFI_LOGGING_SERVER_ID` | Leer | Optionaler Discovery-Filter für einen bestimmten Viewer |
| `TCP_CONNECT_MAX_ATTEMPTS` | `10` | Fehlgeschlagene Zyklen bis zum UDP-Fallback; null ist ungültig |
| `WIFI_LOGGING_UDP_IP` | `192.168.0.112` im Code; `192.168.0.249` in den Projektflags | UDP-Logziel, auch für den Fallback |
| `WIFI_LOGGING_UDP_PORT` | `10514` | UDP-Zielport für Logs |

Ändere die vorhandenen Flags des gewählten Environments, statt widersprüchliche
Definitionen hinzuzufügen. Zeichenketten in `platformio.ini` verwenden die oben
gezeigten maskierten Anführungszeichen.

## TCP-Funktionen und Grenzen

- **Gemeinsame Logging-API:** Text, Client-Makros und CSV-Daten verwenden denselben
  Formatierungspfad wie UDP. Der Viewer kann die Zeilen an Filter, Dateien und
  Plot-/Logic-Visualizer weitergeben.
- **Netzwerk im Hintergrund:** Ein eigener Worker übernimmt Verbindung, Suche,
  Wiederverbindung und Versand. Aufrufende Tasks warten weder auf eine
  TCP-Verbindung noch auf freie Queueplätze. Die Serial-Ausgabe erfolgt weiterhin
  synchron im Logging-Aufruf.
- **Zeilenabgrenzung:** Jeder Logging-Aufruf gilt als vollständige Nachricht.
  Abschließende CR-/LF-Zeichen werden auf genau ein LF normalisiert. Interne
  Zeilenumbrüche und UTF-8-Bytes bleiben erhalten. Eine TCP-Zeile daher nicht aus
  mehreren separaten `RAW()`-Aufrufen zusammensetzen.
- **Teilversand:** Der Worker setzt anhand der tatsächlich gesendeten Bytezahl
  fort. Nachrichten verschiedener Tasks können sich im Netzwerkstrom nicht
  ineinanderschieben.
- **Begrenzter Speicher:** 16 Queueplätze, höchstens 511 formatierte Bytes je
  Aufruf einschließlich Präfix und übergebener Zeilenabschlüsse. Ein abschließendes
  LF wird bei Bedarf ergänzt. Die Queue benötigt ungefähr 8 KiB zuzüglich
  Verwaltungsdaten; Worker und Netzwerk benötigen weiteren Speicher.
- **Überlauf:** Bei voller Queue wird die neue Nachricht verworfen. Zu lange oder
  vom Formatierungspuffer abgeschnittene TCP-Nachrichten werden verworfen und
  gezählt. Serial behält sein bisheriges Abschneideverhalten. Offline erzeugte
  Logs werden nicht für eine spätere Übertragung gespeichert.
- **Zeitlimits:** TCP-Verbindungsversuche haben ein Socket-Zeitlimit von einer
  Sekunde; DNS kann länger dauern. Je Nachricht gilt eine Versandfrist von zwei
  Sekunden. Stop-Anfragen sind asynchron; der aufrufende Task wartet dabei nicht
  auf DNS oder Verbindungsaufbau.
- **Verbindungsabbruch:** Unvollständige Nachrichten und wartende Daten alter
  Sitzungen werden verworfen. Der Rest einer alten Nachricht wird niemals allein
  über eine neue Verbindung gesendet.
- **Diagnose:** Verbindungswechsel, Suchfehler und UDP-Fallback werden direkt auf
  Serial ausgegeben. `logging_transport::diag_print()` zeigt Zustand und Zähler,
  ohne rekursiv den Netzwerklogger zu verwenden.
- **Zustellgrenzen:** Keine Anwendungsbestätigungen, dauerhafte Pufferung, TLS oder
  garantierte Genau-einmal-Zustellung. Ein erfolgreicher Socket-Versand beweist
  nicht, dass der Viewer das Log verarbeitet hat. Es werden keine Heartbeats als
  normale Logzeilen erzeugt.
- **Ein Sender pro Viewer:** Der Viewer akzeptiert einen aktiven TCP-Client.
  Weitere Clients werden abgewiesen; Discovery meldet nur einen freien TCP-Platz.

Das 1-MiB-Zeilenlimit des Viewers vergrößert nicht das Nachrichtenlimit auf dem
ESP32. Beide Boardvarianten verwenden derzeit das Arduino-Framework.

## Automatisierte Prüfungen und Hardwaretests

Die portablen C++-Tests lassen sich ohne ESP32 ausführen:

```sh
python3 tests/run_host_tests.py
```

Geprüft werden Zeilenabgrenzung, Teilversand, Queuegrenzen, konkurrierende Aufrufe,
Serverauswahl, Wiederverbindung, UDP-Fallback und ungültige Build-Einstellungen.
Die plattformabhängigen Worker-Tests simulieren WLAN, Sockets und Task-Funktionen;
sie ersetzen keine Hardwaretests. `pio run` baut alle sechs Firmwarevarianten.

Vor dem praktischen Einsatz auf Board und Netzwerk prüfen: normales TCP-Logging,
Discovery mit einem und mehreren Viewern, Server-Neustart, WLAN-Ausfall, langsame
Gegenstelle sowie Fallback auf einen echten UDP-Empfänger. Diese Hardwareprüfung
steht noch aus.

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

- [`src/main.cpp`](src/main.cpp)
- [`include/log_core.h`](include/log_core.h)
- [`include/udp/fsd_udp.h`](include/udp/fsd_udp.h)
- [`src/fsd_udp.cpp`](src/fsd_udp.cpp)
- [`include/log_transport.h`](include/log_transport.h)
- [`src/fsd_tcp.cpp`](src/fsd_tcp.cpp)
- [`src/server_discovery.cpp`](src/server_discovery.cpp)
- [`tests/run_host_tests.py`](tests/run_host_tests.py)
- [`platformio.ini`](platformio.ini)
- [`CHANGELOG.md`](CHANGELOG.md)

## Gib mir einen Stern

Wenn dir meine Projekte gefallen, gib ihnen gern einen Stern.

## Englische Version

Wenn du dieselbe Readme auf Englisch lesen willst, schau in [`README.md`](README.md).
