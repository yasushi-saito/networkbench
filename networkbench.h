#pragma once
#include <cstdint>
#include <string>

namespace networkbench {
// MessageHeader is sent at the start of every packet sent from client->server.
// The packet sent from server->client doesn't contain the header; it's just raw
// bytes of size send_bytes.
struct MessageHeader {
  // Number of bytes trailing this header.
  int64_t msg_bytes;
  // Number of bytes to send back.
  int64_t send_bytes;
};

// Result of a benchmark run.
struct Result {
  // Number of packets sent or received.
  //
  // Note: the total number of packets exchanged between the endpoints will be 2
  // * n_msgs.
  int n_msgs;
  // The size of each packet sent, in bytes.
  int64_t send_bytes;
  // The size of each packet received, in bytes.
  int64_t recv_bytes;
  // Total elapsed time, in seconds.
  double elapsed_s;
};

}  // namespace networkbench
