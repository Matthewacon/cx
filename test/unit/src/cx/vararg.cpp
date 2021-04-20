#include <cx/test/common/common.h>

#include <stdlib.h>

//Disable libc support for tests
#undef va_start
#undef va_arg
#undef va_end
#undef CX_LIBC_SUPPORT
#define CX_VARARG_INTRENSICS
#include <cx/vararg.h>

namespace CX::Testing {
 TEST(VarargPromoted, types_smaller_than_or_equal_to_int_are_promoted_to_int) {
  EXPECT_TRUE((SameType<Internal::VarargPromoted<char>, int>));
  EXPECT_TRUE((SameType<Internal::VarargPromoted<char8_t>, int>));
  EXPECT_TRUE((SameType<Internal::VarargPromoted<char16_t>, int>));
  EXPECT_TRUE((SameType<Internal::VarargPromoted<char32_t>, int>));
  EXPECT_TRUE((SameType<Internal::VarargPromoted<short>, int>));
  EXPECT_TRUE((SameType<Internal::VarargPromoted<bool>, int>));
 }

 TEST(VarargPromoted, float_promoted_to_double) {
  EXPECT_TRUE((SameType<Internal::VarargPromoted<float const volatile>, double const volatile>));
 }

 TEST(VarargPromoted, non_promotable_types_are_unmodified) {
  EXPECT_TRUE((SameType<Internal::VarargPromoted<long double const volatile>, long double const volatile>));
  EXPECT_TRUE((SameType<Internal::VarargPromoted<int[123]>, int[123]>));
 }

 template<auto N, typename T>
 struct GenerateRandomValuesAndInvoke {
  template<typename... Args>
  static void invoke(auto &op, Args... args) {
   auto value = (T)rand();
   GenerateRandomValuesAndInvoke<N - 1, T>::invoke(op, args..., value);
  }
 };

 template<typename T>
 struct GenerateRandomValuesAndInvoke<0, T> {
  template<typename... Args>
  static void invoke(auto &op, Args... args) {
   op(sizeof...(Args), args...);
  }
 };

 TEST(Vararg, cx_valist_behaves_identically_to_va_list) {
  using ExpectedType = long double;
  constexpr auto const Size = 100;

  //Compare values retrieved from the platform list
  //to those retrieved from `CX::VaList`
  auto op = [&](int n, ...) {
   //Initialize platform list
   va_list platformList;
   va_start(platformList, n);

   //Initialize uniform list
   VaList uniformList;
   va_start(uniformList, n);

   for (int i = 0; i < n; i++) {
    EXPECT_EQ(
     ((ExpectedType)va_arg(platformList, ExpectedType)),
     (uniformList.arg<ExpectedType>())
    );
   }

   //Destruct platform list
   //Note: No need to destruct `uniformList`,
   //as it is managed
   va_end(platformList);
  };

  //Generate `Size` number of random elements and invoke `op` with
  //them
  GenerateRandomValuesAndInvoke<Size, ExpectedType>::invoke(op);
 }

 template<auto Size, typename E, auto N = Size>
 struct PullElementsAndInvoke {
  template<typename... Args>
  static void invoke(auto &op, E (&elements)[Size], Args... args) {
   PullElementsAndInvoke<Size, E, N - 1>::invoke(
    op,
    elements,
    args...,
    elements[Size - N]
   );
  }
 };

 template<auto Size, typename E>
 struct PullElementsAndInvoke<Size, E, 0> {
  template<typename... Args>
  static void invoke(auto &op, E (&)[Size], Args... args) {
   op(Size, args...);
  }
 };

 TEST(Vararg, cx_valist_behaves_correctly_when_passed_by_reference) {
  using ExpectedType = short;
  constexpr auto const Size = 5;
  ExpectedType data[Size];

  //Fill `data` with random values
  for (auto i = 0; i < Size; i++) {
   data[i] = (ExpectedType)rand();
  }

  std::function<void (int, VaList&)> l1, l2;

  //Pops argument from list and invokes `l2`
  l1 = [&](int n, VaList &list) {
   if (n > 0) {
    auto value = list.arg<ExpectedType>();
    EXPECT_EQ(value, data[Size - n]);
    l2(n - 1, list);
   }
  };

  //Pops argumnet from list and invokes `l1`
  l2 = [&](int n, VaList &list) {
   if (n > 0) {
    auto value = list.arg<ExpectedType>();
    EXPECT_EQ(value, data[Size - n]);
    l1(n - 1, list);
   }
  };

  //Sets up list and invokes `l2`
  auto l3 = [&](int n, ...) {
   VaList list;
   va_start(list, n);
   l2(n, list);
  };

  //Unrolls `data` and invokes `l3` with all elements of `data`
  PullElementsAndInvoke<Size, ExpectedType>::invoke(l3, data);
 }

 TEST(Vararg, cx_valist_is_copy_constructible) {
  EXPECT_TRUE((CopyConstructible<VaList>));
 }

 TEST(Vararg, cx_valist_is_copy_assignable) {
  EXPECT_TRUE((CopyAssignable<VaList>));
 }
}
