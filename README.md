# esp-udp-logging

If you want to test the UDP logging directly with a desktop viewer, have a look at the companion project first:

[udp-viewer](https://github.com/mrRobot62/udp-viewer.git)

This demo repo is meant exactly for that use-case too: you flash it to an ESP32, let it send UDP log packets, and then you can check the result directly in the `udp-viewer`.

If you just want a very small ESP32 example for structured logging on `Serial` and optional UDP logging over WiFi, this project is for you.

I made this repo as a compact demo project. It is based on parts from my bigger `FilamentSilicatDryer_480x480` project, but here everything is stripped down a lot so you can understand the basic idea without digging through the full application.

This example shows you mainly two things:

- how you can write structured log messages like `INFO(...)`, `WARN(...)` or `ERR(...)`
- how the same log lines can also be sent as UDP packets over WiFi

So the output can go to the serial monitor and, if WiFi is available, to a UDP log receiver at the same time.

## What is inside

The project is intentionally small.

- `src/main.cpp`
  This is the example application. It starts `Serial`, initializes UDP logging and prints a few demo log messages in the loop.

- `include/log_core.h`
  This is the central logging helper. The macros like `INFO`, `DBG`, `WARN`, `ERR` are defined here.

- `include/udp/fsd_udp.h`
  Small public header for the UDP helper.

- `src/fsd_udp.cpp`
  The WiFi and UDP implementation. It connects the ESP32 to your WLAN and sends log lines as UDP packets.

- `include/wifi_secrets.h.example`
  A template file for your local WiFi credentials.

- `platformio.ini`
  PlatformIO config with the UDP target IP, target port and the compile flag for enabling WiFi logging.

## What the logging looks like

The logs are written with a prefix, so you can see directly what kind of message it is.

Example:

```text
[MAIN/INFO] Booting esp-udp-logging example
[MAIN/INFO] UDP logging ready
[MAIN/DEBUG] Debug log example, free_heap=231456
[MAIN/WARN] Periodic warning example, counter=5
```

It is not a huge logging framework. It is just a simple and practical setup that works nice for embedded debugging.

## How it works

The basic flow is this:

1. `Serial.begin(115200)` starts the serial output.
2. `udp::begin("CLIENT")` tries to connect to WiFi and prepares UDP.
3. `INFO(...)`, `WARN(...)`, `DBG(...)` format the line once.
4. The line is written to `Serial`.
5. If UDP logging is enabled and WiFi is connected, the same line is also sent over UDP.

So you don't need two different log calls for serial and network. One log call is enough.

## Setup

Before you build the project, do this once:

1. Copy `include/wifi_secrets.h.example` to `include/wifi_secrets.h`
2. Put in your WiFi name and password
3. Open `platformio.ini`
4. Adjust the target IP address for your UDP log receiver
5. Adjust the UDP port if needed

Your local `wifi_secrets.h` is ignored by git on purpose, so your real credentials do not end up in the repository.

## Example `wifi_secrets.h`

```cpp
#pragma once

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
```

## PlatformIO config

In `platformio.ini` you will find these relevant settings:

```ini
-DWIFI_LOGGING_ENABLE=1
-DWIFI_LOGGING_UDP_IP="192.168.0.249"
-DWIFI_LOGGING_UDP_PORT=10514
```

What that means:

- `WIFI_LOGGING_ENABLE=1`
  WiFi/UDP logging is compiled in.

- `WIFI_LOGGING_UDP_IP`
  This is the IP of your PC or your logging server.

- `WIFI_LOGGING_UDP_PORT`
  This is the UDP port where your receiver is listening.

One important detail: there is also a default target IP and port inside [`src/fsd_udp.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/fsd_udp.cpp).

So if you do nothing, the code still has a built-in fallback. But in normal use, the values from `platformio.ini` are passed in as compiler defines and override the defaults from `fsd_udp.cpp`.

If you disable WiFi logging, the serial logging still works. That part is useful because it keeps the demo flexible.

## Build and upload

Usual PlatformIO commands:

```bash
pio run
pio run -t upload
pio device monitor
```

If your board is connected and the upload port is configured right, this should be enough already.

## How to receive the UDP logs

You can use whatever UDP receiver you like.

For a quick test on your computer, tools like `nc`, `ncat`, `socat`, Wireshark or your own small Python receiver will do the job, or you use my own [udp-viewer](https://github.com/mrRobot62/udp-viewer.git) written in Python.

One simple example with netcat can be:

```bash
nc -ul 10514
```

Depending on your system, you maybe need `ncat` instead of `nc`, but the idea is the same: open a UDP listener on the port from `platformio.ini`.

## Why this repo exists

Sometimes you don't want to point somebody to a full production project only to explain one small topic.

This repo is meant as a small reference project you can share when somebody asks:

- "How do you do structured logging on an ESP32?"
- "How do you mirror serial logs to UDP?"
- "Do you have a minimal WiFi logging example?"

That was the whole point here.

## Notes

- This project is not trying to be a full reusable library package.
- It is a small example first.
- The code is kept close to the original project style, so moving between demo and real project is easy.
- The WiFi connect is blocking for a short time during startup. For a demo, that is okay.
- If WiFi is not available, the example continues and serial logging still works.

## Related files

If you want to look at the main files directly:

- [`src/main.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/main.cpp)
- [`include/log_core.h`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/include/log_core.h)
- [`include/udp/fsd_udp.h`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/include/udp/fsd_udp.h)
- [`src/fsd_udp.cpp`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/src/fsd_udp.cpp)
- [`platformio.ini`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/platformio.ini)

## Give me a star
If you like my projects give them a star

## German version

If you want the same readme in German, check [`README_DE.md`](/Users/bernhardklein/workspace/arduino/esp32/esp-udp-logging/README_DE.md).
