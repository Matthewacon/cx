#include <cx/test/common/common.h>

#include <cx/variant.h>

namespace CX {
 //TODO move to idioms and create template-parameter and value-parameter
 //equivalents
 template<template<typename...> typename S, typename... Types>
 concept Specializable = requires (S<Types...> s) { (void)s; };

 TEST(Variant, variant_type_disallows_void_and_duplicate_elements) {
  EXPECT_FALSE((Specializable<Variant, void>));
  EXPECT_FALSE((Specializable<Variant, int, float, char, float>));
  struct A {};
  EXPECT_FALSE((Specializable<Variant, A, A>));
 }

 //TODO test all constructors

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
  //TODO element and variant assignment operators 
  //EXPECT_FALSE(());
 }

 TEST(Variant, empty_variant_has_returns_false_for_all_element_types) {
  Variant<char[124], int, double> empty;
  EXPECT_FALSE(empty.has<char[124]>());
  EXPECT_FALSE(empty.has<int>());
  EXPECT_FALSE(empty.has<double>());
 }

 TEST(Variant, empty_variant_get_throws_exception_for_all_element_types) {
  throw "Unimplemented"; 
 }

 TEST(Variant, empty_variant_drain_throws_exception_for_all_element_types) {
  throw "Unimplemented";
 }

 TEST(Variant, empty_variant_rdrain_throws_exception_for_all_element_types) {
  throw "Unimplemented";
 }

 TEST(Variant, a) {
  constexpr float const expectedValue = 1234.5; 
  using expectedType = float;
  Variant<int, float, char> v1{expectedValue};
  EXPECT_TRUE(v1.has<expectedType>());
  EXPECT_EQ(v1.get<expectedType>(), expectedValue);
 }
}
