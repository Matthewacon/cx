#include <cx/test/common/common.h>

#include <cx/allocator.h>

namespace CX {
 //Mock stateless allocator implementation
 template<typename T>
 struct StatelessAllocator final {
  static constexpr bool const Stateless = true;

  static constexpr T& allocate(SizeType) noexcept {
   return *static_cast<T *>(nullptr);
  }

  static constexpr void deallocate(T const&, SizeType) noexcept {}
 };

 //Mock stateful allocator implementation
 template<typename T>
 struct StatefulAllocator final {
  static constexpr bool const Stateless = false;

  constexpr T& allocate(SizeType) noexcept {
   return *static_cast<T *>(nullptr);
  }

  constexpr void deallocate(T const&, SizeType) noexcept {}
 };

 TEST(IsStatelessAllocator, properly_formed_type_satisfies_constraint) {
  EXPECT_TRUE((IsStatelessAllocator<StatelessAllocator>));
 }

 TEST(IsStatelessAllocator, ill_formed_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((IsStatelessAllocator<StatefulAllocator>));
  EXPECT_FALSE((IsStatelessAllocator<VoidT>));
  EXPECT_FALSE((IsStatelessAllocator<Dummy>));
 }

 TEST(IsStatefulAllocator, properly_formed_type_satisfies_constraint) {
  EXPECT_TRUE((IsStatefulAllocator<StatefulAllocator>));
 }

 TEST(IsStatefulAllocator, ill_formed_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((IsStatefulAllocator<StatelessAllocator>));
  EXPECT_FALSE((IsStatefulAllocator<VoidT>));
  EXPECT_FALSE((IsStatefulAllocator<Dummy>));
 }

 TEST(IsAllocator, properly_formed_types_satisfy_constraint) {
  EXPECT_TRUE((IsAllocator<StatefulAllocator>));
  EXPECT_TRUE((IsAllocator<StatelessAllocator>));
 }

 TEST(IsAllocator, ill_formed_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((IsAllocator<VoidT>));
  EXPECT_FALSE((IsAllocator<Dummy>));
 }

 //Tests for the constant-evaluated only allocator
 TEST(ConstexprAllocator, allocator_is_default_constructible_and_destructible) {
  using T = Dummy<>;
  EXPECT_TRUE((Constructible<ConstexprAllocator<T>>));
  EXPECT_TRUE((Destructible<ConstexprAllocator<T>>));
 }

 TEST(ConstexprAllocator, allocator_is_stateless) {
  EXPECT_TRUE((IsStatelessAllocator<ConstexprAllocator>));
 }

 TEST(ConstexprAllocator, runtime_memory_management_yields_an_error) {
  //TODO Convert to expect error
  EXPECT_DEATH(
   [] {
    auto unused = ConstexprAllocator<int>::allocate();
    (void)unused;
   }(),
   ".*"
  );
  //TODO Convert to expect error
  EXPECT_DEATH(
   [] {
    int i = 0;
    ConstexprAllocator<int>::deallocate(i);
   }(),
   ".*"
  );
 }

 TEST(ConstexprAllocator, constant_evaluated_memory_management_does_not_yield_an_error) {
  constexpr auto const r = []() constexpr noexcept {
   auto& p = ConstexprAllocator<int>::allocate(20);
   ConstexprAllocator<int>::deallocate(p, 20);
   return 0;
  }();
  (void)r;
 }

 //TODO Tests for the stl-backed allocator
 /*
 constexpr auto const StlAllocatorCondition = [](auto callback) constexpr {
  #ifdef CX_STL_SUPPORT
   callback();
  #else
   if (!isConstexpr()) {
    GTEST_SKIP();
   }
  #endif
 };
 */

 /*
 TEST(StlAllocator, ) {

 }
 */

 //TODO Tests for the libc-backed allocator

 //TODO Tests for the default allocator
}
