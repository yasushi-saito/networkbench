#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <iostream>
#include <regex>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/numbers.h"
#include "spdlog/spdlog.h"
#include "tcp.h"
#include "io_uring.h"

ABSL_FLAG(int, tcp_server_port, -1, R"(Run a TCP server at the given port.
It will bind to interface 0.0.0.0.)");

ABSL_FLAG(int, uring_server_port, -1,
          R"(Run a io_uring TCP server at the given port.
It will bind to interface 0.0.0.0.)");

ABSL_FLAG(std::string, tcp_client, "localhost:9999",
          R"(Run a TCP client and connect to the given server.)");

ABSL_FLAG(int, n_msgs, 1000, R"(Number of messages to send)");
ABSL_FLAG(int64_t, send_bytes, 1000, R"(Number of messages to send)");
ABSL_FLAG(int64_t, recv_bytes, 1000, R"(Number of messages to send)");

namespace {
void AcceptTCPConnections(int port, std::function<void(int fd)> fn) {
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    abort();
  }
  int on = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) != 0) {
    perror("setsockopt");
    abort();
  }
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof sin);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_port = htons(port);
  if (bind(sock, reinterpret_cast<const sockaddr*>(&sin), sizeof sin) != 0) {
    perror("bind");
    abort();
  }
  if (listen(sock, 128) != 0) {
    perror("listen");
    abort();
  }
  spdlog::info("started TCP server at 0.0.0.0:{}", port);
  for (;;) {
    struct sockaddr_in sin;
    socklen_t sin_len = sizeof sin;
    int fd = accept(sock, reinterpret_cast<sockaddr*>(&sin), &sin_len);
    if (fd < 0) {
      if (errno == EINTR) continue;
      perror("accept");
      abort();
    }
    spdlog::info("accepted connection {}", fd);
    std::thread th([fd, fn]() { fn(fd); });
    th.detach();
  }
}

void ParseHostPort(const std::string& host_port, std::string* host, int* port) {
  std::regex re("^([^:]+):(\\d+)$");
  std::smatch matches;
  if (!std::regex_match(host_port, matches, re)) {
    spdlog::critical("invalid host:port pattern: '{}'", host_port);
    abort();
  }
  *host = matches[1];
  std::string port_str = matches[2];
  if (!absl::SimpleAtoi(port_str, port)) {
    spdlog::critical("invalid port pattern: '{}'", port_str);
    abort();
  }
}
}  // namespace

int main(int argc, char** argv) {
  auto unparsed_args = absl::ParseCommandLine(argc, argv);

  if (auto port = absl::GetFlag(FLAGS_tcp_server_port); port > 0) {
    AcceptTCPConnections(port, networkbench::RunTCPServer);
    return 0;
  }
  if (auto port = absl::GetFlag(FLAGS_uring_server_port); port > 0) {
    AcceptTCPConnections(port, networkbench::RunIoUringServer);
    return 0;
  }

  if (auto tcp_client = absl::GetFlag(FLAGS_tcp_client); !tcp_client.empty()) {
    std::string host;
    int port;
    ParseHostPort(tcp_client, &host, &port);
    auto r = networkbench::RunTCPClient(host, port, absl::GetFlag(FLAGS_n_msgs),
                                        absl::GetFlag(FLAGS_send_bytes),
                                        absl::GetFlag(FLAGS_recv_bytes));
    std::cout << "tcp, " << r.n_msgs << ", " << r.send_bytes << ", "
              << r.recv_bytes << ", " << r.elapsed_s << "\n";
  }
}
