#include <cx/test/common/common.h>

#include <functional>

#include <cx/idioms.h>

namespace CX::Testing {
 TEST(SameType, identical_types_satisfy_constraint) {
  EXPECT_TRUE((SameType<int, int>));
 }

 TEST(SameType, different_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((SameType<double, void>));
 }

 template<typename Incomplete>
 concept IncompleteSafeSameType = requires { SameType<Incomplete, Incomplete>; };

 TEST(SameType, concept_is_incomplete_safe) {
  EXPECT_TRUE((IncompleteSafeSameType<ImpossibleType<>>));
 }

 TEST(MatchAnyType, type_pack_containing_first_type_parameter_satisfies_constraint) {
  EXPECT_TRUE((MatchAnyType<int, float, void, char, double, int, short>));
 }

 TEST(MatchAnyType, type_pack_not_containing_first_type_parameter_does_not_satisfy_constraint) {
  EXPECT_FALSE((MatchAnyType<>));
  EXPECT_FALSE((MatchAnyType<void>));
  EXPECT_FALSE((MatchAnyType<void, char, float, int, double, char, float, int, double>));
 }

 TEST(SameTemplateType, identical_types_satisfy_constraint) {
  EXPECT_TRUE((SameTemplateType<Dummy, Dummy>));
 }

 TEST(SameTemplateType, different_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((SameTemplateType<Dummy, VoidT>));
 }

 template<template<typename...> typename T>
 concept IncompleteSafeSameTemplateType = requires { SameTemplateType<T, T>; };

 TEST(SameTemplateType, concept_is_incomplete_safe) {
  EXPECT_TRUE((IncompleteSafeSameTemplateType<ImpossibleType>));
 }

 TEST(MatchAnyTemplateType, type_pack_containing_first_type_parameter_satisfies_constraint) {
  EXPECT_TRUE((MatchAnyTemplateType<Dummy, VoidT, MetaFunctions::SameType, Dummy>));
 }

 TEST(MatchAnyTemplateType, type_pack_not_containing_first_type_parameter_does_not_satisfy_constraint) {
  EXPECT_FALSE((MatchAnyTemplateType<>));
  EXPECT_FALSE((MatchAnyTemplateType<VoidT>));
  EXPECT_FALSE((MatchAnyTemplateType<Dummy, ImpossibleType, VoidT, ImpossibleType>));
 }

 TEST(SameValue, identical_values_satisfy_constraint) {
  EXPECT_TRUE((SameValue<314, 314>));
 }

 TEST(SameValue, different_values_do_not_satisfy_constraint) {
  EXPECT_FALSE((SameValue<314, (char)0>));
 }

 TEST(MatchAnyValue, value_pack_containing_first_value_satisfies_constraint) {
  EXPECT_TRUE((MatchAnyValue<0, 1, 2, 3, 4, 0>));
 }

 TEST(MatchAnyValue, value_pack_not_containing_first_value_does_not_satisfy_constraint) {
  EXPECT_FALSE((MatchAnyValue<>));
  EXPECT_FALSE((MatchAnyValue<0>));
  EXPECT_FALSE((MatchAnyValue<0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7>));
 }

 TEST(UniqueTypes, unique_type_pack_satisfies_constraint) {
  EXPECT_TRUE((UniqueTypes<>));
  EXPECT_TRUE((UniqueTypes<void>));
  EXPECT_TRUE((UniqueTypes<int, float, void, double, char>));
 }

 TEST(UniqueTypes, type_pack_with_duplicates_does_not_satisfy_constraint) {
  EXPECT_FALSE((UniqueTypes<void, char, short, double, char>));
 }

 TEST(UniqueTemplateTypes, type_pack_without_duplicates_satisfies_constraint) {
  EXPECT_TRUE((UniqueTemplateTypes<>));
  EXPECT_TRUE((UniqueTemplateTypes<VoidT>));
  EXPECT_TRUE((UniqueTemplateTypes<VoidT, ImpossibleType, MetaFunctions::MatchAnyType>));
 }

 TEST(UniqueTemplateTypes, type_pack_with_duplicates_does_not_satisfy_constraint) {
  EXPECT_FALSE((UniqueTemplateTypes<VoidT, ImpossibleType, VoidT, MetaFunctions::SameType>));
 }

 TEST(UniqueValues, unique_value_pack_satisfies_constraint) {
  EXPECT_TRUE((UniqueValues<>));
  EXPECT_TRUE((UniqueValues<0>));
  EXPECT_TRUE((UniqueValues<0, 1, 2, 3, 4, 5, 6, 7, 8, 9>));
 }

 TEST(UniqueValues, value_pack_with_duplicates_does_not_satisfy_constraint) {
  EXPECT_FALSE((UniqueValues<0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 9>));
 }

 TEST(ConvertibleTo, integral_types_satisfy_constraint) {
  EXPECT_TRUE((ConvertibleTo<char, int>));
  EXPECT_TRUE((ConvertibleTo<int, float>));
  EXPECT_TRUE((ConvertibleTo<float, double>));
 }

 TEST(ConvertibleTo, implicitly_convertible_struct_satisfies_constraint) {
  struct S {
   operator int() {
    return 0;
   }
  };
  EXPECT_TRUE((ConvertibleTo<S, int>));
 }

 TEST(ConvertibleTo, non_convertible_types_do_not_satisfy_constraint) {
  struct A {};
  struct B {};
  EXPECT_FALSE((ConvertibleTo<A, B>));
 }

 TEST(Constructible, contstructible_types_satisfy_constraint) {
  EXPECT_TRUE((Constructible<int>));
  struct A {
   A(int) {}
  };
  EXPECT_TRUE((Constructible<A, int>));
  EXPECT_TRUE((Constructible<char[1234]>));
  struct B {
   B(int, float, double, char *) noexcept {}
  };
  EXPECT_TRUE((Constructible<B, int, float, double, char *>));
 }

