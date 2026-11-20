#include "tcp/discovery_protocol.h"
#include <assert.h>
#include <string>

// Exercise the exact wire contract, deduplication and explicit server selection.
int main() {
    const char *nonce = "0123456789abcdef";
    const std::string first = "ULV_SERVER 1 0123456789abcdef viewer-1 10514\n";
    const std::string second = "ULV_SERVER 1 0123456789abcdef viewer-2 10515\n";
    tcp_discovery::Selection selection;
    assert(selection.consider(first.data(), first.size(), 0xc0a80001, nonce, ""));
    assert(selection.ready() && selection.port == 10514);
    assert(selection.consider(first.data(), first.size(), 0xc0a80001, nonce, ""));
    assert(selection.ready());
    assert(selection.consider(second.data(), second.size(), 0xc0a80002, nonce, ""));
    assert(!selection.ready());
    tcp_discovery::Selection filtered;
    assert(!filtered.consider(second.data(), second.size(), 0xc0a80002, nonce, "viewer-1"));
    assert(filtered.consider(first.data(), first.size(), 0xc0a80001, nonce, "viewer-1"));
    assert(filtered.ready());
    const char *invalid[] = {
        "ULV_SERVER 2 0123456789abcdef viewer-1 10514\n",
        "ULV_SERVER 1 0123456789abcdee viewer-1 10514\n",
        "ULV_SERVER 1 0123456789abcdef viewer-1 0\n",
        "ULV_SERVER 1 0123456789abcdef viewer-1 65536\n",
        "ULV_SERVER 1 0123456789abcdef viewer-1 +123\n",
        "ULV_SERVER 1 0123456789abcdef viewer-1 0123\n",
        "ULV_SERVER 1 0123456789abcdef viewer-1 123\nextra",
        "ULV_SERVER 1 0123456789abcdef viewer-1 123",
    };
    for (const char *packet : invalid) {
        tcp_discovery::Selection reject;
        assert(!reject.consider(packet, strlen(packet), 0xc0a80001, nonce, ""));
        assert(!reject.ready());
    }
    const std::string oversized(193, 'x');
    assert(!filtered.consider(oversized.data(), oversized.size(), 0xc0a80001, nonce, ""));
    assert(!filtered.consider(first.data(), first.size(), 0xffffffff, nonce, ""));
}
