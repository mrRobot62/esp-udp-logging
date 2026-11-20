#pragma once

// Exactly one transport is selected at compile time; existing builds use UDP.
#if defined(WIFI_LOGGING_USE_UDP) && defined(WIFI_LOGGING_USE_TCP)
#error "Select only WIFI_LOGGING_USE_UDP or WIFI_LOGGING_USE_TCP"
#endif
#if defined(WIFI_LOGGING_USE_TCP) && WIFI_LOGGING_USE_TCP != 1
#error "WIFI_LOGGING_USE_TCP must be 1 or omitted"
#endif
#if defined(WIFI_LOGGING_USE_UDP) && WIFI_LOGGING_USE_UDP != 1
#error "WIFI_LOGGING_USE_UDP must be 1 or omitted"
#endif
#if !defined(WIFI_LOGGING_USE_TCP) && !defined(WIFI_LOGGING_USE_UDP)
#define WIFI_LOGGING_USE_UDP 1
#endif

#ifndef TCP_CONNECT_MAX_ATTEMPTS
#define TCP_CONNECT_MAX_ATTEMPTS 10
#endif
#if TCP_CONNECT_MAX_ATTEMPTS < 1 || TCP_CONNECT_MAX_ATTEMPTS > 65535
#error "TCP_CONNECT_MAX_ATTEMPTS must be between 1 and 65535 (zero is not allowed)"
#endif
static_assert(TCP_CONNECT_MAX_ATTEMPTS >= 1 && TCP_CONNECT_MAX_ATTEMPTS <= 65535,
              "TCP_CONNECT_MAX_ATTEMPTS must be a positive integer up to 65535");

#ifndef AUTO_SERVER_CONNECT
#define AUTO_SERVER_CONNECT 0
#endif
#if AUTO_SERVER_CONNECT != 0 && AUTO_SERVER_CONNECT != 1
#error "AUTO_SERVER_CONNECT must be 0 or 1"
#endif
// Also reject nonnumeric identifiers which the preprocessor otherwise treats as zero.
static_assert(AUTO_SERVER_CONNECT == 0 || AUTO_SERVER_CONNECT == 1, "AUTO_SERVER_CONNECT must be 0 or 1");
#if AUTO_SERVER_CONNECT && !defined(WIFI_LOGGING_USE_TCP)
#error "AUTO_SERVER_CONNECT requires WIFI_LOGGING_USE_TCP=1"
#endif
#ifndef WIFI_LOGGING_DISCOVERY_PORT
#define WIFI_LOGGING_DISCOVERY_PORT 10515
#endif
#if WIFI_LOGGING_DISCOVERY_PORT < 1 || WIFI_LOGGING_DISCOVERY_PORT > 65535
#error "WIFI_LOGGING_DISCOVERY_PORT must be between 1 and 65535"
#endif
#ifndef WIFI_LOGGING_SERVER_ID
#define WIFI_LOGGING_SERVER_ID ""
#endif

#if defined(WIFI_LOGGING_USE_TCP)
#if !AUTO_SERVER_CONNECT
#ifndef WIFI_LOGGING_TCP_HOST
#error "TCP requires WIFI_LOGGING_TCP_HOST (server IPv4 address or hostname)"
#endif
// Keep the established default port for existing manual configurations.
#ifndef WIFI_LOGGING_TCP_PORT
#define WIFI_LOGGING_TCP_PORT 10514
#endif
#if WIFI_LOGGING_TCP_PORT < 1 || WIFI_LOGGING_TCP_PORT > 65535
#error "WIFI_LOGGING_TCP_PORT must be between 1 and 65535"
#endif
#endif

#endif // manual TCP configuration
