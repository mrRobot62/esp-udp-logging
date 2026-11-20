#pragma once

#include <stddef.h>
#include <stdint.h>
#include "log_transport_config.h"

namespace logging_transport {
// Start from setup/main context. TCP success means worker started, not connected.
bool begin(const char *role);
// Request shutdown without waiting for network operations; call from main context.
void end();
// Copy one complete logging call; TCP never waits for queue capacity or networking.
bool send_bytes(const uint8_t *data, size_t len);
// Report a formatting overflow without sending a truncated TCP record.
void note_oversize();
// Print transport state and counters directly to Serial, never through logging.
void diag_print();
// Return the compile-time transport name for application status messages.
const char *name();
}
