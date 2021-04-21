#include <cx/test/benchmark/common.h>

#include <cx/vararg.h>

#include <cstdarg>

namespace CX::Testing {
 //Constants
 using VarargType = long double;

 //Argument generator
 template<auto N, typename T = VarargType>
 struct VarargInvoker {
  template<typename... Args>
  static void invoke(auto op, Args... args) {
   VarargInvoker<N - 1, T>::template invoke(op, args..., T{});
  }
 };

 template<typename T>
 struct VarargInvoker<0, T> {
  template<typename... Args>
  static void invoke(auto op, Args... args) {
   op(args...);
  }
 };

 template<auto N>
 struct VarargsBenchmarkFixture : benchmark::Fixture {
  void runva_list(benchmark::State &state) {
   VarargInvoker<N>::invoke([&]<typename... Args>(Args... args) {
    for (auto _ : state) {
     [](int i, ...) {
      va_list list;
      va_start(list, i);
      for (int c = 0; c < N; c++) {
       doNotOptimize(va_arg(list, VarargType));
      }
      va_end(list);
     }(0, args...);
    }
   });
  }

  void runCXVaList(benchmark::State &state) {
   VarargInvoker<N>::invoke([&]<typename... Args>(Args... args) {
    for (auto _ : state) {
     [](int i, ...) {
      CX::VaList list;
      va_start(list, i);
      for (int c = 0; c < N; c++) {
       doNotOptimize(list.arg<VarargType>());
      }
     }(0, args...);
    }
   });
  }
 };

 BENCHMARK_TEMPLATE_F(VarargsBenchmarkFixture, small_va_list, 5)(benchmark::State &state) {
  runva_list(state);
 }

 BENCHMARK_TEMPLATE_F(VarargsBenchmarkFixture, small_cx_va_list, 5)(benchmark::State &state) {
  runCXVaList(state);
 }

 BENCHMARK_TEMPLATE_F(VarargsBenchmarkFixture, medium_va_list, 50)(benchmark::State &state) {
  runva_list(state);
 }

 BENCHMARK_TEMPLATE_F(VarargsBenchmarkFixture, medium_cx_va_list, 50)(benchmark::State &state) {
  runCXVaList(state);
 }

 BENCHMARK_TEMPLATE_F(VarargsBenchmarkFixture, large_va_list, 500)(benchmark::State &state) {
  runva_list(state);
 }

 BENCHMARK_TEMPLATE_F(VarargsBenchmarkFixture, large_cx_va_list, 500)(benchmark::State &state) {
  runCXVaList(state);
 }
}
