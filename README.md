# esp-logging-demo — Serial, UDP and TCP logging

Version **0.2.0**. Repository: `esp-udp-logging`.

If you want to test network logging directly with a desktop viewer, have a look at the companion project first:

[udp-viewer](https://github.com/mrRobot62/udp-viewer.git)

**This demo repo is meant exactly for that use-case too: you flash it to an ESP32, let it send UDP datagrams or TCP log lines, and then you can check the result directly in the `udp-viewer`.**

If you just want a very small ESP32 example for structured logging on `Serial` and optional UDP or TCP logging over WiFi, this project is for you.

I made this repo as a compact demo project. It is based on parts from my bigger `FilamentSilicatDryer_480x480` project, but here everything is stripped down a lot so you can understand the basic idea without digging through the full application.

This example shows you mainly two things:

- how you can write structured log messages like `INFO(...)`, `WARN(...)` or `ERR(...)`
- how the same logging calls can send UDP datagrams or TCP lines over WiFi

So the output can go to the serial monitor and, if WiFi is available, to a network log receiver at the same time. UDP remains the default; select TCP at build time as described below.

## Why this repo exists

Sometimes you don't want to point somebody to a full production project only to explain one small topic.

This repo is meant as a small reference project you can share when somebody asks:

- "How do you do structured logging on an ESP32?"
- "How do you mirror serial logs to UDP?"
- "Do you have a minimal WiFi logging example?"
- "Do you have a small CSV logging example for udp-viewer?"

That was the whole point here.

## Notes

- This project is not trying to be a full reusable library package.
- It is a small example first.
- The code is kept close to the original project style, so moving between demo and real project is easy.
- In UDP mode, WiFi initialization can block startup for up to eight seconds. TCP initializes and sends in a background worker.
- The ESP32 is the TCP client; the desktop viewer is the TCP server.
- Existing `INFO`, `DBG`, `WARN`, `ERR`, `CLIENT_*`, `RAW` and CSV logging calls work with both transports.
- Firmware builds and automated tests have passed. Real ESP32/WLAN integration and Windows-specific discovery behavior still require hardware/platform testing.

## What is inside of this project

The project is intentionally small and nothing more as an example demo

- `src/main.cpp`
  This is the example application. It starts `Serial`, initializes the selected logging transport and prints a few demo log messages in the loop.

- `include/log_core.h`
  This is the central logging helper. The macros like `INFO`, `DBG`, `WARN`, `ERR` are defined here.

- `include/udp/fsd_udp.h`
  Small public header for the UDP helper.

- `src/fsd_udp.cpp`
  The WiFi and UDP implementation. It connects the ESP32 to your WLAN and sends log lines as UDP packets.

- `include/wifi_secrets.h.example`
  A template file for your local WiFi credentials.

- `platformio.ini`
  Build environments for ESP32 and ESP32-S3, each with UDP, manual TCP and automatic TCP discovery.

- `include/log_transport.h` / `src/log_transport.cpp`
  Common transport interface selected at compile time.

- `src/fsd_tcp.cpp`
  TCP worker, bounded queue, reconnect handling and UDP fallback.

- `src/server_discovery.cpp` / `include/tcp/discovery_protocol.h`
  UDP-based server discovery and response validation.

## What the logging looks like

The logs are written with a prefix, so you can see directly what kind of message it is.

**Example:**

```text
[MAIN/INFO] Booting esp-logging-demo 0.2.0
[MAIN/DEBUG] Debug log example, free_heap=231456
[MAIN/WARN] Periodic warning example, counter=5
```
- [xxxxx] - up to you what kind of info do you want to insert here
- [MAIN/INFO] - this text can be used as filter inside the UDP-LOG-VIEWER.

It is not a huge logging framework. It is just a simple and practical setup that works nice for embedded debugging.

## CSV logging examples

The demo in [`src/main.cpp`](src/main.cpp) does not only send normal text logs. It also sends two CSV style log lines that are useful for plotting or for checking state changes in the `udp-viewer`.

- Format: [xxxxx] this is you filter criteria for UDP-LOG-VIEWER, Semicolon as devider from values. Number of values up to you, but this should be exactly used in the visualization configuration of your graph in UDP-LOG-VIEWER
- [CSV_CLIENT_PLOT] : Example: this line represent some interesting Numbers created by your client software 
- [CSV_HOST_PLOT] : Example: this line represent some interesting Numbers created by your host software 
- [CSV_LONG_TERM_PLOT] : Example: this line represent some interesting Numbers for a long term analysis 
- [feel_free] : feel free to use filter names as you like :-) 

### Example 1: temperature CSV

This is generated with `CSV_LOG_CLIENT_TEMP(...)`. In this demo, this log lines represent measurements from a client esp32 sending data via UDP or TCP

