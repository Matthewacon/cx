#include <cx/test/benchmark/common.h>

#include <cx/lambda.h>
#include <cx/vararg.h>

#include <functional>

namespace CX::Testing {
 //Constants for c-variadic lambda tests
 using VarargType = long double;
 constexpr auto const VarargCount = 10;

 //Argument generator for c-variadic lambda benchmark cases
 template<typename T = VarargType, auto N = VarargCount>
 struct VarargInvoker {
  template<typename... Args>
  static auto invoke(auto op, Args... args) {
   return VarargInvoker<T, N - 1>::template invoke(op, args..., T{});
  }
 };

 template<typename T>
 struct VarargInvoker<T, 0> {
  template<typename... Args>
  static auto invoke(auto op, Args... args) {
   return op(args...);
  }
 };

 //Creates an empty lambda with the supplied prototype
 template<typename>
 struct EmptyLambdaGenerator;

 template<typename R, typename... Args>
 struct EmptyLambdaGenerator<R (Args...)> {
  static constexpr auto const Value = [](Args... args) -> R {
   doNotOptimize(args...);
   benchmark::ClobberMemory();
   if constexpr (SameType<R, void>) {
    return []<typename...>() constexpr noexcept {}();
   } else {
    return []<typename...>() constexpr noexcept {
     return R{};
    }();
   }
  };
 };

 template<typename R, typename... Args>
 struct EmptyLambdaGenerator<R (Args...) noexcept> {
  static constexpr auto const Value = [](Args... args) noexcept -> R {
   doNotOptimize(args...);
   benchmark::ClobberMemory();
   if constexpr (SameType<R, void>) {
    return []<typename...>() constexpr noexcept {}();
   } else {
    return [&]<typename...>() constexpr noexcept {
     return R{};
    }();
   }
  };
 };

 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wvarargs"

 template<typename R, typename... Args>
 struct EmptyLambdaGenerator<R (Args..., ...)> {
  static constexpr auto const Value = [](Args... args, ...) {
   CX::VaList list;
   va_start(list, argumentAtIndex<sizeof...(Args) - 1>(args...));
   for (int i = 0; i < VarargCount; i++) {
    doNotOptimize(list.arg<VarargType>());
   }
   doNotOptimize(args...);
   benchmark::ClobberMemory();
   if constexpr (SameType<R, void>) {
    return []<typename...>() constexpr noexcept {}();
   } else {
    return [&]<typename...>() constexpr noexcept {
     return R{};
    }();
   }
  };
 };

 template<typename R, typename... Args>
 struct EmptyLambdaGenerator<R (Args..., ...) noexcept> {
  static constexpr auto const Value = [](Args... args, ...) noexcept -> R {
   CX::VaList list;
   va_start(list, argumentAtIndex<sizeof...(Args) - 1>(args...));
   for (int i = 0; i < VarargCount; i++) {
    doNotOptimize(list.arg<VarargType>());
   }
   doNotOptimize(args...);
   benchmark::ClobberMemory();
   return R{};
  };
 };

 #pragma GCC diagnostic pop

 //CX::Lambda<...> fixture
 template<typename Prototype, bool StdFunction>
 struct LambdaBenchmarkFixture;

 template<typename Prototype>
 struct LambdaBenchmarkFixture<Prototype, false> : benchmark::Fixture {
  static constexpr auto const &emptyLambda = EmptyLambdaGenerator<Prototype>::Value;
  Lambda<Prototype> lambda;

  void SetUp(benchmark::State&) {
   //Set up lambda
   lambda.~decltype(lambda)();
   new (&lambda) Lambda<Prototype> {emptyLambda};
  }
 };

 template<typename Prototype>
 struct LambdaBenchmarkFixture<Prototype, true> : LambdaBenchmarkFixture<Prototype, false> {
  static constexpr auto const &emptyLambda = LambdaBenchmarkFixture<Prototype, false>::emptyLambda;
  std::function<Prototype> function;

  void SetUp(benchmark::State &state) override {
   //Set up lambda
   LambdaBenchmarkFixture<Prototype, false>::SetUp(state);

   //Set up function
   function.~decltype(function)();
   new (&function) std::function<Prototype> {emptyLambda};
  }
 };

 BENCHMARK_TEMPLATE_F(LambdaBenchmarkFixture, direct_invocation, char (int), true)(benchmark::State &state) {
  for (auto _ : state) {
   doNotOptimize(emptyLambda(1234));
  }
 }

 BENCHMARK_TEMPLATE_F(LambdaBenchmarkFixture, lambda_invocation, char (int), true)(benchmark::State &state) {
  for (auto _ : state) {
   doNotOptimize(lambda(1234));
  }
 }

 BENCHMARK_TEMPLATE_F(LambdaBenchmarkFixture, std_function_invocation, char (int), true)(benchmark::State &state) {
  for (auto _ : state) {
   doNotOptimize(function(1234));
  }
 }

 BENCHMARK_TEMPLATE_F(LambdaBenchmarkFixture, noexcept_direct_invocation, float (double) noexcept, false)(benchmark::State &state) {
  for (auto _ : state) {
   doNotOptimize(emptyLambda(3.14));
  }
 }

 BENCHMARK_TEMPLATE_F(LambdaBenchmarkFixture, noexcept_lambda_invocation, float (double) noexcept, false)(benchmark::State &state) {
  for (auto _ : state) {
   doNotOptimize(lambda(3.14));
  }
 }

 BENCHMARK_TEMPLATE_F(LambdaBenchmarkFixture, c_variadic_direct_invocation, short (int, ...), false)(benchmark::State &state) {
  VarargInvoker<>::invoke(
   [&]<typename... Args>(int i, Args... args) {
    for (auto _ : state) {
     doNotOptimize(emptyLambda(i, args...));
    }
   },
   135
  );
 }

 BENCHMARK_TEMPLATE_F(LambdaBenchmarkFixture, c_variadic_lambda_invocation, short (int, ...), false)(benchmark::State &state) {
  VarargInvoker<>::invoke(
   [&]<typename... Args>(int i, Args... args) {
    for (auto _ : state) {
     doNotOptimize(lambda(i, args...));
    }
   },
   135
  );
 }

 BENCHMARK_TEMPLATE_F(LambdaBenchmarkFixture, noexcept_c_variadic_direct_invocation, double (char, ...) noexcept, false)(benchmark::State &state) {
  VarargInvoker<>::invoke(
   [&]<typename... Args>(int i, Args... args) {
    for (auto _ : state) {
     doNotOptimize(emptyLambda(i, args...));
    }
   },
   135
  );
 }

 BENCHMARK_TEMPLATE_F(LambdaBenchmarkFixture, noexcept_c_variadic_lambda_invocation, double (char, ...) noexcept, false)(benchmark::State &state) {
  VarargInvoker<>::invoke(
   [&]<typename... Args>(int i, Args... args) {
    for (auto _ : state) {
     doNotOptimize(lambda(i, args...));
    }
   },
   135
  );
 }

 //TODO CX::AllocLambda<...> fixture
 //TODO C-Variadic lambda to fptr conversion fixture
 //TODO C-Variadic lambda to unsafe fptr conversion fixture
 //TODO Lambda/AllocLambda/std::function initialization benchmarks
}
