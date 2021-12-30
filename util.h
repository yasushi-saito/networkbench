#pragma once
#include <memory>
namespace networkbench {

std::unique_ptr<uint8_t[]> NewBuf(int bytes, int seed);

}
