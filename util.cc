#include "util.h"

std::unique_ptr<uint8_t[]> networkbench::NewBuf(int bytes, int seed) {
  auto buf = std::make_unique<uint8_t[]>(bytes);
  uint8_t val = seed;
  for (int i = 0; i < bytes; i++) {
    buf[i] = val++;
  }
  return buf;
}
