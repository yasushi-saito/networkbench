#pragma once
#include <chrono>

namespace networkbench {
/// A very basic encapsulation of chrono to report
/// timings
class Timer {
 public:
  void Start() { start_ = std::chrono::high_resolution_clock::now(); }

  double ElapsedSeconds() {
    const auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(now -
                                                                     start_)
        .count();
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

}  // namespace networkbench
