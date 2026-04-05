#pragma once

#include "log_core.h"

#ifdef CLIENTINFO
#define CLIENT_INFO(...)                          \
    do {                                          \
        logPrintf("CLIENT", "INFO", __VA_ARGS__); \
    } while (0)
#else
#define CLIENT_INFO(...) \
    do {                 \
    } while (0)
#endif

#ifdef CLIENTDBG
#define CLIENT_DBG(...)                            \
    do {                                           \
        logPrintf("CLIENT", "DEBUG", __VA_ARGS__); \
    } while (0)
#else
#define CLIENT_DBG(...) \
    do {                \
    } while (0)
#endif

#ifdef CLIENTWARN
#define CLIENT_WARN(...)                          \
    do {                                          \
        logPrintf("CLIENT", "WARN", __VA_ARGS__); \
    } while (0)
#else
#define CLIENT_WARN(...) \
    do {                 \
    } while (0)
#endif

#ifdef CLIENTERR
#define CLIENT_ERR(...)                          \
    do {                                         \
        logPrintf("CLIENT", "ERR", __VA_ARGS__); \
    } while (0)
#else
#define CLIENT_ERR(...) \
    do {                \
    } while (0)
#endif

#ifdef CLIENTRAW
#define CLIENT_RAW(...)            \
    do {                           \
        logRawPrintf(__VA_ARGS__); \
    } while (0)
#else
#define CLIENT_RAW(...) \
    do {                \
    } while (0)
#endif