<img width="1582" height="658" alt="udp_log_viewer_plot_measure1" src="https://github.com/user-attachments/assets/1238b9ae-ddcc-4558-8290-7f842e4d69fd" />

```text
[CSV_CLIENT_PLOT];12482;1471;615;10147;1329;442;2;1;0
```

Meaning of the values:

- `12482` = hotspot raw ADC value
- `1471` = hotspot millivolts
- `615` = hotspot temperature in deci-degrees C, so `61.5C`
- `10147` = chamber raw ADC value
- `1329` = chamber millivolts
- `442` = chamber temperature in deci-degrees C, so `44.2C`
- `2` = demo state, for example `HEATING`
- `1` = heater on
- `0` = door closed

This format is based on the temperature CSV logging style I already use in the bigger dryer project.

### Example 2: logic CSV

This is generated with `CSV_LOG_CLIENT_LOGIC(...)`. In this demo we try to log GPIO states (HIGH/LOW) and want to visualize them in UDP-LOG_VIEWER as 
<img width="1769" height="782" alt="udp_log_viewer_logic_signal2" src="https://github.com/user-attachments/assets/810f19b4-f61e-4a76-adb0-5886ad821c76" />

```text
[CSV_CLIENT_LOGIC];1;1;0;0;1;1;0;2
```

Meaning of the values:

- `1` = fan12V on
- `1` = fan230V on
- `0` = fan230V slow off
- `0` = motor off
- `1` = heater on
- `1` = lamp on
- `0` = door closed
- `2` = demo state as integer

So with the current demo you can test both things at once:

- classic readable log text
- structured CSV data for viewer or plotting

## Configure the UDP-LOG-VIEWER
If you want to use this demo with UDP-LOG-Viewer you should configure the graphs inside UDP-LOG-VIEWER

### Config PLOT-Visualizer
<img width="1372" height="860" alt="image" src="https://github.com/user-attachments/assets/0a014dca-baef-4f3f-ad80-e2f46e11d0d2" />

### Config LOGIC-Visualizer
<img width="1372" height="860" alt="image" src="https://github.com/user-attachments/assets/ed71a0da-6213-422e-b73c-e7ccb7067a82" />

## How it works

The basic flow is this:

1. `Serial.begin(115200)` starts the serial output.
2. `logging_transport::begin("CLIENT")` initializes the selected transport. For TCP, success means the worker started, not that a server is already connected.
3. `INFO(...)`, `WARN(...)`, `DBG(...)` format the line once.
4. The line is written to `Serial`.
5. If network logging is enabled, the selected transport sends the formatted message. TCP queues complete calls only while connected; early boot and offline messages remain available on Serial.

So you don't need two different log calls for serial and network. One log call is enough.

## Setup

Before you build the project, do this once:

1. Copy `include/wifi_secrets.h.example` to `include/wifi_secrets.h`
2. Put in your WiFi name and password
3. Open `platformio.ini`
4. Select the UDP, manual TCP or automatic TCP environment below.
5. Configure the receiver address and ports. Keep the UDP target valid even for TCP builds: it is used after fallback.
6. Start the matching receiver in the viewer before flashing/running the ESP32.

Your local `wifi_secrets.h` is ignored by git on purpose, so your real credentials do not end up in the repository.

## Example `wifi_secrets.h`

```cpp
#pragma once

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
```

## Select UDP or TCP before compiling

Choose a PlatformIO environment; no changes to your logging calls are needed.

| Board | UDP (default) | TCP with fixed server | TCP with automatic discovery |
| --- | --- | --- | --- |
| ESP32 | `esp32dev` | `esp32dev-tcp` | `esp32dev-tcp-auto` |
| ESP32-S3 | `esp32-s3-devkitc-1` | `esp32-s3-devkitc-1-tcp` | `esp32-s3-devkitc-1-tcp-auto` |

In the PlatformIO IDE, use **Project Tasks → environment → Build / Upload**.
From a terminal, choose exactly the environment you want:

```sh
# Classic ESP32: choose one of these builds
pio run -e esp32dev
pio run -e esp32dev-tcp
pio run -e esp32dev-tcp-auto

# Example: build and upload automatic TCP discovery to ESP32-S3
pio run -e esp32-s3-devkitc-1-tcp-auto -t upload
pio device monitor -b 115200
```

Changing the environment or its flags takes effect after rebuilding and flashing
that firmware. `pio run` without `-e` builds **all six environments**. Always specify
`-e` when uploading. A running TCP build can switch to UDP through the fallback
policy below; there is no runtime menu for choosing the transport on the ESP32.

### UDP

The shared `[env]` section of `platformio.ini` contains the UDP destination:

```ini
-DWIFI_LOGGING_ENABLE=1
-DWIFI_LOGGING_UDP_IP=\"192.168.0.249\"
-DWIFI_LOGGING_UDP_PORT=10514
```

