#include "io_uring.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <memory>
#include <thread>

#include "absl/cleanup/cleanup.h"
#include "liburing.h"
#include "liburing/io_uring.h"
#include "networkbench.h"
#include "spdlog/spdlog.h"
#include "timer.h"
#include "util.h"

void networkbench::RunIoUringServer(int fd) {
  constexpr int kFdIndex = 0;
  constexpr int kGroupId = 3333;

  auto do_recv = [](struct io_uring* ring, void* buf, int bytes) -> int {
    uint8_t* b = static_cast<uint8_t*>(buf);

    struct io_uring_cqe* cqe;
    auto* sqe = io_uring_get_sqe(ring);
    io_uring_prep_recv(sqe, kFdIndex, b, bytes, 0);
    // io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE | IOSQE_BUFFER_SELECT);
    io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);
    sqe->buf_group = kGroupId;
    io_uring_submit(ring);
    io_uring_wait_cqe(ring, &cqe);
    int res = cqe->res;
    io_uring_cqe_seen(ring, cqe);
    if (cqe->res <= 0) {
      spdlog::critical("read: {}", strerror(-cqe->res));
    }
    return res;
  };

  auto do_send = [](struct io_uring* ring, void* buf, int bytes) -> int {
    uint8_t* b = static_cast<uint8_t*>(buf);
    struct io_uring_cqe* cqe;
    auto* sqe = io_uring_get_sqe(ring);
    io_uring_prep_send(sqe, kFdIndex, b, bytes, MSG_MORE);
    io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);
    sqe->buf_group = kGroupId;
    io_uring_submit(ring);
    io_uring_wait_cqe(ring, &cqe);
    int res = cqe->res;
    io_uring_cqe_seen(ring, cqe);
    if (cqe->res <= 0) {
      spdlog::critical("read: {}", strerror(-cqe->res));
    }
    return res;
  };

  struct io_uring ring;
  struct io_uring_params params;
  memset(&params, 0, sizeof params);
  int ring_fd = io_uring_queue_init_params(2, &ring, &params);
  if (ring_fd < 0) {
    perror("io_uring_setup");
    abort();
  }
#if 1
  if (io_uring_register_files(&ring, &fd, 1) != 0) {
    perror("io_uring_register_files");
    abort();
  }
#endif

  struct io_uring_probe* probe;
  probe = io_uring_get_probe_ring(&ring);
  if (!probe || !io_uring_opcode_supported(probe, IORING_OP_PROVIDE_BUFFERS)) {
    printf("Buffer select not supported, skipping...\n");
    exit(0);
  }
  free(probe);

#if 0
  constexpr int kBufSize = 16384;
  constexpr int kNBufs = 16;
  char bufs[kNBufs][kBufSize];
  auto* sqe = io_uring_get_sqe(&ring);
  spdlog::info("registering {} buffers of {} bytes each", kNBufs, kBufSize);
  io_uring_prep_provide_buffers(sqe, bufs, kBufSize, kNBufs, kGroupId, 0);
  io_uring_submit(&ring);
  struct io_uring_cqe* cqe;
  io_uring_wait_cqe(&ring, &cqe);
  if (cqe->res != 0) {
    perror("io_uring_prep");
    abort();
  }
  io_uring_cqe_seen(&ring, cqe);
#endif
  constexpr int kBufSize = 4 << 20;
  auto buf = NewBuf(kBufSize, 1);

  for (;;) {
    MessageHeader hdr;
    if (do_recv(&ring, &hdr, sizeof(MessageHeader)) <= 0) break;
    int remaining = hdr.msg_bytes;
    uint8_t* b = buf.get();
    while (remaining > 0) {
      int n = std::min<int>(remaining, kBufSize);
      auto got = do_recv(&ring, buf.get(), n);
      if (got <= 0) {
        spdlog::critical("io_uring read: {}!={}, data={}-{}-{}", got, n,
                         int(buf[0]), int(buf[1]), int(buf[2]));
        abort();
      }
      b += got;
      remaining -= got;
    }
    remaining = hdr.send_bytes;
    b = buf.get();
    while (remaining > 0) {
      int n = std::min<int>(remaining, kBufSize);
      auto got = do_send(&ring, buf.get(), n);
      if (got != n) {
        spdlog::critical("io_uring write: {}!={}, data={}-{}-{}", got, n,
                         int(buf[0]), int(buf[1]), int(buf[2]));
        abort();
      }
      b += got;
      remaining -= got;
    }
  }
  spdlog::info("done reading");
}

networkbench::Result networkbench::RunIoUringClient(const std::string& host,
                                                    int port, int n_msgs,
                                                    int64_t send_bytes,
                                                    int64_t recv_bytes) {
  abort();
}
