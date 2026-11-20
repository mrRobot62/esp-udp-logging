#pragma once
#include <atomic>
#include <WiFi.h>

namespace tcp_discovery {
// Search within a bounded window; all socket work belongs to the TCP worker.
bool find_server(IPAddress &ip, uint16_t &port, const std::atomic<bool> &stop);
}
