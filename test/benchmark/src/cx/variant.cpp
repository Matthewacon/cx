#include <cx/test/benchmark/common.h>

#include <cx/variant.h>

#include <variant>

namespace CX::Testing {
 template<typename...>
 struct VariantBenchmarkFixture;

 template<>
 struct VariantBenchmarkFixture<> : benchmark::Fixture {
  Variant<> variant;
 };

 template<typename... Types>
 struct VariantBenchmarkFixture : benchmark::Fixture {
  Variant<Types...> variant;
  std::variant<Types...> std_variant;
 };

 using EmptyVariantBenchmarkFixture = VariantBenchmarkFixture<>;

 BENCHMARK_F(EmptyVariantBenchmarkFixture, empty_cx_variant_default_initialization)(benchmark::State &state) {
  using VariantType = decltype(variant);
  for (auto _ : state) {
   variant.~VariantType();
   new (&variant) decltype(variant) {};
  }
 }

 BENCHMARK_TEMPLATE_F(VariantBenchmarkFixture, cx_variant_default_initialization, int, float, char)(benchmark::State &state) {
  using VariantType = decltype(variant);
  for (auto _ : state) {
   variant.~VariantType();
   new (&variant) decltype(variant) {};
  }
 }

 BENCHMARK_TEMPLATE_F(VariantBenchmarkFixture, std_variant_default_initialization, int, float, char)(benchmark::State &state) {
  using VariantType = decltype(std_variant);
  for (auto _ : state) {
   std_variant.~VariantType();
   new (&std_variant) decltype(std_variant) {};
  }
 }

 BENCHMARK_TEMPLATE_F(VariantBenchmarkFixture, cx_variant_copy_initialization, int, float, char)(benchmark::State &state) {
  using VariantType = decltype(variant);
  for (auto _ : state) {
   variant.~VariantType();
   new (&variant) decltype(variant) {1234567};
  }
 }

 BENCHMARK_TEMPLATE_F(VariantBenchmarkFixture, std_variant_copy_initialization, int, float, char)(benchmark::State &state) {
  using VariantType = decltype(std_variant);
  for (auto _ : state) {
   std_variant.~VariantType();
   new (&std_variant) decltype(std_variant) {1234567};
  }
 }

 struct S {
  int data[1234];

  S() = default;
  S(S const&) = delete;
  S(S &&s) {
   memcpy(data, s.data, sizeof(int) * 1234);
   memset(s.data, 0, sizeof(int) * 1234);
  }
 };

 BENCHMARK_TEMPLATE_F(VariantBenchmarkFixture, cx_variant_move_initialization, S)(benchmark::State &state) {
  using VariantType = decltype(variant);
  for (auto _ : state) {
   S s;
   doNotOptimize(&s);
   variant.~VariantType();
   doNotOptimize(new (&variant) decltype(variant) {(S&&)s});
  }
 }

 BENCHMARK_TEMPLATE_F(VariantBenchmarkFixture, std_variant_move_initialization, S)(benchmark::State &state) {
  using VariantType = decltype(std_variant);
  for (auto _ : state) {
   S s;
   doNotOptimize(&s);
   std_variant.~VariantType();
   doNotOptimize(new (&std_variant) decltype(std_variant) {(S&&)s});
  }
 }

 BENCHMARK_TEMPLATE_F(VariantBenchmarkFixture, cx_variant_presence_check, char, long, short *)(benchmark::State &state) {
  for (auto _ : state) {
   doNotOptimize(variant.has<short *>());
  }
 }

 BENCHMARK_TEMPLATE_F(VariantBenchmarkFixture, std_variant_presence_check, char, long, short *)(benchmark::State &state) {
  for (auto _ : state) {
   doNotOptimize(std::holds_alternative<short *>(std_variant));
  }
 }

 //TODO benchmarks for the remaining variant features
}
