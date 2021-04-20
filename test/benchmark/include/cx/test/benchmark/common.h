#include <cx/test/common/common.h>

#include <benchmark/benchmark.h>

namespace CX::Testing {
 template<typename... Args>
 void doNotOptimize(Args...) noexcept;

 template<typename T, typename... Args>
 void doNotOptimize(T t, Args... args) noexcept {
  benchmark::DoNotOptimize(t);
  doNotOptimize(args...);
 }

 template<>
 inline void doNotOptimize() noexcept {}
}
