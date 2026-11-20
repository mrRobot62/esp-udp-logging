#pragma once

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

#include "log_transport.h"

#ifdef INFO
#undef INFO
#endif
#ifdef WARN
#undef WARN
#endif
#ifdef ERR
#undef ERR
#endif

#ifndef LOG_CORE_LINEBUF_SIZE
#define LOG_CORE_LINEBUF_SIZE 512
#endif

// Keep the legacy helper name while dispatching to the selected transport.
inline void logWriteUdpIfEnabled(const char *s, size_t n, bool complete = true) {
#if defined(WIFI_LOGGING_ENABLE) && (WIFI_LOGGING_ENABLE == 1)
#if defined(WIFI_LOGGING_USE_TCP)
    if (!complete) {
        logging_transport::note_oversize();
        return;
    }
#else
    (void)complete;
#endif
    if (s && n) {
        logging_transport::send_bytes(reinterpret_cast<const uint8_t *>(s), n);
    }
#else
    (void)s;
    (void)n;
    (void)complete;
#endif
}

inline void logWriteSerial(const char *s, size_t n) {
    if (s && n) {
        Serial.write(reinterpret_cast<const uint8_t *>(s), n);
    }
}

inline void logPrintPrefixToBuf(char *out, size_t out_size, const char *tag, const char *level) {
    if (!out || out_size == 0) {
        return;
    }
    const int n = snprintf(out, out_size, "[%s/%s] ", tag, level);
    if (n < 0) {
        out[0] = '\0';
    }
}

// Format once; retain Serial output and reject truncated TCP records.
inline void logVPrintf(const char *tag, const char *level, const char *fmt, va_list args) {
    char line[LOG_CORE_LINEBUF_SIZE];
    char prefix[64];
    logPrintPrefixToBuf(prefix, sizeof(prefix), tag, level);

    size_t p_len = strnlen(prefix, sizeof(prefix));
    if (p_len >= sizeof(line)) {
        p_len = sizeof(line) - 1;
    }
    memcpy(line, prefix, p_len);

    va_list args_copy;
    va_copy(args_copy, args);
    const int n = vsnprintf(line + p_len, sizeof(line) - p_len, fmt, args_copy);
    va_end(args_copy);
    if (n < 0) {
        return;
    }

    size_t total = p_len + static_cast<size_t>(n);
    const bool complete = total < sizeof(line);
    if (total >= sizeof(line)) {
        total = sizeof(line) - 1;
    }

    logWriteSerial(line, total);
    logWriteUdpIfEnabled(line, total, complete);
}

inline void logPrintf(const char *tag, const char *level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logVPrintf(tag, level, fmt, args);
    va_end(args);
}

// Format an unprefixed call with the same transport overflow policy.
inline void logRawVPrintf(const char *fmt, va_list args) {
    char line[LOG_CORE_LINEBUF_SIZE];

    va_list args_copy;
    va_copy(args_copy, args);
    const int n = vsnprintf(line, sizeof(line), fmt, args_copy);
    va_end(args_copy);
    if (n < 0) {
        return;
    }

    size_t total = static_cast<size_t>(n);
    const bool complete = total < sizeof(line);
    if (total >= sizeof(line)) {
        total = sizeof(line) - 1;
    }

    logWriteSerial(line, total);
    logWriteUdpIfEnabled(line, total, complete);
}

inline void logRawPrintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logRawVPrintf(fmt, args);
    va_end(args);
}

#define RAW(...)                   \
    do {                           \
        logRawPrintf(__VA_ARGS__); \
    } while (0)

#define INFO(...)                               \
    do {                                        \
        logPrintf("MAIN", "INFO", __VA_ARGS__); \
    } while (0)

#define DBG(...)                                 \
    do {                                         \
        logPrintf("MAIN", "DEBUG", __VA_ARGS__); \
    } while (0)

#define WARN(...)                               \
    do {                                        \
        logPrintf("MAIN", "WARN", __VA_ARGS__); \
    } while (0)

#define ERR(...)                               \
    do {                                       \
        logPrintf("MAIN", "ERR", __VA_ARGS__); \
    } while (0)

#define ERROR(...)                             \
    do {                                       \
        logPrintf("MAIN", "ERR", __VA_ARGS__); \
    } while (0)
