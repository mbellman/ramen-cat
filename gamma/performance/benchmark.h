#pragma once

#include <chrono>
#include <functional>

#include "system/type_aliases.h"

u64 Gm_GetMicroseconds();

namespace Gamma {
  void Gm_CompareBenchmarks(u64 a, u64 b);

  auto Gm_CreateTimer() {
    auto start = std::chrono::system_clock::now();

    return [&]() {
      auto end = std::chrono::system_clock::now();

      std::chrono::system_clock::duration duration = end - start;

      return duration;
    };
  };

  u64 Gm_RepeatBenchmarkTest(const std::function<void()>& test, u32 times = 1);
  u64 Gm_RunBenchmarkTest(const std::function<void()>& test);
  void Gm_RunLoopedBenchmarkTest(const std::function<void()>& test, u32 pause = 1000);
  void Gm_Sleep(u32 milliseconds);
}