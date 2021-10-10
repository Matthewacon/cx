#include <cx/test/common/common.h>

#include <cx/tuple.h>

namespace CX::Testing {
 //Supporting concepts
 template<typename Tuple, SizeType Index>
 concept TupleGetIsValid = requires (Tuple t) {
  t.template get<Index>();
 };

 template<typename Tuple, SizeType Index>
 concept TupleRgetIsValid = requires (Tuple t) {
  t.template rget<Index>();
 };

 TEST(Tuple, empty_tuple_noexcept_constructible) {
  EXPECT_TRUE((noexcept(Tuple{})));
 }

 TEST(Tuple, empty_tuple_noexcept_destructible) {
  EXPECT_TRUE((noexcept(Tuple{}.~Tuple())));
 }

 TEST(Tuple, empty_tuple_get_is_invalid) {
  EXPECT_FALSE((TupleGetIsValid<Tuple<>, 0>));
 }

 TEST(Tuple, empty_tuple_rget_is_invalid) {
  FAIL();
 }

 TEST(Tuple, empty_tuple_concat_empty_tuple_yields_empty_tuple) {
  FAIL();
 }

 TEST(Tuple, empty_tuple_concat_populated_tuple_yields_populated_tuple) {
  FAIL();
 }

 TEST(Tuple, populated_tuple_with_noexcept_types_is_noexcept_constructible) {
  struct A {
   A() noexcept = default;
   ~A() noexcept = default;
  };
  EXPECT_TRUE((noexcept(Tuple<int, float, A>{
   0,
   0.0,
   A{}
  })));
 }

 TEST(Tuple, populated_tuple_with_noexcept_types_is_noexcept_destructible) {
  struct A {
   A() noexcept = default;
   ~A() noexcept = default;
  };
  EXPECT_TRUE((noexcept(Tuple<void *, char, A, int>{
   nullptr,
   'f',
   A{},
   0
  }.~Tuple())));
 }

 TEST(Tuple, populated_tuple_with_non_noexcept_types_is_not_noexcept_constructible) {
  FAIL();
 }

 TEST(Tuple, populated_tuple_with_non_noexcept_types_is_not_noexcept_destructible) {
  FAIL();
 }

 //TODO remaining tuple tests
 TEST(Tuple, ooga_booga) {
  Tuple t{123, 1.23, "hello world"};
  auto [v1, v2, v3] = t;
  printf(
   "%d %f '%s'\n",
   v1,
   v2,
   v3
  );
  printf("%s\n", typeid(t).name());
 }
}