Use `esp32dev` or `esp32-s3-devkitc-1`. With no explicit transport flag, UDP is
selected. `-DWIFI_LOGGING_USE_UDP=1` is also supported, but must not be combined
with `WIFI_LOGGING_USE_TCP`. The IP above is an example: replace it with your
receiver's address. If the UDP options are omitted, the implementation defaults
to `192.168.0.112:10514`.

### TCP with a configured server

The `*-tcp` environments add these flags to the shared settings:

```ini
-DWIFI_LOGGING_USE_TCP=1
-DAUTO_SERVER_CONNECT=0
-DWIFI_LOGGING_TCP_HOST=\"192.168.0.249\"
-DWIFI_LOGGING_TCP_PORT=10514
-DTCP_CONNECT_MAX_ATTEMPTS=10
```

The ESP32 connects to this IPv4 address or hostname. Do not use `0.0.0.0` or a
broadcast address as the TCP destination. `AUTO_SERVER_CONNECT` defaults to `0`
when omitted. A manual TCP build requires `WIFI_LOGGING_TCP_HOST`; the TCP port
defaults to `10514` for compatibility, but is explicitly set in the environments.

In the viewer, select **TCP**, enter the local bind address and TCP port, then
click **CONNECT**. A viewer bind address of `0.0.0.0` means “listen on all local
interfaces”; the ESP32 still needs a concrete destination address.

### TCP with automatic server discovery

The `*-tcp-auto` environments use:

```ini
-DWIFI_LOGGING_USE_TCP=1
-DAUTO_SERVER_CONNECT=1
-DWIFI_LOGGING_DISCOVERY_PORT=10515
-DTCP_CONNECT_MAX_ATTEMPTS=10
```

No fixed TCP address or port is required in this mode. On the viewer with discovery
support, select **TCP**, enable **Auto server discovery**, set the same discovery
UDP port, and click **CONNECT**. The viewer's TCP port may differ from the discovery
port: its reply tells the ESP32 which TCP port to use.

Discovery uses **UDP broadcast**, followed by a normal TCP connection for logs.
The ESP32 sends two requests during a 700 ms search window. Replies must match the
protocol version and the current random request identifier. The source IP of the
reply becomes the TCP destination. A viewer only answers while its TCP listener
is running and no sender is connected.

If multiple distinct matching endpoints answer, the ESP32 does not choose one
arbitrarily. Select a particular viewer by copying its **Server ID** into the
chosen environment's `build_flags`:

```ini
-DWIFI_LOGGING_SERVER_ID=\"paste-the-viewer-server-id-here\"
```

Without this optional flag, exactly one responding endpoint must be found. The
viewer saves its generated identity across restarts. Discovery normally stays
within the local subnet; routing, firewalls and WiFi client isolation can prevent
it. Allow the discovery UDP port and the TCP listening port on the receiver.
Discovery identifies endpoints but does not authenticate them.

### TCP retry limit and UDP fallback

`TCP_CONNECT_MAX_ATTEMPTS` applies to **manual and automatic TCP**:

- Default: `10`; allowed range: `1`–`65535`. `0` is a compile-time error.
- An unsuccessful search or a failed TCP connection cycle consumes one attempt.
  Connections that immediately close also count toward the limit.
- Retry delays grow through 1, 2, 4, 8 and at most 10 seconds. A TCP connection
  that remains active for 10 seconds resets the failure count and backoff.
- Waiting for WiFi alone does not consume TCP attempts. Discovery, DNS and
  connection time add to the retry delays; the limit is a count, not a timeout.
- At the limit, the worker reports the switch on Serial and sends subsequent logs
  through the existing `WIFI_LOGGING_UDP_IP` / `WIFI_LOGGING_UDP_PORT` settings.
- It stays on UDP until logging is stopped and started again or the ESP32 reboots.
  It does not keep probing TCP while UDP fallback is active.
- Pending TCP messages and partially sent tails are discarded during the switch.

**An active UDP receiver must listen at the fallback destination.** A viewer
running in TCP mode does not automatically receive UDP log packets. Use a separate
UDP receiver or switch the viewer to UDP when testing fallback. The UDP discovery
port is not the UDP log destination.

### Configuration reference