 TEST(Constructible, non_constructible_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Constructible<void>));
  EXPECT_FALSE((Constructible<float[]>));
  struct A {
   A() = delete;
  };
  EXPECT_FALSE((Constructible<A>));
 }

 TEST(Destructible, destructible_types_satisfy_constraint) {
  EXPECT_TRUE((Destructible<int>));
  struct A {};
  EXPECT_TRUE((Destructible<A>));
  EXPECT_TRUE((Destructible<void *>));
 }

 TEST(Destructible, non_destructible_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Destructible<void>));
  EXPECT_FALSE((Destructible<int&>));
  EXPECT_FALSE((Destructible<ImpossibleType<>>));
 }

 TEST(CopyConstructible, copy_constructible_types_satisfy_constraint) {
  EXPECT_TRUE((CopyConstructible<int>));
  EXPECT_TRUE((CopyConstructible<void *>));
  struct A {};
  EXPECT_TRUE((CopyConstructible<A>));
 }

 TEST(CopyConstructible, non_copy_constructible_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((CopyConstructible<void>));
  EXPECT_FALSE((CopyConstructible<int[]>));
  struct A {
   A(A const&) = delete;
  };
  EXPECT_FALSE((CopyConstructible<A>));
 }

 TEST(MoveConstructible, move_constructible_types_satisfy_constraint) {
  EXPECT_TRUE((MoveConstructible<int>));
  EXPECT_TRUE((MoveConstructible<void *>));
  struct A {};
  EXPECT_TRUE((MoveConstructible<A>));
 }

 TEST(MoveConstructible, non_move_constructible_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((MoveConstructible<void>));
  EXPECT_FALSE((MoveConstructible<int[]>));
  EXPECT_FALSE((MoveConstructible<double[1234]>));
  struct A {
   A(A&&) = delete;
  };
  EXPECT_FALSE((MoveConstructible<A>));
 }

 TEST(CopyAssignable, copy_assignable_types_satisfy_constraint) {
  EXPECT_TRUE((CopyAssignable<int>));
  EXPECT_TRUE((CopyAssignable<char *>));
  struct A {};
  EXPECT_TRUE((CopyAssignable<A>));
 }

 TEST(CopyAssignable, non_copy_assignable_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((CopyAssignable<void>));
  EXPECT_FALSE((CopyAssignable<short[]>));
  EXPECT_FALSE((CopyAssignable<long[323]>));
  struct A {
   A& operator=(A const&) = delete;
  };
  EXPECT_FALSE((CopyAssignable<A>));
 }

 TEST(MoveAssignable, move_assignable_types_satisfy_constraint) {
  EXPECT_TRUE((MoveAssignable<char>));
  EXPECT_TRUE((MoveAssignable<double *>));
  struct A {};
  EXPECT_TRUE((MoveAssignable<A>));
 }

 TEST(MoveAssignable, non_move_assignable_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((MoveAssignable<void>));
  EXPECT_FALSE((MoveAssignable<long long[]>));
  struct A {
   A& operator=(A&&) = delete;
  };
  EXPECT_FALSE((MoveAssignable<A>));
 }

 TEST(Unqualified, qualified_types_lose_all_qualifiers) {
  using TypeA = char const[1234];
  using ExpectedTypeA = char;
  EXPECT_TRUE((SameType<Unqualified<TypeA>, ExpectedTypeA>));

  using TypeB = float const&;
  using ExpectedTypeB = float;
  EXPECT_TRUE((SameType<Unqualified<TypeB>, ExpectedTypeB>));

  using TypeC = int const * const;
  using ExpectedTypeC = int;
  EXPECT_TRUE((SameType<Unqualified<TypeC>, ExpectedTypeC>));

  using TypeD = char const[];
  using ExpectedTypeD = char;
  EXPECT_TRUE((SameType<Unqualified<TypeD>, ExpectedTypeD>));

  using TypeE = float Dummy<>::* const;
  using ExpectedTypeE = float Dummy<>::*;
  EXPECT_TRUE((SameType<Unqualified<TypeE>, ExpectedTypeE>));
 }

 TEST(Unqualified, unqualified_types_are_unmodified) {
  using TypeA = char;
  using ExpectedTypeA = TypeA;
  EXPECT_TRUE((SameType<Unqualified<TypeA>, ExpectedTypeA>));

  using TypeB = void (int, float) noexcept;
  using ExpectedTypeB = TypeB;
  EXPECT_TRUE((SameType<Unqualified<TypeB>, ExpectedTypeB>));

  using TypeC = float (Dummy<>::*)(Dummy<> const&) const noexcept;
  using ExpectedTypeC = TypeC;
  EXPECT_TRUE((SameType<Unqualified<TypeC>, ExpectedTypeC>));
 }

 TEST(Decayed, qualified_types_lose_lowest_depth_qualifier) {
  using TypeA = char[1234];
  using ExpectedTypeA = char[];
  EXPECT_TRUE((SameType<Decayed<TypeA>, ExpectedTypeA>));

  using TypeB = int const * const;
  using ExpectedTypeB = int const *;
  EXPECT_TRUE((SameType<Decayed<TypeB>, ExpectedTypeB>));

  using TypeC = double&&;
  using ExpectedTypeC = double&;
  EXPECT_TRUE((SameType<Decayed<TypeC>, ExpectedTypeC>));

  using TypeD = char const[];
  using ExpectedTypeD = char const;
  EXPECT_TRUE((SameType<Decayed<TypeD>, ExpectedTypeD>));
 }

 TEST(Decayed, unqualified_types_are_unmodified) {
  using TypeA = float (Dummy<>::*)() const;
  using ExpectedTypeA = TypeA;
  EXPECT_TRUE((SameType<Decayed<TypeA>, ExpectedTypeA>));

  using TypeB = decltype(nullptr);
  using ExpectedTypeB = TypeB;
  EXPECT_TRUE((SameType<Decayed<TypeB>, ExpectedTypeB>));

  using TypeC = signed char;
  using ExpectedTypeC = TypeC;
  EXPECT_TRUE((SameType<Decayed<TypeC>, ExpectedTypeC>));
 }

 TEST(Const, const_types_satisfy_constraints) {
  EXPECT_TRUE((Const<int const>));
  EXPECT_TRUE((Const<Dummy<> const>));
  EXPECT_TRUE((Const<char * const>));
 }

 TEST(Const, const_reference_types_satisfy_constraint) {
  EXPECT_TRUE((Const<short const&>));
  EXPECT_TRUE((Const<double const&&>));
 }

 TEST(Const, non_const_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Const<void>));
  EXPECT_FALSE((Const<char>));
  EXPECT_FALSE((Const<int&>));
  EXPECT_FALSE((Const<float&&>));
 }

 TEST(ConstDecayed, const_types_lose_const_qualifier) {
  EXPECT_TRUE((SameType<ConstDecayed<float const>, float>));
  EXPECT_TRUE((SameType<ConstDecayed<char const (Dummy<>::* const)>, char const (Dummy<>::*)>));
 }

 TEST(ConstDecayed, non_const_types_are_unmodified) {
  EXPECT_TRUE((SameType<ConstDecayed<int *>, int *>));
  EXPECT_TRUE((SameType<ConstDecayed<char volatile>, char volatile>));
 }

 TEST(Volatile, volatile_types_satisfy_constraint) {
  EXPECT_TRUE((Volatile<int volatile>));
  EXPECT_TRUE((Volatile<bool * volatile>));
  EXPECT_TRUE((Volatile<void (Dummy<>::* volatile)()>));
 }

 TEST(Volatile, non_volatile_tyeps_do_not_satisfy_constraint) {
  EXPECT_FALSE((Volatile<void>));
  EXPECT_FALSE((Volatile<char8_t>));
  EXPECT_FALSE((Volatile<int>));
 }

 TEST(VolatileDecayed, volatile_types_lose_volatile_qualifier) {
  EXPECT_TRUE((SameType<VolatileDecayed<char volatile>, char>));
  EXPECT_TRUE((SameType<VolatileDecayed<void * volatile>, void *>));
  EXPECT_TRUE((SameType<VolatileDecayed<int (Dummy<>::* volatile)>, int (Dummy<>::*)>));
 }

 TEST(VolatileDecayed, non_volatile_types_are_unmodified) {
  EXPECT_TRUE((SameType<VolatileDecayed<float>, float>));
  EXPECT_TRUE((SameType<VolatileDecayed<ImpossibleType<>[]>, ImpossibleType<>[]>));
  EXPECT_TRUE((SameType<VolatileDecayed<int&>, int&>));
 }

 TEST(Array, unsized_array_types_satisfy_constraint) {
  EXPECT_TRUE((Array<char[]>));
  EXPECT_TRUE((Array<float const[]>));
  EXPECT_TRUE((Array<void (Dummy<>::*[])()>));
 }

 TEST(Array, sized_array_types_satisfy_constraint) {
  EXPECT_TRUE((Array<double[1234]>));
  EXPECT_TRUE((Array<char const[5513]>));
  EXPECT_TRUE((Array<void (Dummy<>::*[500])()>));
 }

 TEST(Array, non_array_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Array<void>));
  EXPECT_FALSE((Array<char *>));
  EXPECT_FALSE((Array<void (Dummy<>::*)()>));
 }

 TEST(SizedArray, sized_array_types_satisfy_constraint) {
  EXPECT_TRUE((SizedArray<int[123]>));
  //Note: This due to bugs in clang and gcc, this line will
  //always fail. See related comment in 'cx/idioms.h'
  //EXPECT_TRUE((SizedArray<char[0]>));
 }

 TEST(SizedArray, unsized_array_and_non_array_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((SizedArray<double[]>));
  EXPECT_FALSE((SizedArray<void>));
 }

 TEST(UnsizedArray, unsized_array_types_satisfy_constraint) {
  EXPECT_TRUE((UnsizedArray<bool[]>));
  EXPECT_TRUE((UnsizedArray<void *[]>));
 }

 TEST(UnsizedArray, sized_array_non_array_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((UnsizedArray<float[123]>));
  EXPECT_FALSE((UnsizedArray<void (Dummy<>::*[5])()>));
 }

 TEST(ArrayDecayed, array_types_yield_array_element_type) {
  using TypeA = char[123];
  using ExpectedTypeA = char;
  EXPECT_TRUE((SameType<ArrayDecayed<TypeA>, ExpectedTypeA>));

  using TypeB = float[];
  using ExpectedTypeB = float;
  EXPECT_TRUE((SameType<ArrayDecayed<TypeB>, ExpectedTypeB>));

  using TypeC = void * const[34];
  using ExpectedTypeC = void * const;
  EXPECT_TRUE((SameType<ArrayDecayed<TypeC>, ExpectedTypeC>));

  using TypeD = float (Dummy<>::* const[5124])() const;
  using ExpectedTypeD = float (Dummy<>::* const)() const;
  EXPECT_TRUE((SameType<ArrayDecayed<TypeD>, ExpectedTypeD>));
 }

 TEST(ArrayDecayed, non_array_types_yield_the_same_type) {
  using TypeA = char (Dummy<>::*)() noexcept;
  using ExpectedTypeA = TypeA;
  EXPECT_TRUE((SameType<ArrayDecayed<TypeA>, ExpectedTypeA>));

  using TypeB = float const * const;
  using ExpectedTypeB = TypeB;
  EXPECT_TRUE((SameType<ArrayDecayed<TypeB>, ExpectedTypeB>));

  using TypeC = void;
  using ExpectedTypeC = TypeC;
  EXPECT_TRUE((SameType<ArrayDecayed<TypeC>, ExpectedTypeC>));
 }

 TEST(ArraySize, sized_array_types_yield_the_correct_size) {
  EXPECT_EQ((ArraySize<char const[1234]>), 1234);
  EXPECT_EQ((ArraySize<double (Dummy<>::*[1846729])()>), 1846729);
  EXPECT_EQ((ArraySize<long long const[1]>), 1);
 }

 TEST(ArraySize, unsized_array_types_yield_negaitve_one) {
  EXPECT_EQ((ArraySize<char[]>), -1);
  EXPECT_EQ((ArraySize<float (*[])() noexcept>), -1);
  EXPECT_EQ((ArraySize<int const[]>), -1);
 }

 TEST(Pointer, pointer_types_satisfy_constraint) {
  EXPECT_TRUE((Pointer<int *>));
  EXPECT_TRUE((Pointer<void ***>));
 }

 TEST(Pointer, non_pointer_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Pointer<int[]>));
  EXPECT_FALSE((Pointer<bool&>));
  EXPECT_FALSE((Pointer<void (Dummy<>::*)()>));
  EXPECT_FALSE((Pointer<char (Dummy<>::*)>));
 }

 TEST(MemberPointer, member_pointer_types_satisfy_constraint) {
  EXPECT_TRUE((MemberPointer<char (Dummy<>::*)>));
  EXPECT_TRUE((MemberPointer<void (ImpossibleType<>::*)()>));
 }

 TEST(MemberPointer, non_member_pointer_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((MemberPointer<void *>));
  EXPECT_FALSE((MemberPointer<int>));
  EXPECT_FALSE((MemberPointer<float (*)()>));
 }

 TEST(LValueReference, lvalue_reference_types_satisfy_constraint) {
  EXPECT_TRUE((LValueReference<int&>));
  EXPECT_TRUE((LValueReference<double const&>));
  EXPECT_TRUE((LValueReference<void (&)()>));
 }

 TEST(LValueReference, non_lvalue_reference_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((LValueReference<void>));
  EXPECT_FALSE((LValueReference<int[]>));
  EXPECT_FALSE((LValueReference<char *>));
 }

 TEST(LValueReferenceDecayed, lvalue_reference_types_yield_the_reference_element_type) {
  using TypeA = int&;
  using ExpectedTypeA = int;
  EXPECT_TRUE((SameType<LValueReferenceDecayed<TypeA>, ExpectedTypeA>));

  using TypeB = char * const&;
  using ExpectedTypeB = char * const;
  EXPECT_TRUE((SameType<LValueReferenceDecayed<TypeB>, ExpectedTypeB>));

  using TypeC = void (&)();
  using ExpectedTypeC = void ();
  EXPECT_TRUE((SameType<LValueReferenceDecayed<TypeC>, ExpectedTypeC>));
 }

 TEST(LValueReferenceDecayed, non_lvalue_reference_types_yield_the_same_type) {
  using TypeA = char *;
  using ExpectedTypeA = TypeA;
  EXPECT_TRUE((SameType<LValueReferenceDecayed<TypeA>, ExpectedTypeA>));

  using TypeB = float ();
  using ExpectedTypeB = TypeB;
  EXPECT_TRUE((SameType<LValueReferenceDecayed<TypeB>, ExpectedTypeB>));

  using TypeC = void;
  using ExpectedTypeC = TypeC;
  EXPECT_TRUE((SameType<LValueReferenceDecayed<TypeC>, ExpectedTypeC>));
 }

 TEST(RValueReference, rvalue_reference_types_satisfy_constraint) {
  EXPECT_TRUE((RValueReference<int&&>));
  EXPECT_TRUE((RValueReference<float(&&)[]>));
  EXPECT_TRUE((RValueReference<short const&&>));
 }

 TEST(RValueReference, non_rvalue_reference_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((RValueReference<void>));
  EXPECT_FALSE((RValueReference<char&>));
  EXPECT_FALSE((RValueReference<Dummy<>>));
 }

 TEST(RValueReferenceDecayed, rvalue_reference_types_yield_the_reference_element_type) {
  using TypeA = Dummy<>&&;
  using ExpectedTypeA = Dummy<>;
  EXPECT_TRUE((SameType<RValueReferenceDecayed<TypeA>, ExpectedTypeA>));

  using TypeB = char (Dummy<>::*&&)();
  using ExpectedTypeB = char (Dummy<>::*)();
  EXPECT_TRUE((SameType<RValueReferenceDecayed<TypeB>, ExpectedTypeB>));

  using TypeC = void *&&;
  using ExpectedTypeC = void *;
  EXPECT_TRUE((SameType<RValueReferenceDecayed<TypeC>, ExpectedTypeC>));
 }

 TEST(RValueReferenceDecayed, non_rvalue_reference_types_yield_the_same_type) {
  using TypeA = float *;
  using ExpectedTypeA = TypeA;
  EXPECT_TRUE((SameType<RValueReferenceDecayed<TypeA>, ExpectedTypeA>));

  using TypeB = void (Dummy<>::*[])() const;
  using ExpectedTypeB = TypeB;
  EXPECT_TRUE((SameType<RValueReferenceDecayed<TypeB>, ExpectedTypeB>));

  using TypeC = void *;
  using ExpectedTypeC = TypeC;
  EXPECT_TRUE((SameType<RValueReferenceDecayed<TypeC>, ExpectedTypeC>));
 }

 TEST(ReferenceDecayed, rvalue_reference_types_yield_the_reference_element_type) {
  EXPECT_TRUE((SameType<ReferenceDecayed<int&&>, int>));
  EXPECT_TRUE((SameType<ReferenceDecayed<void (* const&&)(...)>, void (* const)(...)>));
 }

 TEST(ReferenceDecayed, lvalue_reference_types_yield_the_reference_element_type) {
  EXPECT_TRUE((SameType<ReferenceDecayed<void *&>, void *>));
  EXPECT_TRUE((SameType<ReferenceDecayed<float&>, float>));
 }

 TEST(ReferenceDecayed, non_reference_types_yield_the_same_type) {
  EXPECT_TRUE((SameType<ReferenceDecayed<double ****>, double ****>));
  EXPECT_TRUE((SameType<ReferenceDecayed<void (Dummy<>::**)(...)>, void (Dummy<>::**)(...)>));
 }

 TEST(SignDecayed, signed_types_lose_signed_qualifier) {
  EXPECT_TRUE((SameType<SignDecayed<signed char>, unsigned char>));
  EXPECT_TRUE((SameType<SignDecayed<signed int>, unsigned int>));
 }

 TEST(SignDecayed, unsigned_types_are_unmodified) {
  EXPECT_TRUE((SameType<SignDecayed<float>, float>));
  EXPECT_TRUE((SameType<SignDecayed<void *>, void *>));
 }

 TEST(SignDecayed, non_sign_types_are_unmodified) {
  EXPECT_TRUE((SameType<SignDecayed<void (signed char)>, void (signed char)>));
  EXPECT_TRUE((SameType<SignDecayed<unsigned int[]>, unsigned int[]>));
 }

 TEST(SignPromoted, unsingned_types_are_sign_qualified) {
  EXPECT_TRUE((SameType<SignPromoted<unsigned int>, signed int>));
  EXPECT_EQ(sizeof(SignPromoted<char8_t>), sizeof(char8_t));
  EXPECT_TRUE((SameType<SignPromoted<unsigned long long>, signed long long>));
 }

 TEST(SignPromoted, signed_types_are_unmodified) {
  EXPECT_TRUE((SameType<SignPromoted<signed int>, signed int>));
  EXPECT_TRUE((SameType<SignPromoted<signed short>, signed short>));
 }

 TEST(SignPromoted, non_signed_types_are_unmodified) {
  struct A {};
  EXPECT_TRUE((SameType<SignPromoted<A>, A>));
  EXPECT_TRUE((SameType<SignPromoted<float>, float>));
 }

 TEST(Enum, enum_types_satisfy_constraint) {
  enum struct E1 {};
  EXPECT_TRUE((Enum<E1>));
  enum E2 { A };
  EXPECT_TRUE((Enum<decltype(A)>));
 }

 TEST(Enum, non_enum_types_do_not_satisfy_constraint) {
  struct A {};
  EXPECT_FALSE((Enum<A>));
  enum struct E {};
  EXPECT_FALSE((Enum<E ()>));
 }

 TEST(Union, union_types_satisfy_constraint) {
  union U {};
  EXPECT_TRUE((Union<U>));
  union {} u;
  EXPECT_TRUE((Union<decltype(u)>));
 }

 TEST(Union, non_union_types_do_not_satisfy_constraint) {
  struct A {};
  EXPECT_FALSE((Union<A>));
  EXPECT_FALSE((Union<int[]>));
 }

 TEST(Struct, struct_types_satisfy_constraint) {
  EXPECT_TRUE((Struct<Dummy<>>));
  struct A {};
  EXPECT_TRUE((Struct<A>));
 }

 TEST(Struct, non_struct_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Struct<float *>));
  enum struct E {};
  EXPECT_FALSE((Struct<E>));
 }

 TEST(Integral, integral_types_satisfy_constraint) {
  EXPECT_TRUE((Integral<bool>));
  EXPECT_TRUE((Integral<char>));
  EXPECT_TRUE((Integral<wchar_t>));
  EXPECT_TRUE((Integral<char8_t>));
  EXPECT_TRUE((Integral<char16_t>));
  EXPECT_TRUE((Integral<char32_t>));
  EXPECT_TRUE((Integral<short>));
  EXPECT_TRUE((Integral<int>));
  EXPECT_TRUE((Integral<long>));
  EXPECT_TRUE((Integral<long long>));
 }

 TEST(Integral, non_integral_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Integral<float>));
  EXPECT_FALSE((Integral<Dummy<>>));
  EXPECT_FALSE((Integral<void *>));
 }

 TEST(Floating, floating_precision_types_satisfy_constraint) {
  EXPECT_TRUE((Floating<float>));
  EXPECT_TRUE((Floating<double>));
  EXPECT_TRUE((Floating<long double>));
 }

 TEST(Floating, non_floating_precision_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Floating<int>));
  EXPECT_FALSE((Floating<char *>));
 }

 TEST(Arithmetic, arithmetic_types_satisfy_constraint) {
  EXPECT_TRUE((Arithmetic<bool>));
  EXPECT_TRUE((Arithmetic<char>));
  EXPECT_TRUE((Arithmetic<wchar_t>));
  EXPECT_TRUE((Arithmetic<char8_t>));
  EXPECT_TRUE((Arithmetic<char16_t>));
  EXPECT_TRUE((Arithmetic<char32_t>));
  EXPECT_TRUE((Arithmetic<short>));
  EXPECT_TRUE((Arithmetic<int>));
  EXPECT_TRUE((Arithmetic<long>));
  EXPECT_TRUE((Arithmetic<long long>));
  EXPECT_TRUE((Arithmetic<float>));
  EXPECT_TRUE((Arithmetic<double>));
  EXPECT_TRUE((Arithmetic<long double>));
 }

 TEST(Arithmetic, non_arithmetic_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Arithmetic<void *>));
  EXPECT_FALSE((Arithmetic<int[]>));
 }

 TEST(Signed, signed_types_satisfy_constraint) {
  EXPECT_TRUE((Signed<signed char>));
  EXPECT_TRUE((Signed<signed short>));
  EXPECT_TRUE((Signed<signed int>));
  EXPECT_TRUE((Signed<signed long>));
  EXPECT_TRUE((Signed<signed long long>));
  EXPECT_TRUE((Signed<float>));
  EXPECT_TRUE((Signed<double>));
  EXPECT_TRUE((Signed<long double>));
 }

 TEST(Signed, unsigned_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Signed<unsigned char>));
  EXPECT_FALSE((Signed<unsigned short>));
  EXPECT_FALSE((Signed<unsigned int>));
  EXPECT_FALSE((Signed<unsigned long>));
  EXPECT_FALSE((Signed<unsigned long long>));
  EXPECT_FALSE((Signed<char8_t>));
  EXPECT_FALSE((Signed<char16_t>));
  EXPECT_FALSE((Signed<char32_t>));
  enum struct E : unsigned int {};
  EXPECT_FALSE((Signed<E>));
  //Bool is a special case
  EXPECT_FALSE((Signed<bool>));
 }

 TEST(Signed, non_signed_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Signed<signed int[]>));
  EXPECT_FALSE((Signed<signed char *>));
  struct A {};
  EXPECT_FALSE((Signed<A>));
 }

 TEST(Unsigned, unsigned_types_satisfy_constraint) {
  EXPECT_TRUE((Unsigned<unsigned char>));
  EXPECT_TRUE((Unsigned<unsigned short>));
  EXPECT_TRUE((Unsigned<unsigned int>));
  EXPECT_TRUE((Unsigned<unsigned long>));
  EXPECT_TRUE((Unsigned<unsigned long long>));
  //bool is a special case
  EXPECT_TRUE((Unsigned<bool>));
 }

 TEST(Unsigned, signed_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Unsigned<signed char>));
  EXPECT_FALSE((Unsigned<signed short>));
  EXPECT_FALSE((Unsigned<signed int>));
  EXPECT_FALSE((Unsigned<signed long>));
  EXPECT_FALSE((Unsigned<signed long long>));
  EXPECT_FALSE((Unsigned<float>));
  EXPECT_FALSE((Unsigned<double>));
  EXPECT_FALSE((Unsigned<long double>));
  enum struct E : int {};
  EXPECT_FALSE((Unsigned<E>));
 }

 TEST(Unsigned, non_signed_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((Unsigned<unsigned int[]>));
  EXPECT_FALSE((Unsigned<void *>));
  struct A {};
  EXPECT_FALSE((Unsigned<A>));
 }

 TEST(Scalar, scalar_types_satisfy_constraint) {
  EXPECT_TRUE((Arithmetic<bool>));
  EXPECT_TRUE((Arithmetic<char>));
  EXPECT_TRUE((Arithmetic<wchar_t>));
  EXPECT_TRUE((Arithmetic<char8_t>));
  EXPECT_TRUE((Arithmetic<char16_t>));
  EXPECT_TRUE((Arithmetic<char32_t>));
  EXPECT_TRUE((Arithmetic<short>));
  EXPECT_TRUE((Arithmetic<int>));
  EXPECT_TRUE((Arithmetic<long>));
  EXPECT_TRUE((Arithmetic<long long>));
  EXPECT_TRUE((Arithmetic<float>));
  EXPECT_TRUE((Arithmetic<double>));
  EXPECT_TRUE((Arithmetic<long double>));
  enum E {};
  EXPECT_TRUE((Scalar<E>));
  EXPECT_TRUE((Scalar<void *>));
  EXPECT_TRUE((Scalar<int (Dummy<>::*)>));
 }

 TEST(Scalar, non_scalar_types_do_not_satisfy_constraint) {
  struct A {};
  EXPECT_FALSE((Scalar<A>));
  EXPECT_FALSE((Scalar<char[123]>));
 }

 TEST(TriviallyCopyable, trivially_copyable_types_satisfy_constraint) {
  EXPECT_TRUE((TriviallyCopyable<Dummy<>>));
  union U {};
  EXPECT_TRUE((TriviallyCopyable<U>));
  EXPECT_TRUE((TriviallyCopyable<int[123]>));
 }

 TEST(TriviallyCopyable, non_trivially_copyable_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((TriviallyCopyable<char&>));
  struct A {
   virtual ~A() = default;
  };
  EXPECT_FALSE((TriviallyCopyable<A>));
 }

 TEST(Trivial, trivial_types_satisfy_constraint) {
  EXPECT_TRUE((Arithmetic<bool>));
  EXPECT_TRUE((Arithmetic<char>));
  EXPECT_TRUE((Arithmetic<wchar_t>));
  EXPECT_TRUE((Arithmetic<char8_t>));
  EXPECT_TRUE((Arithmetic<char16_t>));
  EXPECT_TRUE((Arithmetic<char32_t>));
  EXPECT_TRUE((Arithmetic<short>));
  EXPECT_TRUE((Arithmetic<int>));
  EXPECT_TRUE((Arithmetic<long>));
  EXPECT_TRUE((Arithmetic<long long>));
  EXPECT_TRUE((Arithmetic<float>));
  EXPECT_TRUE((Arithmetic<double>));
  EXPECT_TRUE((Arithmetic<long double>));
  enum E {};
  EXPECT_TRUE((Scalar<E>));
  EXPECT_TRUE((Scalar<void *>));
  EXPECT_TRUE((Scalar<int (Dummy<>::*)>));
  EXPECT_TRUE((TriviallyCopyable<Dummy<>>));
  union U {};
  EXPECT_TRUE((TriviallyCopyable<U>));
  EXPECT_TRUE((TriviallyCopyable<int[123]>));
 }

 TEST(Trivial, non_trivial_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((TriviallyCopyable<int const&&>));
  struct A {
   virtual void doNothing();
  };
  EXPECT_FALSE((TriviallyCopyable<A>));
 }

 struct S {
  void f1() {}
  int f2() const { return 0; }
  float f3(int) noexcept { return 0; }
  void f4(int, float, double) const noexcept {}
  virtual void f5(...) {}
  virtual signed char f6(void (*)()) const noexcept { return 0; }
  template<typename R>
  void f7(R) const {}

  int m1;
  static float m2;
  char8_t m3;
  static void f8() noexcept {}

  int f9() const & { return 0; }
  void f10() && noexcept {}
 };

 TEST(MemberFunction, member_functions_satisfy_constraint) {
  EXPECT_TRUE((MemberFunction<decltype(&S::f1)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f2)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f3)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f4)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f5)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f6)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f7<long>)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f9)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f10)>));
  EXPECT_TRUE((MemberFunction<void (Dummy<>::*)()>));
 }

 TEST(MemberFunction, conditional_constraint_satisfied_for_expected_functions) {
  EXPECT_TRUE((MemberFunction<decltype(&S::f3), float, int>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f2), int>));
  EXPECT_TRUE((MemberFunction<void (Dummy<>::*)(float, char), void>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f6), signed char, void (*)()>));

  EXPECT_FALSE((MemberFunction<decltype(&S::f1), int>));
  EXPECT_FALSE((MemberFunction<decltype(&S::f2), void>));
  EXPECT_FALSE((MemberFunction<decltype(&S::f3), float, char>));
  EXPECT_FALSE((MemberFunction<decltype(&S::f4), float>));
 }

 TEST(MemberFunction, non_member_functions_do_not_satisfy_constraint) {
  EXPECT_FALSE((MemberFunction<decltype(&S::m1)>));
  EXPECT_FALSE((MemberFunction<decltype(&S::m2)>));
  EXPECT_FALSE((MemberFunction<decltype(&S::f8)>));
  EXPECT_FALSE((MemberFunction<float>));
 }

 void nonMemberFunction();

 TEST(StaticFunction, static_functions_satisfy_constraint) {
  EXPECT_TRUE((StaticFunction<decltype(&S::f8)>));
  EXPECT_TRUE((StaticFunction<decltype(&nonMemberFunction)>));
  EXPECT_TRUE((StaticFunction<int ()>));
  EXPECT_TRUE((StaticFunction<float (&)(double)>));
  EXPECT_TRUE((StaticFunction<long (*)(...)>));
 }

 TEST(StaticFunction, conditional_constraint_satisfied_for_expected_functions) {
  EXPECT_TRUE((StaticFunction<decltype(&S::f8), void>));
  EXPECT_TRUE((StaticFunction<decltype(&nonMemberFunction), void>));
  EXPECT_TRUE((StaticFunction<float (&)(double), float, double>));
  EXPECT_TRUE((StaticFunction<short (...), short>));

  EXPECT_FALSE((StaticFunction<decltype(&S::f4), void, int, float, char>));
  EXPECT_FALSE((StaticFunction<short (&)(float), int>));
  EXPECT_FALSE((StaticFunction<decltype(&nonMemberFunction), void, S>));
  EXPECT_FALSE((StaticFunction<int (), void>));
 }

 TEST(StaticFunction, non_static_functions_do_not_satisfy_constraint) {
  EXPECT_FALSE((StaticFunction<void>));
  EXPECT_FALSE((StaticFunction<decltype(&S::f3)>));
  EXPECT_FALSE((StaticFunction<int& (Dummy<>::*)()>));
 }

 TEST(VariadicFunction, c_variadic_function_types_satisfy_constriant) {
  EXPECT_TRUE((VariadicFunction<void (...)>));
  EXPECT_TRUE((VariadicFunction<void (*)(...)>));
  EXPECT_TRUE((VariadicFunction<void (Dummy<>::*)(...) const & noexcept>));
  EXPECT_TRUE((VariadicFunction<float (...) noexcept>));
 }

 TEST(VariadicFunction, non_c_variadic_function_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((VariadicFunction<void ()>));
  EXPECT_FALSE((VariadicFunction<char (Dummy<>::*)(int)>));
  EXPECT_FALSE((VariadicFunction<int (*)() noexcept>));
 }

 TEST(VariadicFunction, non_function_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((VariadicFunction<float>));
  EXPECT_FALSE((VariadicFunction<char[]>));
  EXPECT_FALSE((VariadicFunction<int *>));
 }

 TEST(FunctionPrototype, static_function_types_yield_function_prototype) {
  EXPECT_TRUE((SameType<FunctionPrototype<void ()>, void ()>));
  EXPECT_TRUE((SameType<FunctionPrototype<char (...)>, char (...)>));
  EXPECT_TRUE((SameType<FunctionPrototype<float (*)(int, ...) noexcept>, float (int, ...) noexcept>));
 }

 TEST(FunctionPrototype, member_function_ptr_types_yield_function_prototype) {
  EXPECT_TRUE((SameType<FunctionPrototype<void (Dummy<>::*)()>, void ()>));
  EXPECT_TRUE((SameType<FunctionPrototype<int (Dummy<>::*)(int, ...) const && noexcept>, int (int, ...) noexcept>));
  EXPECT_TRUE((SameType<FunctionPrototype<float (Dummy<>::*)() &>, float ()>));
 }

 TEST(FunctionPrototype, non_function_types_yield_default_value) {
  EXPECT_TRUE((SameType<FunctionPrototype<void>, ImpossibleType<>>));
  EXPECT_TRUE((SameType<FunctionPrototype<char *>, ImpossibleType<>>));
  EXPECT_TRUE((SameType<FunctionPrototype<float[123]>, ImpossibleType<>>));
 }

 TEST(MemberFunctionPrototype, member_function_types_yield_member_function_prototype) {
  EXPECT_TRUE((SameType<MemberFunctionPrototype<void (Dummy<>::*)()>, void (Dummy<>::*)()>));
  EXPECT_TRUE((SameType<MemberFunctionPrototype<void (Dummy<>::*)(...) const &>, void (Dummy<>::*)(...) const &>));
  EXPECT_TRUE((SameType<MemberFunctionPrototype<float * (Dummy<>::*)(int) &&>, float * (Dummy<>::*)(int) &&>));
 }

 TEST(MemberFunctionPrototype, non_member_functions_yield_default_value) {
  EXPECT_TRUE((SameType<MemberFunctionPrototype<void>, ImpossibleType<>>));
  EXPECT_TRUE((SameType<MemberFunctionPrototype<int ()>, ImpossibleType<>>));
  EXPECT_TRUE((SameType<MemberFunctionPrototype<float (*)(...) noexcept>, ImpossibleType<>>));
 }

 TEST(NoexceptFunction, noexcept_qualified_functions_satisfy_constraint) {
  EXPECT_TRUE((NoexceptFunction<void () noexcept>));
  EXPECT_TRUE((NoexceptFunction<int (*)(...) noexcept>));
  EXPECT_TRUE((NoexceptFunction<float (Dummy<>::*)(...) const noexcept>));
 }

 TEST(NoexceptFunction, non_noexcept_functions_do_not_satisfy_constraint) {
  EXPECT_FALSE((NoexceptFunction<void (*)()>));
  EXPECT_FALSE((NoexceptFunction<char (Dummy<>::*)(float, ...) const &>));
 }

 TEST(NoexceptFunction, non_function_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((NoexceptFunction<float>));
  EXPECT_FALSE((NoexceptFunction<long double>));
  EXPECT_FALSE((NoexceptFunction<void *>));
 }

 TEST(VirtualFunction, virtual_functions_satisfy_constraint) {
  EXPECT_TRUE((VirtualFunction<&S::f5>));
  EXPECT_TRUE((VirtualFunction<&S::f6>));
 }

 TEST(VirtualFunction, non_virtual_functions_do_not_satisfy_constraint) {
  EXPECT_FALSE((VirtualFunction<&S::f1>));
  EXPECT_FALSE((VirtualFunction<&nonMemberFunction>));
  EXPECT_FALSE((VirtualFunction<5678>));
 }

 TEST(FunctionWithPrototype, matching_function_prototypes_satisfy_constraint) {
  EXPECT_TRUE((FunctionWithPrototype<void (), void ()>));
  EXPECT_TRUE((FunctionWithPrototype<void (Dummy<>::*)() const && noexcept, void (Dummy<>::*)() const && noexcept>));
  EXPECT_TRUE((FunctionWithPrototype<void (Dummy<>::*)(...) noexcept, void (...) noexcept>));
  EXPECT_TRUE((FunctionWithPrototype<char * (*)(float, ...) noexcept, char * (float, ...) noexcept>));
 }

 TEST(FunctionWithPrototype, struct_with_function_operator_matching_prototype_satisfies_constraint) {
  struct A {
   void operator()() {}
   int operator()(...) & { return 0; }
   float * operator()(int, char, ...) volatile & noexcept { return nullptr; }
   long double ** operator()(...) const && noexcept { return nullptr; }
  };

  EXPECT_TRUE((FunctionWithPrototype<A, void ()>));
  EXPECT_TRUE((FunctionWithPrototype<A, void (A::*)()>));
  EXPECT_TRUE((FunctionWithPrototype<A, int (A::*)(...) &>));
  EXPECT_TRUE((FunctionWithPrototype<A, float * (A::*)(int, char ...) volatile & noexcept>));
  EXPECT_TRUE((FunctionWithPrototype<A, long double ** (A::*)(...) const && noexcept>));

  auto lambda = [] {};
  EXPECT_TRUE((FunctionWithPrototype<decltype(lambda), void (decltype(lambda)::*)() const>));
 }

 TEST(FunctionWithPrototype, non_matching_function_prototypes_do_not_satisfy_constraint) {
  EXPECT_FALSE((FunctionWithPrototype<float (), char ()>));
  EXPECT_FALSE((FunctionWithPrototype<double (Dummy<>::*)() &, double (Dummy<>::*)()>));
  EXPECT_FALSE((FunctionWithPrototype<void (Dummy<>::*)() noexcept, void ()>));
 }

 TEST(FunctionWithPrototype, struct_with_function_operator_not_matching_prototype_does_not_satisfy_constraint) {
  struct A {
   void operator()(float(&)[1234]) {};
   char operator()() const volatile && noexcept { return 0; };
  };

  EXPECT_FALSE((FunctionWithPrototype<A, void (float(&)[12])>));
  EXPECT_FALSE((FunctionWithPrototype<A, char (A::*)() volatile && noexcept>));
 }

 TEST(MemberField, member_fields_satisfy_constraint) {
  EXPECT_TRUE((MemberField<decltype(&S::m1)>));
 }

 TEST(MemberField, conditional_constraint_satisfied_for_expected_fields) {
  EXPECT_TRUE((MemberField<decltype(&S::m3), char8_t>));
  EXPECT_TRUE((MemberField<decltype(&S::m1), int, S>));
 }

 TEST(MemberField, non_member_fields_do_not_satisfy_constraint) {
  EXPECT_FALSE((MemberField<decltype(&S::f1)>));
  EXPECT_FALSE((MemberField<decltype(&nonMemberFunction)>));
  EXPECT_FALSE((MemberField<void *>));
  EXPECT_FALSE((MemberField<decltype(&S::m2)>));
 }

 //All operator overloads
 namespace OperatorConceptTests {
  struct UnqualifiedOperators {
   void operator+(int) {}
   void operator-(int) {}
   void operator/(int) {}
   void operator*(int) {}
   void operator%(int) {}
   void operator&&(int) {}
   void operator||(int) {}
   void operator<<(int) {}
   void operator>>(int) {}
   void operator>(int) {}
   void operator<(int) {}
   void operator==(int) {}
   void operator!=(int) {}
   void operator>=(int) {}
   void operator<=(int) {}
   void operator=(int) {}
   void operator+=(int) {}
   void operator-=(int) {}
   void operator*=(int) {}
   void operator/=(int) {}
   void operator%=(int) {}
   void operator|=(int) {}
   void operator&=(int) {}
   void operator^=(int) {}
   void operator<<=(int) {}
   void operator>>=(int) {}
   void operator,(int) {}
   void operator<=>(int) {}
   void operator->() {}
   void operator->*(int) {}
   void operator[](int) {}
   void operator()(int) {}
   void operator~() {}
   void operator++(int) {}
   void operator--(int) {}
   void operator!() {}
   void operator&(int) {}
   operator int() { return 0; }
  };

  TEST(Operators, unqualified_operators_satisfy_constraints) {
   EXPECT_TRUE((AdditionOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((SubtractionOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((DivisionOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((MultiplicationOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((ModulusOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((BinaryAndOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((BinaryOrOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((LeftShiftOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((RightShiftOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((GreaterThanOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((LessThanOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((EqualityOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((InequalityOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((GreaterOrEqualThanOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((LessOrEqualThanOperator<UnqualifiedOperators, void (int)>));
   //Must explicitly specify the return type and arguments
   //for the assignment operator concept since there may be l/r-value
   //assignment operators present
   EXPECT_TRUE((AssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((AdditionAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((SubtractionAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((MultiplicationAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((DivisionAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((ModulusAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((OrAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((AndAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((XORAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((LeftShiftAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((RightShiftAssignmentOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((CommaOperator<UnqualifiedOperators, void (int)>));

   //Special operators
   EXPECT_TRUE((ThreeWayComparisonOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((MemberAccessOperator<UnqualifiedOperators, void ()>));
   EXPECT_TRUE((MemberPointerAccessOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((SubscriptOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((FunctionOperator<UnqualifiedOperators, void (int)>));

   //Unary operators
   EXPECT_TRUE((DereferenceOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((ComplimentOperator<UnqualifiedOperators, void ()>));
   EXPECT_TRUE((IncrementOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((DecrementOperator<UnqualifiedOperators, void (int)>));
   EXPECT_TRUE((NotOperator<UnqualifiedOperators, void ()>));
   EXPECT_TRUE((AddressOperator<UnqualifiedOperators, void (int)>));

   //Must explicitly specify the result of the conversion operator
   //since you can't directly address it
   EXPECT_TRUE((ConversionOperator<UnqualifiedOperators, int>));
  }

  struct QualifiedOperators {
   virtual void operator+(int) const noexcept {}
   virtual void operator-(int) const noexcept {}
   virtual void operator/(int) const noexcept {}
   virtual void operator*(int) const noexcept {}
   virtual void operator%(int) const noexcept {}
   virtual void operator&&(int) const noexcept {}
   virtual void operator||(int) const noexcept {}
   virtual void operator<<(int) const noexcept {}
   virtual void operator>>(int) const noexcept {}
   virtual void operator>(int) const noexcept {}
   virtual void operator<(int) const noexcept {}
   virtual void operator==(int) const noexcept {}
   virtual void operator!=(int) const noexcept {}
   virtual void operator>=(int) const noexcept {}
   virtual void operator<=(int) const noexcept {}
   virtual void operator=(int) const noexcept {}
   virtual void operator+=(int) const noexcept {}
   virtual void operator-=(int) const noexcept {}
   virtual void operator*=(int) const noexcept {}
   virtual void operator/=(int) const noexcept {}
   virtual void operator%=(int) const noexcept {}
   virtual void operator|=(int) const noexcept {}
   virtual void operator&=(int) const noexcept {}
   virtual void operator^=(int) const noexcept {}
   virtual void operator<<=(int) const noexcept {}
   virtual void operator>>=(int) const noexcept {}
   virtual void operator,(int) const noexcept {}
   virtual void operator<=>(int) const noexcept {}
   virtual void operator->() const noexcept {}
   virtual void operator->*(int) const noexcept {}
   virtual void operator[](int) const noexcept {}
   virtual void operator()(int) const noexcept {}
   virtual void operator~() const noexcept {}
   virtual void operator++(int) const noexcept {}
   virtual void operator--(int) const noexcept {}
   virtual void operator!() const noexcept {}
   virtual void operator&(int) const noexcept {}
   virtual operator int() const noexcept { return 0; }
  };

  TEST(Operators, qualified_operators_satisfy_constraints) {
   EXPECT_TRUE((AdditionOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((SubtractionOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((DivisionOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((MultiplicationOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((ModulusOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((BinaryAndOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((BinaryOrOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((LeftShiftOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((RightShiftOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((GreaterThanOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((LessThanOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((EqualityOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((InequalityOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((GreaterOrEqualThanOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((LessOrEqualThanOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   //Must explicitly specify the return type and arguments
   //for the assignment operator concept since there may be l/r-value
   //assignment operators present
   EXPECT_TRUE((AssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((AdditionAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((SubtractionAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((MultiplicationAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((DivisionAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((ModulusAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((OrAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((AndAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((XORAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((LeftShiftAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((RightShiftAssignmentOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((CommaOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));

   //Special operators
   EXPECT_TRUE((ThreeWayComparisonOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((MemberAccessOperator<QualifiedOperators, void (QualifiedOperators::*)() const noexcept>));
   EXPECT_TRUE((MemberPointerAccessOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((SubscriptOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((FunctionOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));

   //Unary operators
   EXPECT_TRUE((DereferenceOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((ComplimentOperator<QualifiedOperators, void (QualifiedOperators::*)() const noexcept>));
   EXPECT_TRUE((IncrementOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((DecrementOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));
   EXPECT_TRUE((NotOperator<QualifiedOperators, void (QualifiedOperators::*)() const noexcept>));
   EXPECT_TRUE((AddressOperator<QualifiedOperators, void (QualifiedOperators::*)(int) const noexcept>));

   //Must explicitly specify the result of the conversion operator
   //since you can't directly address it
   EXPECT_TRUE((ConversionOperator<QualifiedOperators, int>));
  }

  struct TemplateOperators {
   template<typename A> void operator+(A) {}
   template<typename A> void operator-(A) {}
   template<typename A> void operator/(A) {}
   template<typename A> void operator*(A) {}
   template<typename A> void operator%(A) {}
   template<typename A> void operator&&(A) {}
   template<typename A> void operator||(A) {}
   template<typename A> void operator<<(A) {}
   template<typename A> void operator>>(A) {}
   template<typename A> void operator>(A) {}
   template<typename A> void operator<(A) {}
   template<typename A> void operator==(A) {}
   template<typename A> void operator!=(A) {}
   template<typename A> void operator>=(A) {}
   template<typename A> void operator<=(A) {}
   template<typename A> void operator=(A) {}
   template<typename A> void operator+=(A) {}
   template<typename A> void operator-=(A) {}
   template<typename A> void operator*=(A) {}
   template<typename A> void operator/=(A) {}
   template<typename A> void operator%=(A) {}
   template<typename A> void operator|=(A) {}
   template<typename A> void operator&=(A) {}
   template<typename A> void operator^=(A) {}
   template<typename A> void operator<<=(A) {}
   template<typename A> void operator>>=(A) {}
   template<typename A> void operator,(A) {}
   template<typename A> void operator<=>(A) {}
   template<typename = void> void operator->() {}
   template<typename A> void operator->*(A) {}
   template<typename A> void operator[](A) {}
   template<typename A> void operator()(A) {}
   template<typename = void> void operator~() {}
   template<typename A> void operator++(A) {}
   template<typename A> void operator--(A) {}
   template<typename = void> void operator!() {}
   template<typename A> void operator&(A) {}
   template<typename A> [[noreturn]] operator A() { while (true); }
  };

  TEST(Operators, templated_operators_satisfy_conditional_constraints) {
   EXPECT_TRUE((AdditionOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((SubtractionOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((DivisionOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((MultiplicationOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((ModulusOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((BinaryAndOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((BinaryOrOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((LeftShiftOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((RightShiftOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((GreaterThanOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((LessThanOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((EqualityOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((InequalityOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((GreaterOrEqualThanOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((LessOrEqualThanOperator<TemplateOperators, void (int)>));
   //Must explicitly specify the return type and arguments
   //for the assignment operator concept since there may be l/r-value
   //assignment operators present
   EXPECT_TRUE((AssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((AdditionAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((SubtractionAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((MultiplicationAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((DivisionAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((ModulusAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((OrAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((AndAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((XORAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((LeftShiftAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((RightShiftAssignmentOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((CommaOperator<TemplateOperators, void (int)>));

   //Special operators
   EXPECT_TRUE((ThreeWayComparisonOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((MemberAccessOperator<TemplateOperators, void ()>));
   EXPECT_TRUE((MemberPointerAccessOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((SubscriptOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((FunctionOperator<TemplateOperators, void (int)>));

   //Unary operators
   EXPECT_TRUE((DereferenceOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((ComplimentOperator<TemplateOperators, void ()>));
   EXPECT_TRUE((IncrementOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((DecrementOperator<TemplateOperators, void (int)>));
   EXPECT_TRUE((NotOperator<TemplateOperators, void ()>));
   EXPECT_TRUE((AddressOperator<TemplateOperators, void (int)>));

   //Must explicitly specify the result of the conversion operator
   //since you can't directly address it
   EXPECT_TRUE((ConversionOperator<TemplateOperators, int>));
  }

  TEST(Operators, non_operators_do_not_satisfy_constraints) {
   EXPECT_FALSE((AdditionOperator<S, void ()>));
   EXPECT_FALSE((SubtractionOperator<S, void ()>));
   EXPECT_FALSE((DivisionOperator<S, void ()>));
   EXPECT_FALSE((MultiplicationOperator<S, void ()>));
   EXPECT_FALSE((ModulusOperator<S, void ()>));
   EXPECT_FALSE((BinaryAndOperator<S, void ()>));
   EXPECT_FALSE((BinaryOrOperator<S, void ()>));
   EXPECT_FALSE((LeftShiftOperator<S, void ()>));
   EXPECT_FALSE((RightShiftOperator<S, void ()>));
   EXPECT_FALSE((GreaterThanOperator<S, void ()>));
   EXPECT_FALSE((LessThanOperator<S, void ()>));
   EXPECT_FALSE((EqualityOperator<S, void ()>));
   EXPECT_FALSE((InequalityOperator<S, void ()>));
   EXPECT_FALSE((GreaterOrEqualThanOperator<S, void ()>));
   EXPECT_FALSE((LessOrEqualThanOperator<S, void ()>));
   //Must explicitly specify the return type and arguments
   //for the assignment operator concept since there may be l/r-value
   //assignment operators present
   EXPECT_FALSE((AssignmentOperator<S, void ()>));
   EXPECT_FALSE((AdditionAssignmentOperator<S, void ()>));
   EXPECT_FALSE((SubtractionAssignmentOperator<S, void ()>));
   EXPECT_FALSE((MultiplicationAssignmentOperator<S, void ()>));
   EXPECT_FALSE((DivisionAssignmentOperator<S, void ()>));
   EXPECT_FALSE((ModulusAssignmentOperator<S, void ()>));
   EXPECT_FALSE((OrAssignmentOperator<S, void ()>));
   EXPECT_FALSE((AndAssignmentOperator<S, void ()>));
   EXPECT_FALSE((XORAssignmentOperator<S, void ()>));
   EXPECT_FALSE((LeftShiftAssignmentOperator<S, void ()>));
   EXPECT_FALSE((RightShiftAssignmentOperator<S, void ()>));
   EXPECT_FALSE((CommaOperator<S, void ()>));

   //Special operators
   EXPECT_FALSE((ThreeWayComparisonOperator<S, void ()>));
   EXPECT_FALSE((MemberAccessOperator<S, void ()>));
   EXPECT_FALSE((MemberPointerAccessOperator<S, void ()>));
   EXPECT_FALSE((SubscriptOperator<S, void ()>));
   EXPECT_FALSE((FunctionOperator<S, void ()>));

   //Unary operators
   EXPECT_FALSE((DereferenceOperator<S, void ()>));
   EXPECT_FALSE((ComplimentOperator<S, void ()>));
   EXPECT_FALSE((IncrementOperator<S, void ()>));
   EXPECT_FALSE((DecrementOperator<S, void ()>));
   EXPECT_FALSE((NotOperator<S, void ()>));
   EXPECT_FALSE((AddressOperator<S, void ()>));

   //Must explicitly specify the result of the conversion operator
   //since you can't directly address it
   EXPECT_FALSE((ConversionOperator<S, int>));
  }
 }
}
