#include "tcp.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <memory>
#include <thread>

#include "absl/cleanup/cleanup.h"
#include "networkbench.h"
#include "spdlog/spdlog.h"
#include "timer.h"
#include "util.h"

namespace networkbench {
namespace {
bool Read(int fd, void* buf, int buf_size) {
  uint8_t* b = static_cast<uint8_t*>(buf);
  while (buf_size > 0) {
    int n = read(fd, b, buf_size);
    if (n <= 0) {
      if (errno == EINTR) continue;
      perror("read");
      return false;
    }
    b += n;
    buf_size -= n;
  }
  return true;
}

bool Write(int fd, void* buf, int buf_size, int flags) {
  uint8_t* b = static_cast<uint8_t*>(buf);
  while (buf_size > 0) {
    const int n = send(fd, b, buf_size, flags);
    if (n < 0) {
      if (errno == EINTR) continue;
      perror("write");
      return false;
    }
    b += n;
    buf_size -= n;
  }
  return true;
}

}  // namespace
}  // namespace networkbench

void networkbench::RunTCPServer(int fd) {
  int64_t n_read = 0, n_written = 0;
  auto d = absl::MakeCleanup([&]() {
    spdlog::info("closing socket {}, n_read={}, n_written={}", fd, n_read,
                 n_written);
    close(fd);
  });
  constexpr int64_t kBufSize = 4 << 20;
  auto buf = NewBuf(kBufSize, 0);
  for (;;) {
    MessageHeader h;
    if (!Read(fd, &h, sizeof h)) {
      return;
    }
    int64_t remaining = h.msg_bytes;
    while (remaining > 0) {
      const auto to_read = std::min(kBufSize, remaining);
      if (!Read(fd, buf.get(), to_read)) {
        return;
      }
      remaining -= to_read;
      n_read += to_read;
    }
    remaining = h.send_bytes;
    while (remaining > 0) {
      const auto to_write = std::min(kBufSize, remaining);
      if (!Write(fd, buf.get(), to_write,
                 to_write <= remaining ? 0 : MSG_MORE)) {
        return;
      }
      remaining -= to_write;
      n_written += to_write;
    }
  }
}

networkbench::Result networkbench::RunTCPClient(const std::string& host,
                                                int port, int n_msgs,
                                                int64_t send_bytes,
                                                int64_t recv_bytes) {
  int err;
  struct hostent hbuf, *hent;
  char hent_buf[4096];
  if (gethostbyname_r(host.c_str(), &hbuf, hent_buf, sizeof hent_buf, &hent,
                      &err) < 0) {
    perror("gethostbyname_r");
    abort();
  }
  int fd = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof sin);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, reinterpret_cast<const sockaddr*>(&sin), sizeof sin) != 0) {
    perror("bind");
    abort();
  }

  memset(&sin, 0, sizeof sin);
  sin.sin_family = AF_INET;
  sin.sin_addr = *reinterpret_cast<struct in_addr*>(hent->h_addr);
  sin.sin_port = htons(port);
  if (connect(fd, reinterpret_cast<sockaddr*>(&sin), sizeof sin) < 0) {
    perror("connect");
    abort();
  }
  constexpr int64_t kBufSize = 4 << 20;
  auto buf = NewBuf(kBufSize, 0);

  auto round_trip = [&]() {
    MessageHeader h = {.msg_bytes = send_bytes, .send_bytes = recv_bytes};
    if (!Write(fd, &h, sizeof h, MSG_MORE)) {
      perror("write");
      abort();
    }
    auto remaining = send_bytes;
    while (remaining > 0) {
      const auto to_write = std::min(kBufSize, remaining);
      if (!Write(fd, buf.get(), to_write,
                 to_write <= remaining ? 0 : MSG_MORE)) {
        abort();
      }
      remaining -= to_write;
    }
    remaining = h.msg_bytes;
    while (remaining > 0) {
      const auto to_read = std::min(kBufSize, remaining);
      if (!Read(fd, buf.get(), to_read)) {
        abort();
      }
      remaining -= to_read;
    }
  };

  Timer timer_all;
  timer_all.Start();
  for (;;) {
    round_trip();
    if (timer_all.ElapsedSeconds() > 2) break;
  }
  spdlog::info("start measuring");
  Timer timer_net;
  timer_net.Start();
  for (int i = 0; i < n_msgs; i++) {
    round_trip();
  }
  Result r{.n_msgs = n_msgs,
           .send_bytes = send_bytes,
           .recv_bytes = recv_bytes,
           .elapsed_s = timer_net.ElapsedSeconds()};
  return r;
}