| Flag | Default | Purpose |
| --- | --- | --- |
| `WIFI_LOGGING_ENABLE` | Set to `1` by this project's shared flags | Enable network logging; `0` leaves Serial logging active |
| `WIFI_LOGGING_USE_TCP` | Omitted | Set to `1` to select TCP |
| `WIFI_LOGGING_USE_UDP` | Implicit `1` when no transport is selected | Explicit UDP selection; mutually exclusive with TCP |
| `AUTO_SERVER_CONNECT` | `0` | `0`: manual TCP target; `1`: UDP discovery before TCP; requires TCP |
| `WIFI_LOGGING_TCP_HOST` | No default | Required for manual TCP; IPv4 address or hostname |
| `WIFI_LOGGING_TCP_PORT` | `10514` | Manual TCP destination port |
| `WIFI_LOGGING_DISCOVERY_PORT` | `10515` | Discovery UDP port, matching the viewer |
| `WIFI_LOGGING_SERVER_ID` | Empty | Optional discovery filter for one viewer |
| `TCP_CONNECT_MAX_ATTEMPTS` | `10` | Failed cycles before UDP fallback; zero is invalid |
| `WIFI_LOGGING_UDP_IP` | `192.168.0.112` in code; `192.168.0.249` in this project's flags | UDP log destination, also used for fallback |
| `WIFI_LOGGING_UDP_PORT` | `10514` | UDP log destination port |

Edit the selected environment's existing flags rather than adding conflicting
copies. String values in `platformio.ini` use the escaped quotes shown above.

## TCP behavior and limits

- **Common logging API:** text, client macros and CSV data use the same formatting
  path as UDP. The viewer can feed received lines into filters, files and
  Plot/Logic visualizers.
- **Background networking:** a dedicated worker owns connection setup, discovery,
  reconnects and network sends. Calling tasks do not wait for TCP connection or
  free queue space. Serial output still runs synchronously in the logging call.
- **Line framing:** each logging call is treated as a complete message. Trailing
  CR/LF characters are normalized to one LF; internal line breaks and UTF-8 bytes
  are preserved. Do not use separate `RAW()` calls to assemble one TCP line.
- **Partial writes:** the worker continues from the actual number of bytes sent.
  Messages from different tasks cannot interleave on the network stream.
- **Bounded memory:** 16 queued messages, with at most 511 formatted bytes per
  call, including any log prefix and supplied line endings; one final LF is added
  as needed. Queue storage is about 8 KiB, plus worker and network overhead.
- **Overflow handling:** a full queue drops the new message. Oversized or formatter-
  truncated TCP messages are dropped and counted. Serial retains its existing
  truncation behavior. Logs produced offline are not stored for later delivery.
- **Timeouts:** TCP connection attempts have a 1-second socket timeout; DNS lookup
  can take longer. Each record has a 2-second send deadline. Stop requests are
  asynchronous and do not wait for DNS or connection setup in the caller.
- **Disconnects:** the worker discards incomplete records and queued old-session
  data. It never sends only an old record's remaining bytes on a new connection.
- **Diagnostics:** connection changes, discovery failures and UDP fallback are
  printed directly to Serial. `logging_transport::diag_print()` reports state and
  counters without recursively using the network logger.
- **Delivery limits:** there are no application acknowledgements, persistent
  buffering, TLS or exactly-once guarantees. A successful socket send does not
  prove that the viewer processed the log. No log-line heartbeats are generated.
- **One sender per viewer:** the viewer accepts one active TCP client. Additional
  clients are rejected; discovery only advertises an available TCP slot.

The viewer's 1 MiB maximum line size does not increase the ESP32's message limit.
Both ESP32 and ESP32-S3 environments currently use the Arduino framework.

## Automated checks and hardware validation

Run the portable C++ tests without an ESP32:

```sh
python3 tests/run_host_tests.py
```

These cover framing, partial sends, queue limits, concurrent producers, discovery
selection, reconnects, UDP fallback and invalid build settings. Platform-dependent
worker tests use simulated WiFi, sockets and task primitives; they do not replace
hardware tests. `pio run` builds all six firmware variants.

Before relying on the firmware, test on the actual board and network: normal TCP
logging, discovery with one/multiple viewers, server restart, WiFi loss, a slow
receiver, and fallback to a real UDP receiver. Hardware validation is still pending.

## How to receive the UDP logs

You can use whatever UDP receiver you like.

For a quick test on your computer, tools like `nc`, `ncat`, `socat`, Wireshark or your own small Python receiver will do the job, or you use my own [udp-viewer](https://github.com/mrRobot62/udp-viewer.git) written in Python.

**One simple example with netcat can be:** <br>
(only checking if your PC get UDP messages - I don't know how to do this on Windows :-/)

```bash
nc -ul 10514
```

Depending on your system, you maybe need `ncat` instead of `nc`, but the idea is the same: open a UDP listener on the port from `platformio.ini`.

# Ready to start with your UDP-LOG-VIEWER
If you get messages on you PC, now it's time to use the UDP-LOG-VIEWER.
Feel free to play around your embedded system, try to change values on your embedded system, analyze them with your UDP-LOG-VIEWER.

Be happy .....

# Related files

If you want to look at the main files directly:

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

## Give me a star
If you like my projects give them a star

## German version

The German version, including TCP, discovery and UDP fallback, is available in
[`README_DE.md`](README_DE.md).
