#pragma once

// Simple echo client/server using io_uring over TCP/IO.

#include <string>

#include "networkbench.h"

namespace networkbench {
// Start a server at the given port. This function never returns.
void RunIoUringServer(int port);

// Run a benchmark against the given host:port. It sends a packet of size
// send_bytes, then receives a packet of size recv_bytes, and repeat that n_msgs
// times. The RunTCPServer server must be running at the given host:port
// already.
Result RunIoUringClient(const std::string& host, int port, int n_msgs,
                        int64_t send_bytes, int64_t recv_bytes);
}  // namespace networkbench
