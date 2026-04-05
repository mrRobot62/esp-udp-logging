#pragma once

#include "log_core.h"

namespace csv {

struct CLIENT_TEMP {
    static constexpr const char *PREFIX = "CLIENT_PLOT";
    static constexpr const char *FMT =
        "[CSV_%s];%ld;%ld;%ld;%ld;%ld;%ld;%d;%d;%d\n";
};

struct CLIENT_LOGIC {
    static constexpr const char *PREFIX = "CLIENT_LOGIC";
    static constexpr const char *FMT =
        "[CSV_%s];%d;%d;%d;%d;%d;%d;%d;%u\n";
};

} // namespace csv

#ifdef CSV_OUT
#define CSV_LOG(prefix, fmt, ...)        \
    do {                                 \
        RAW(fmt, prefix, ##__VA_ARGS__); \
    } while (0)
#else
#define CSV_LOG(...) \
    do {             \
    } while (0)
#endif

#ifdef CSV_OUT
#define CSV_LOG_CLIENT_TEMP(...)                                                 \
    do {                                                                         \
        CSV_LOG(csv::CLIENT_TEMP::PREFIX, csv::CLIENT_TEMP::FMT, ##__VA_ARGS__); \
    } while (0)
#else
#define CSV_LOG_CLIENT_TEMP(...) \
    do {                         \
    } while (0)
#endif

#ifdef CSV_OUT
#define CSV_LOG_CLIENT_LOGIC(...)                                                  \
    do {                                                                           \
        CSV_LOG(csv::CLIENT_LOGIC::PREFIX, csv::CLIENT_LOGIC::FMT, ##__VA_ARGS__); \
    } while (0)
#else
#define CSV_LOG_CLIENT_LOGIC(...) \
    do {                          \
    } while (0)
#endif
