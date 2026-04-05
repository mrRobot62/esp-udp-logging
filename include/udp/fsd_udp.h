#pragma once

#include <stddef.h>
#include <stdint.h>

namespace udp {

bool is_enabled();
void configure(const char *targetIp, uint16_t targetPort);
bool begin(const char *roleLabel);
void end();
bool send_bytes(const uint8_t *data, size_t len);
void diag_print();

inline bool send_cstr(const char *s) {
    if (!s) {
        return false;
    }
    size_t n = 0;
    while (s[n] != '\0') {
        ++n;
    }
    return send_bytes(reinterpret_cast<const uint8_t *>(s), n);
}

} // namespace udp
