#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace tcp_discovery {
constexpr size_t kDatagramLimit = 192;

// Permit portable ASCII identities without protocol separators.
inline bool valid_id(const char *id) {
    const size_t n = strlen(id);
    if (!n || n > 64) return false;
    for (size_t i = 0; i < n; ++i) {
        const char c = id[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
    }
    return true;
}

// Retain one endpoint and reject ambiguity without allocating a candidate list.
struct Selection {
    uint32_t ip = 0;
    uint16_t port = 0;
    char id[65] = {};
    bool ambiguous = false;

    // Validate the exact response grammar and ignore stale or unrelated replies.
    bool consider(const char *data, size_t size, uint32_t source_ip,
                  const char *nonce, const char *wanted_id) {
        if (size == 0 || size > kDatagramLimit || memchr(data, 0, size)) return false;
        char packet[kDatagramLimit + 1];
        memcpy(packet, data, size);
        packet[size] = 0;
        char response_nonce[17] = {}, server_id[65] = {}, port_text[6] = {};
        int consumed = 0;
        if (sscanf(packet, "ULV_SERVER 1 %16s %64s %5s%n",
                   response_nonce, server_id, port_text, &consumed) != 3) return false;
        if (strcmp(response_nonce, nonce) || !valid_id(server_id)) return false;
        if (*wanted_id && strcmp(server_id, wanted_id)) return false;
        for (const char *p = port_text; *p; ++p) if (*p < '0' || *p > '9') return false;
        const unsigned long server_port = strtoul(port_text, nullptr, 10);
        if (!server_port || server_port > 65535) return false;
        char canonical[kDatagramLimit + 1];
        const int length = snprintf(canonical, sizeof(canonical), "ULV_SERVER 1 %s %s %lu\n",
                                    nonce, server_id, server_port);
        if (length != static_cast<int>(size) || memcmp(canonical, data, size)) return false;
        if (!source_ip || (source_ip >> 24) == 0 || (source_ip >> 24) >= 224) return false;
        if (port && (ip != source_ip || port != server_port || strcmp(id, server_id))) {
            ambiguous = true;
        } else if (!port) {
            ip = source_ip;
            port = static_cast<uint16_t>(server_port);
            memcpy(id, server_id, strlen(server_id) + 1);
        }
        return true;
    }

    // Only one distinct matching endpoint may be used after the collection window.
    bool ready() const { return port && !ambiguous; }
};
}
