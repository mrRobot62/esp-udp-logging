#include "log_transport.h"
#if defined(WIFI_LOGGING_USE_TCP)
#include "tcp/fsd_tcp.h"
#else
#include "udp/fsd_udp.h"
#endif

namespace logging_transport {
#if defined(WIFI_LOGGING_USE_TCP)
bool begin(const char *role) { return tcp_logging::begin(role); }
void end() { tcp_logging::end(); }
bool send_bytes(const uint8_t *data, size_t len) { return tcp_logging::send_bytes(data, len); }
void note_oversize() { tcp_logging::note_oversize(); }
void diag_print() { tcp_logging::diag_print(); }
const char *name() { return "TCP"; }
#else
bool begin(const char *role) { return udp::begin(role); }
void end() { udp::end(); }
bool send_bytes(const uint8_t *data, size_t len) { return udp::send_bytes(data, len); }
void note_oversize() {}
void diag_print() { udp::diag_print(); }
const char *name() { return "UDP"; }
#endif
}
