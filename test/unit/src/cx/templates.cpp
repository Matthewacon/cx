#include <cx/test/common/common.h>

#include <cx/templates.h>

namespace CX {
 TEST(MaxValue, identically_typed_value_pack_yields_expected_value) {
  EXPECT_EQ((MaxValue<1, -2, 3, 100, -23 -100, 1000>), 1000);
 }

 TEST(MaxValue, uniquely_typed_value_pack_yields_expected_value) {
  EXPECT_EQ((MaxValue<(short)123, (char)32767, (int)-3124, (long)12345>), 12345);
 }

 TEST(MaxValue, special_cases_yield_expected_value) {
  //Empty pack should yield `0`
  EXPECT_EQ((MaxValue<>), 0);

  //Single argument pack should yield the argument
  EXPECT_EQ((MaxValue<1234>), 1234);
 }

 TEST(MinValue, identically_typed_value_pack_yields_expected_value) {
  EXPECT_EQ((MinValue<1, -2, 3, 100, -23, -100, 1000>), -100);
 }

 TEST(MinValue, uniquely_typed_value_pack_yields_expected_value) {
  EXPECT_EQ((MinValue<0, (unsigned char)40000, (int)-67890, (long long)-12345, (short)2435>), -67890);
 }

 TEST(MinValue, special_cases_yield_expected_value) {
  //Empty pack should yield `0`
  EXPECT_EQ((MinValue<>), 0);

  //Single argument pack should yield the argument
  EXPECT_EQ((MinValue<1234>), 1234);
 }

 TEST(MaxTypeSize, type_pack_yields_expected_value) {
  EXPECT_EQ((MaxTypeSize<bool, char, float, int, double>), sizeof(double));
  EXPECT_EQ((MaxTypeSize<char *, short, char[12345], int>), sizeof(char[12345]));
 }

 TEST(MaxTypeSize, special_cases_yield_default_value) {
  EXPECT_EQ((MaxTypeSize<>), 0);
 }

 TEST(MinTypeSize, type_pack_yields_expected_value) {
  EXPECT_EQ((MinTypeSize<double, short, long long, int>), sizeof(long long));
  EXPECT_EQ((MinTypeSize<void *, int, long double[123]>), sizeof(long double[123]));
 }

 TEST(MinTypeSize, special_cases_yield_default_value) {
  EXPECT_EQ((MinTypeSize<>), 0);
 }

 TEST(IndexOfType, type_pack_yeilds_expected_value) {
  EXPECT_EQ((IndexOfType<float, char, void, int, float, double, long>), 3);
  EXPECT_EQ((IndexOfType<char, Dummy<>, VoidT<>, char, int, Dummy<int>>), 2);
 }

 TEST(IndexOfType, special_cases_yield_default_value) {
  EXPECT_EQ((IndexOfType<void>), -1);
 }

 TEST(TypeAtIndex, valid_indices_yield_expected_type) {
  EXPECT_TRUE((SameType<int, TypeAtIndex<3, void, float, double, int>>));
  EXPECT_TRUE((SameType<char [5], TypeAtIndex<2, int, char[], char[5]>>));
 }

 TEST(TypeAtIndex, invalid_indices_yield_default_type) {
  EXPECT_TRUE((SameType<ImpossibleType<>, TypeAtIndex<0>>));
  EXPECT_TRUE((SameType<ImpossibleType<>, TypeAtIndex<-1, int, float>>));
  EXPECT_TRUE((SameType<ImpossibleType<>, TypeAtIndex<52, char[1], int, double *>>));
 }

 TEST(IndexOfTemplateType, type_pack_yields_expected_value) {
  EXPECT_EQ((IndexOfTemplateType<VoidT, ImpossibleType, VoidT, Dummy>), 1);
  EXPECT_EQ((IndexOfTemplateType<ImpossibleType, Dummy, VoidT, MetaFunctions::SameType, AsStlCompatible, ImpossibleType>), 4);
 }

 TEST(IndexOfTemplateType, special_cases_yield_default_value) {
  EXPECT_EQ((IndexOfTemplateType<VoidT>), -1);
 }

 template<template<typename...> typename T>
 struct Receiver {
  template<template<typename...> typename Other>
  //Note: can't use `auto const` w/ clang due to fatal frontend bug
  static constexpr bool const SameTemplateType = CX::SameTemplateType<T, Other>;
 };

 TEST(TemplateTypeAtIndex, valid_indices_yield_expected_type) {
  EXPECT_TRUE((TemplateTypeAtIndex<2, Receiver, VoidT, Dummy, MetaFunctions::SameType>
   ::SameTemplateType<MetaFunctions::SameType>
  ));
  EXPECT_TRUE((TemplateTypeAtIndex<
    3,
    Receiver,
    VoidT,
    Dummy,
    MetaFunctions::MatchAnyType,
    MetaFunctions::IndexOfType,
    MetaFunctions::SameType,
    MetaFunctions::UniqueTypes
   >::SameTemplateType<MetaFunctions::IndexOfType>
  ));
 }

