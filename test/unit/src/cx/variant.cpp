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

 TEST(CompatibleVariant, superset_and_matched_set_with_mixed_const_variant_types_satisfy_constraint) {
  //Matched type set
  EXPECT_TRUE((CompatibleVariant<Variant<int const>, Variant<int>>));
  EXPECT_TRUE((CompatibleVariant<Variant<float, char const, int>, Variant<float const, char, int const>>));

  //Superset
  EXPECT_TRUE((CompatibleVariant<Variant<int, float const>, Variant<int const, float, char const, double>>));
  EXPECT_TRUE((CompatibleVariant<Variant<char const, int, std::tuple<> const>, Variant<char, int const, std::tuple<>, void * const>>));
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
  //Test with integral type
  using ExpectedTypeA = char;
  ExpectedTypeA const expectedValueA = 123;
  Variant<int, ExpectedTypeA> v{expectedValueA};
  EXPECT_TRUE((v.has<ExpectedTypeA>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v.get<ExpectedTypeA>()), expectedValueA);
  }()));

  //Test with copyable type
  //Ensure element copy constructor is properly invoked
  static bool copyConstructorInvoked;
  //Reset static flag if test is re-run
  copyConstructorInvoked = false;
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
  //Test with integral type
  using ExpectedTypeA = short;
  ExpectedTypeA const expectedValueA = 534;
  Variant<float, ExpectedTypeA> v1{expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>()), expectedValueA);
  }()));

  //Test with movable type
  //Ensure element move constructor is properly invoked
  static bool moveConstructorInvoked;
  //Reset static flag if test is re-run
  moveConstructorInvoked = false;
  using ExpectedTypeB = struct A {
   int i;

   A(int i) :
    i(i)
   {}

   A(A &&a) :
    i(a.i)
   {
    a.i = 0;
    moveConstructorInvoked = true;
   }
  };
  int const expectedValueB = 473829;
  ExpectedTypeB temp{expectedValueB};
  Variant<char, ExpectedTypeB> v2{(ExpectedTypeB&&)temp};
  EXPECT_TRUE((moveConstructorInvoked));
  EXPECT_EQ(temp.i, 0);
  EXPECT_TRUE((v2.has<ExpectedTypeB>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().i), expectedValueB);
  }()));
 }

 TEST(Constructor, variant_copy_constructor_properly_initializes_variant) {
  using ExpectedType = int;
  constexpr ExpectedType const expectedValue = 314151617;

  //Initialize first variant and ensure it was initialized correctly
  Variant<char, int> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Copy first variant into second variant
  Variant<float, int, char> v2{(decltype(v1)&)v1};

  //Ensure all properties of the second variant were correctly initialized
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 TEST(Constructor, variant_move_constructor_properly_initializes_variant_and_destructs_moved_variant) {
  using ExpectedType = short;
  constexpr ExpectedType const expectedValue = 1345;

  //Initialize first variant and ensure it was initialized correctly
  Variant<double, char, short> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Move first variant into second variant
  Variant<short, double, char, int> v2{(decltype(v1)&&)v1};

  //Ensure first variant was properly destructed
  EXPECT_FALSE((v1.has<ExpectedType>()));
  EXPECT_THROW(
   ([&] { v1.get<ExpectedType>(); }()),
   VariantTypeError
  );

  //Ensure all properties of second variant were correctly initialized
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 TEST(Constructor, variant_const_member_supports_l_and_r_value_reference_construction) {
  using ExpectedTypeA = int const;
  constexpr int expectedValueA = 153125;
  (void)(ExpectedTypeA)expectedValueA;
  //Test with intgral types
  //TODO deal with const element conversions within variant
  /*
  Variant<ExpectedTypeA> v1{(decltype(expectedValueA)&)expectedValueA};
  Variant<ExpectedTypeA> v2{(decltype(expectedValueA)&&)expectedValueA};
  */

  //Test with copyable and movable type
  struct A {
   int data[124];
  };
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
