#include <cx/test/common/common.h>

#include <cx/variant.h>

namespace CX {
 //Variant meta-function and concept tests
 TEST(IsVariant, variant_satisfies_constraint) {
  EXPECT_TRUE((IsVariant<Variant<>>));
  EXPECT_TRUE((IsVariant<Variant<int, char, float>>));
 }

 TEST(IsVariant, non_variant_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((IsVariant<char>));
  EXPECT_FALSE((IsVariant<Dummy<>>));
 }

 TEST(CompatibleVariant, superset_and_matched_set_variant_types_satisfy_constraint) {
  //Matched type set
  EXPECT_TRUE((CompatibleVariant<Variant<>, Variant<>>));
  EXPECT_TRUE((CompatibleVariant<Variant<char>, Variant<char>>));

  //Superset
  EXPECT_TRUE((CompatibleVariant<Variant<int, float>, Variant<int, float, char>>));
  EXPECT_TRUE((CompatibleVariant<Variant<double, char, float>, Variant<float, double, char, int [1234]>>));
 }

 TEST(CompatibleVariant, subset_variant_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((CompatibleVariant<Variant<float [123]>, Variant<>>));
  EXPECT_FALSE((CompatibleVariant<Variant<double, char>, Variant<int, double>>));
  EXPECT_FALSE((CompatibleVariant<Variant<int, float, short>, Variant<int, short>>));
 }

 TEST(CompatibleVariant, non_variant_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((CompatibleVariant<Dummy<>, ImpossibleType<>>));
  EXPECT_FALSE((CompatibleVariant<Variant<>, Dummy<>>));
  EXPECT_FALSE((CompatibleVariant<MetaFunctions::MatchAnyType<>, Variant<int>>));
 }

 //Variant tests
 template<template<typename...> typename S, typename... Types>
 concept Specializable = requires (S<Types...> s) { (void)s; };

 TEST(Variant, variant_type_disallows_void_and_duplicate_elements) {
  EXPECT_FALSE((Specializable<Variant, void>));
  EXPECT_FALSE((Specializable<Variant, int, float, char, float>));
  struct A {};
  EXPECT_FALSE((Specializable<Variant, A, A>));
 }

 //Constructor tests
 TEST(Constructor, element_copy_constructor_properly_initializes_variant) {
  using ExpectedTypeA = char;
  ExpectedTypeA const expectedValueA = 123;
  Variant<int, ExpectedTypeA> v{expectedValueA};
  EXPECT_TRUE((v.has<ExpectedTypeA>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v.get<ExpectedTypeA>()), expectedValueA);
  }()));

  //Ensure element copy constructor is properly invoked
  static bool copyConstructorInvoked = false;
  using ExpectedTypeB = struct A {
   int i;

   A(int i) :
    i(i)
   {}

   A(A const &a) :
    i(a.i)
   {
    copyConstructorInvoked = true;
   }
  };
  A const expectedValueB{54321098};
  Variant<double[1234], ExpectedTypeB> v2{expectedValueB};
  EXPECT_TRUE(copyConstructorInvoked);
  EXPECT_TRUE((v2.has<ExpectedTypeB>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().i), expectedValueB.i);
  }()));
 }

 TEST(Constructor, element_move_constructor_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, variant_copy_constructor_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, variant_move_constructor_properly_initializes_variant_and_destructs_moved_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, variant_const_member_supports_l_and_r_value_reference_construction) {
  throw std::runtime_error{"Unimplemented"};
 }

 //Assignment operator tests
 TEST(Assignment, element_copy_assignment_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, element_move_assignment_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, variant_copy_assignment_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, variant_move_assignment_properly_initializes_variant_and_destructs_moved_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, variant_const_member_supports_l_and_r_value_reference_assignment) {
  throw std::runtime_error{"Unimplemented"};
 }

 //Member function tests
 template<typename V, typename E>
 concept VariantHasInvokable = requires (V v) { v.template has<E>(); };

 template<typename V, typename E>
 concept VariantDrainInvokable = requires (V v) { v.template drain<E>(); };

 template<typename V, typename E>
 concept VariantRdrainInvokable = requires (V v, E e) { v.template rdrain<E>(e); };

 TEST(Variant, member_templates_are_invalid_for_non_element_types) {
  using VariantType = Variant<float, char>;
  EXPECT_FALSE((VariantHasInvokable<VariantType, void>));
  EXPECT_FALSE((VariantDrainInvokable<VariantType, void>));
  EXPECT_FALSE((VariantRdrainInvokable<VariantType, void>));
  throw std::runtime_error{"Unfinished"};
  //TODO element and variant assignment operators
  //EXPECT_FALSE(());
 }

 //Tests for the empty variant specialization
 TEST(Variant, empty_variant_has_returns_false_for_all_element_types) {
  Variant<char[124], int, double> empty;
  EXPECT_FALSE(empty.has<char[124]>());
  EXPECT_FALSE(empty.has<int>());
  EXPECT_FALSE(empty.has<double>());
 }

 TEST(Variant, empty_variant_get_throws_exception_for_all_element_types) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Variant, empty_variant_drain_throws_exception_for_all_element_types) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Variant, empty_variant_rdrain_throws_exception_for_all_element_types) {
  throw std::runtime_error{"Unimplemented"};
 }
}
