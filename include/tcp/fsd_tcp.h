#pragma once
#include <stddef.h>
#include <stdint.h>

namespace tcp_logging {
// Start the asynchronous worker; repeated calls while active are harmless.
bool begin(const char *role);
// Request asynchronous stop; a new begin is rejected until the worker exits.
void end();
// Enqueue a complete call only while connected; never wait for free queue space.
bool send_bytes(const uint8_t *data, size_t len);
// Count a formatter overflow without recursively invoking network logging.
void note_oversize();
// Report state and delivery-attempt counters directly on Serial.
void diag_print();
}