 TEST(TemplateTypeAtIndex, invalid_indices_yield_default_type) {
  EXPECT_TRUE((TemplateTypeAtIndex<0, Receiver>::SameTemplateType<ImpossibleType>));
  EXPECT_TRUE((TemplateTypeAtIndex<124, Receiver, VoidT, Dummy>::SameTemplateType<ImpossibleType>));
 }

 TEST(IndexOfValue, value_pack_yields_expected_value) {
  EXPECT_EQ((IndexOfValue<1, 2, 3, 4, 5, 6, 1, 7, 8>), 5);
  EXPECT_EQ((IndexOfValue<314, 0, 314, 628, 1256>), 1);
 }

 TEST(IndexOfValue, special_cases_yield_default_value) {
  EXPECT_EQ((IndexOfValue<0>), -1);
 }

 TEST(SelectType, condition_yields_expected_type) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(SelectTemplateType, condition_yields_expected_template_type) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(SelectValue, condition_yields_expected_value) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(TypeIterator, empty_type_pack_does_not_iterate) {
  int i = 0;
  TypeIterator<>::run([&]<typename> { i++; });
  EXPECT_EQ(i, 0);
 }

 TEST(TypeIterator, bool_producer_callback_halts_iteration_when_returning_false) {
  int i = 0;
  TypeIterator<int, float, char, void, double>::run([&]<typename T> {
   return i++ < 2;
  });
  EXPECT_EQ(i, 3);
 }

 TEST(TypeIterator, bool_producer_callback_does_not_halt_iteration_when_returning_true) {
  int i = 0;
  TypeIterator<char, double, float, int, void>::run([&]<typename> {
   i++;
   return true;
  });
  EXPECT_EQ(i, 5);
 }

 TEST(TypeIterator, callback_invoked_for_all_types_in_pack) {
  auto const expected = ~(~0 << 6);
  auto invoked = 0;
  TypeIterator<char, void, double, int, float, short>::run([&]<typename T> {
   invoked |= (1 << IndexOfType<T, char, void, double, int, float, short>);
  });
  EXPECT_EQ(invoked ^ expected, 0);
 }

 TEST(TemplateTypeIterator, empty_type_pack_does_not_iterate) {
  int i = 0;
  TemplateTypeIterator<>::run([&]<template<typename...> typename> { i++; });
  EXPECT_EQ(i, 0);
 }

 TEST(TemplateTypeIterator, bool_producer_callback_halts_iteration_when_returning_false) {
  int i = 0;
  TemplateTypeIterator<MetaFunctions::SameType, Dummy, ImpossibleType>::run([&]<template<typename...> typename> {
   return i++ < 1;
  });
  EXPECT_EQ(i, 2);
 }

 TEST(TemplateTypeIterator, bool_producer_callback_does_not_halt_iteration_when_returning_true) {
  int i = 0;
  TemplateTypeIterator<MetaFunctions::MatchAnyType, Dummy, ImpossibleType, Dummy>::run([&]<template<typename...> typename> {
   i++;
   return true;
  });
  EXPECT_EQ(i, 4);
 }

 TEST(TemplateTypeIterator, callback_invoked_for_all_types_in_pack) {
  auto const expected = ~(~0 << 4);
  auto invoked = 0;
  TemplateTypeIterator<MetaFunctions::Unqualified, MetaFunctions::SameType, Dummy, ImpossibleType>::run([&]<template<typename...> typename T> {
   invoked |= (1 << IndexOfTemplateType<T, MetaFunctions::Unqualified, MetaFunctions::SameType, Dummy, ImpossibleType>);
  });
  EXPECT_EQ(invoked ^ expected, 0);
 }

 TEST(ValueIterator, empty_value_pack_does_not_iterate) {
  int i = 0;
  ValueIterator<>::run([&]<auto> { i++; });
  EXPECT_EQ(i, 0);
 }

 TEST(ValueIterator, bool_producer_callback_halts_iteration_when_returning_false) {
  int i = 0;
  ValueIterator<1, 2, 3, 4, 5, 6>::run([&]<auto> {
   return i++ < 3;
  });
  EXPECT_EQ(i, 4);
 }

 TEST(ValueIterator, bool_producer_callback_does_not_halt_iteration_when_returning_true) {
  int i = 0;
  ValueIterator<5, 4, 3, 2, 1, 0, -1>::run([&]<auto> {
   i++;
   return true;
  });
  EXPECT_EQ(i, 7);
 }

 TEST(ValueIterator, callback_invoked_for_all_types_in_pack) {
  auto const expected = ~(~0 << 12);
  auto invoked = 0;
  ValueIterator<12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1>::run([&]<auto V> {
   invoked |= (1 << IndexOfValue<V, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1>);
  });
  EXPECT_EQ(invoked ^ expected, 0);
 }
}
