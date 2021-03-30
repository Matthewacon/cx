#include <cx/test/common/common.h>

#include <functional>

#include <cx/idioms.h>

namespace CX {
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
  EXPECT_FALSE((Constructible<void>));
  EXPECT_FALSE((Constructible<int[]>));
  struct A {
   A(A const&) = delete;
  };
  EXPECT_FALSE((Constructible<A>));
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
  using TypeA = char [1234];
  using ExpectedTypeA = char [];
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

 TEST(ArrayElementType, array_types_yield_array_element_type) {
  using TypeA = char [123];
  using ExpectedTypeA = char;
  EXPECT_TRUE((SameType<ArrayElementType<TypeA>, ExpectedTypeA>));

  using TypeB = float[];
  using ExpectedTypeB = float;
  EXPECT_TRUE((SameType<ArrayElementType<TypeB>, ExpectedTypeB>));

  using TypeC = void * const[34];
  using ExpectedTypeC = void * const;
  EXPECT_TRUE((SameType<ArrayElementType<TypeC>, ExpectedTypeC>));

  using TypeD = float (Dummy<>::* const[5124])() const;
  using ExpectedTypeD = float (Dummy<>::* const)() const;
  EXPECT_TRUE((SameType<ArrayElementType<TypeD>, ExpectedTypeD>));
 }

 TEST(ArrayElementType, non_array_types_yield_the_same_type) {
  using TypeA = char (Dummy<>::*)() noexcept;
  using ExpectedTypeA = TypeA;
  EXPECT_TRUE((SameType<ArrayElementType<TypeA>, ExpectedTypeA>));

  using TypeB = float const * const;
  using ExpectedTypeB = TypeB;
  EXPECT_TRUE((SameType<ArrayElementType<TypeB>, ExpectedTypeB>));

  using TypeC = void;
  using ExpectedTypeC = TypeC;
  EXPECT_TRUE((SameType<ArrayElementType<TypeC>, ExpectedTypeC>));
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

 TEST(LValueReferenceElementType, lvalue_reference_types_yield_the_reference_element_type) {
  using TypeA = int&;
  using ExpectedTypeA = int;
  EXPECT_TRUE((SameType<LValueReferenceElementType<TypeA>, ExpectedTypeA>));

  using TypeB = char * const&;
  using ExpectedTypeB = char * const;
  EXPECT_TRUE((SameType<LValueReferenceElementType<TypeB>, ExpectedTypeB>));

  using TypeC = void (&)();
  using ExpectedTypeC = void ();
  EXPECT_TRUE((SameType<LValueReferenceElementType<TypeC>, ExpectedTypeC>));
 }

 TEST(LValueReferenceElementType, non_lvalue_reference_types_yield_the_same_type) {
  using TypeA = char *;
  using ExpectedTypeA = TypeA;
  EXPECT_TRUE((SameType<LValueReferenceElementType<TypeA>, ExpectedTypeA>));

  using TypeB = float ();
  using ExpectedTypeB = TypeB;
  EXPECT_TRUE((SameType<LValueReferenceElementType<TypeB>, ExpectedTypeB>));

  using TypeC = void;
  using ExpectedTypeC = TypeC;
  EXPECT_TRUE((SameType<LValueReferenceElementType<TypeC>, ExpectedTypeC>));
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

 TEST(RValueReferenceElementType, rvalue_reference_types_yield_the_reference_element_type) {
  using TypeA = Dummy<>&&;
  using ExpectedTypeA = Dummy<>;
  EXPECT_TRUE((SameType<RValueReferenceElementType<TypeA>, ExpectedTypeA>));

  using TypeB = char (Dummy<>::*&&)();
  using ExpectedTypeB = char (Dummy<>::*)();
  EXPECT_TRUE((SameType<RValueReferenceElementType<TypeB>, ExpectedTypeB>));

  using TypeC = void *&&;
  using ExpectedTypeC = void *;
  EXPECT_TRUE((SameType<RValueReferenceElementType<TypeC>, ExpectedTypeC>));
 }

 TEST(RValueReferenceElementType, non_rvalue_reference_types_yield_the_same_type) {
  using TypeA = float *;
  using ExpectedTypeA = TypeA;
  EXPECT_TRUE((SameType<RValueReferenceElementType<TypeA>, ExpectedTypeA>));

  using TypeB = void (Dummy<>::*[])() const;
  using ExpectedTypeB = TypeB;
  EXPECT_TRUE((SameType<RValueReferenceElementType<TypeB>, ExpectedTypeB>));

  using TypeC = void *;
  using ExpectedTypeC = TypeC;
  EXPECT_TRUE((SameType<RValueReferenceElementType<TypeC>, ExpectedTypeC>));
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
  static void f8() noexcept {}
 };

 TEST(MemberFunction, member_functions_satisfy_constraint) {
  EXPECT_TRUE((MemberFunction<decltype(&S::f1)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f2)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f3)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f4)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f5)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f6)>));
  EXPECT_TRUE((MemberFunction<decltype(&S::f7<long>)>));
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

 TEST(VirtualFunction, virtual_functions_satisfy_constraint) {
  EXPECT_TRUE((VirtualFunction<&S::f5>));
  EXPECT_TRUE((VirtualFunction<&S::f6>));
 }

 TEST(VirtualFunction, non_virtual_functions_do_not_satisfy_constraint) {
  EXPECT_FALSE((VirtualFunction<&S::f1>));
  EXPECT_FALSE((VirtualFunction<&nonMemberFunction>));
  EXPECT_FALSE((VirtualFunction<5678>));
 }

 TEST(MemberField, member_fields_satisfy_constraint) {
  EXPECT_TRUE((MemberField<decltype(&S::m1)>));
 }

 TEST(MemberField, non_member_fields_do_not_satisfy_constraint) {
  EXPECT_FALSE((MemberField<decltype(&S::f1)>));
  EXPECT_FALSE((MemberField<decltype(&nonMemberFunction)>));
  EXPECT_FALSE((MemberField<void *>));
  EXPECT_FALSE((MemberField<decltype(&S::m2)>));
 }

 TEST(Function, all_functions_satisfy_constraint) {
  EXPECT_TRUE((Function<void ()>));
  EXPECT_TRUE((Function<void (*)()>));
  EXPECT_TRUE((Function<void (&)()>));
  EXPECT_TRUE((Function<void (&)() noexcept>));
  EXPECT_TRUE((Function<void (&)(...) noexcept>));
  EXPECT_TRUE((Function<void (Dummy<>::*)()>));
  EXPECT_TRUE((Function<void (Dummy<>::*)() const>));
  EXPECT_TRUE((Function<void (Dummy<>::*)() const noexcept>));
  EXPECT_TRUE((Function<void (Dummy<>::*)(...) const noexcept>));
  EXPECT_TRUE((Function<std::function<void ()>>));
  EXPECT_TRUE((Function<decltype(&S::f1)>));
  EXPECT_TRUE((Function<decltype(&S::f5)>));
  EXPECT_TRUE((Function<decltype(nonMemberFunction)>));
 }

 TEST(Function, conditional_constraint_satisified_for_expected_functions) {
  EXPECT_TRUE((Function<decltype(&S::f1), void>));
  EXPECT_TRUE((Function<decltype(&S::f3), float, int>));
  EXPECT_TRUE((Function<decltype(&S::f6), signed char>));
  EXPECT_TRUE((Function<std::function<double (float)>, double>));

  EXPECT_FALSE((Function<void (...), char>));
  EXPECT_FALSE((Function<decltype(&nonMemberFunction), float, int>));
  EXPECT_FALSE((Function<decltype(&S::f1), short>));
  EXPECT_FALSE((Function<decltype(&S::f6), int, void (*)()>));
 }

 TEST(Function, non_functions_do_not_satisfy_constraint) {
  EXPECT_FALSE((Function<S>));
  EXPECT_FALSE((Function<unsigned char[512]>));
  EXPECT_FALSE((Function<int * const *&>));
  EXPECT_FALSE((Function<void>));
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
   EXPECT_TRUE((AdditionOperator<UnqualifiedOperators>));
   EXPECT_TRUE((SubtractionOperator<UnqualifiedOperators>));
   EXPECT_TRUE((DivisionOperator<UnqualifiedOperators>));
   EXPECT_TRUE((MultiplicationOperator<UnqualifiedOperators>));
   EXPECT_TRUE((ModulusOperator<UnqualifiedOperators>));
   EXPECT_TRUE((BinaryAndOperator<UnqualifiedOperators>));
   EXPECT_TRUE((BinaryOrOperator<UnqualifiedOperators>));
   EXPECT_TRUE((LeftShiftOperator<UnqualifiedOperators>));
   EXPECT_TRUE((RightShiftOperator<UnqualifiedOperators>));
   EXPECT_TRUE((GreaterThanOperator<UnqualifiedOperators>));
   EXPECT_TRUE((LessThanOperator<UnqualifiedOperators>));
   EXPECT_TRUE((EqualityOperator<UnqualifiedOperators>));
   EXPECT_TRUE((InequalityOperator<UnqualifiedOperators>));
   EXPECT_TRUE((GreaterOrEqualThanOperator<UnqualifiedOperators>));
   EXPECT_TRUE((LessOrEqualThanOperator<UnqualifiedOperators>));
   //Must explicitly specify the return type and arguments
   //for the assignment operator concept since there may be l/r-value
   //assignment operators present
   EXPECT_TRUE((AssignmentOperator<UnqualifiedOperators, void, int>));
   EXPECT_TRUE((AdditionAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((SubtractionAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((MultiplicationAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((DivisionAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((ModulusAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((OrAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((AndAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((XORAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((LeftShiftAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((RightShiftAssignmentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((CommaOperator<UnqualifiedOperators>));

   //Special operators
   EXPECT_TRUE((ThreeWayComparisonOperator<UnqualifiedOperators>));
   EXPECT_TRUE((MemberAccessOperator<UnqualifiedOperators>));
   EXPECT_TRUE((MemberPointerAccessOperator<UnqualifiedOperators>));
   EXPECT_TRUE((SubscriptOperator<UnqualifiedOperators>));
   EXPECT_TRUE((FunctionOperator<UnqualifiedOperators>));

   //Unary operators
   EXPECT_TRUE((DereferenceOperator<UnqualifiedOperators>));
   EXPECT_TRUE((ComplimentOperator<UnqualifiedOperators>));
   EXPECT_TRUE((IncrementOperator<UnqualifiedOperators>));
   EXPECT_TRUE((DecrementOperator<UnqualifiedOperators>));
   EXPECT_TRUE((NotOperator<UnqualifiedOperators>));
   EXPECT_TRUE((AddressOperator<UnqualifiedOperators>));

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
   EXPECT_TRUE((AdditionOperator<QualifiedOperators>));
   EXPECT_TRUE((SubtractionOperator<QualifiedOperators>));
   EXPECT_TRUE((DivisionOperator<QualifiedOperators>));
   EXPECT_TRUE((MultiplicationOperator<QualifiedOperators>));
   EXPECT_TRUE((ModulusOperator<QualifiedOperators>));
   EXPECT_TRUE((BinaryAndOperator<QualifiedOperators>));
   EXPECT_TRUE((BinaryOrOperator<QualifiedOperators>));
   EXPECT_TRUE((LeftShiftOperator<QualifiedOperators>));
   EXPECT_TRUE((RightShiftOperator<QualifiedOperators>));
   EXPECT_TRUE((GreaterThanOperator<QualifiedOperators>));
   EXPECT_TRUE((LessThanOperator<QualifiedOperators>));
   EXPECT_TRUE((EqualityOperator<QualifiedOperators>));
   EXPECT_TRUE((InequalityOperator<QualifiedOperators>));
   EXPECT_TRUE((GreaterOrEqualThanOperator<QualifiedOperators>));
   EXPECT_TRUE((LessOrEqualThanOperator<QualifiedOperators>));
   //Must explicitly specify the return type and arguments
   //for the assignment operator concept since there may be l/r-value
   //assignment operators present
   EXPECT_TRUE((AssignmentOperator<QualifiedOperators, void, int>));
   EXPECT_TRUE((AdditionAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((SubtractionAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((MultiplicationAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((DivisionAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((ModulusAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((OrAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((AndAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((XORAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((LeftShiftAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((RightShiftAssignmentOperator<QualifiedOperators>));
   EXPECT_TRUE((CommaOperator<QualifiedOperators>));

   //Special operators
   EXPECT_TRUE((ThreeWayComparisonOperator<QualifiedOperators>));
   EXPECT_TRUE((MemberAccessOperator<QualifiedOperators>));
   EXPECT_TRUE((MemberPointerAccessOperator<QualifiedOperators>));
   EXPECT_TRUE((SubscriptOperator<QualifiedOperators>));
   EXPECT_TRUE((FunctionOperator<QualifiedOperators>));

   //Unary operators
   EXPECT_TRUE((DereferenceOperator<QualifiedOperators>));
   EXPECT_TRUE((ComplimentOperator<QualifiedOperators>));
   EXPECT_TRUE((IncrementOperator<QualifiedOperators>));
   EXPECT_TRUE((DecrementOperator<QualifiedOperators>));
   EXPECT_TRUE((NotOperator<QualifiedOperators>));
   EXPECT_TRUE((AddressOperator<QualifiedOperators>));

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
   template<typename A> void operator->() {}
   template<typename A> void operator->*(A) {}
   template<typename A> void operator[](A) {}
   template<typename A> void operator()(A) {}
   template<typename A> void operator~() {}
   template<typename A> void operator++(A) {}
   template<typename A> void operator--(A) {}
   template<typename A> void operator!() {}
   template<typename A> void operator&(A) {}
   template<typename A> operator A() { throw; }
  };

  TEST(Operators, templated_operators_satisfy_conditional_constraints) {
   EXPECT_TRUE((AdditionOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((SubtractionOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((DivisionOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((MultiplicationOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((ModulusOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((BinaryAndOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((BinaryOrOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((LeftShiftOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((RightShiftOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((GreaterThanOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((LessThanOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((EqualityOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((InequalityOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((GreaterOrEqualThanOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((LessOrEqualThanOperator<TemplateOperators, void, int>));
   //Must explicitly specify the return type and arguments
   //for the assignment operator concept since there may be l/r-value
   //assignment operators present
   EXPECT_TRUE((AssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((AdditionAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((SubtractionAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((MultiplicationAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((DivisionAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((ModulusAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((OrAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((AndAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((XORAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((LeftShiftAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((RightShiftAssignmentOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((CommaOperator<TemplateOperators, void, int>));

   //Special operators
   EXPECT_TRUE((ThreeWayComparisonOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((MemberAccessOperator<TemplateOperators, void>));
   EXPECT_TRUE((MemberPointerAccessOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((SubscriptOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((FunctionOperator<TemplateOperators, void, int>));

   //Unary operators
   EXPECT_TRUE((DereferenceOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((ComplimentOperator<TemplateOperators, void>));
   EXPECT_TRUE((IncrementOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((DecrementOperator<TemplateOperators, void, int>));
   EXPECT_TRUE((NotOperator<TemplateOperators, void>));
   EXPECT_TRUE((AddressOperator<TemplateOperators, void, int>));

   //Must explicitly specify the result of the conversion operator
   //since you can't directly address it
   EXPECT_TRUE((ConversionOperator<TemplateOperators, int>));
  }

  TEST(Operators, non_operators_do_not_satisfy_constraints) {
   EXPECT_FALSE((AdditionOperator<S>));
   EXPECT_FALSE((SubtractionOperator<S>));
   EXPECT_FALSE((DivisionOperator<S>));
   EXPECT_FALSE((MultiplicationOperator<S>));
   EXPECT_FALSE((ModulusOperator<S>));
   EXPECT_FALSE((BinaryAndOperator<S>));
   EXPECT_FALSE((BinaryOrOperator<S>));
   EXPECT_FALSE((LeftShiftOperator<S>));
   EXPECT_FALSE((RightShiftOperator<S>));
   EXPECT_FALSE((GreaterThanOperator<S>));
   EXPECT_FALSE((LessThanOperator<S>));
   EXPECT_FALSE((EqualityOperator<S>));
   EXPECT_FALSE((InequalityOperator<S>));
   EXPECT_FALSE((GreaterOrEqualThanOperator<S>));
   EXPECT_FALSE((LessOrEqualThanOperator<S>));
   //Must explicitly specify the return type and arguments
   //for the assignment operator concept since there may be l/r-value
   //assignment operators present
   EXPECT_FALSE((AssignmentOperator<S, void, int>));
   EXPECT_FALSE((AdditionAssignmentOperator<S>));
   EXPECT_FALSE((SubtractionAssignmentOperator<S>));
   EXPECT_FALSE((MultiplicationAssignmentOperator<S>));
   EXPECT_FALSE((DivisionAssignmentOperator<S>));
   EXPECT_FALSE((ModulusAssignmentOperator<S>));
   EXPECT_FALSE((OrAssignmentOperator<S>));
   EXPECT_FALSE((AndAssignmentOperator<S>));
   EXPECT_FALSE((XORAssignmentOperator<S>));
   EXPECT_FALSE((LeftShiftAssignmentOperator<S>));
   EXPECT_FALSE((RightShiftAssignmentOperator<S>));
   EXPECT_FALSE((CommaOperator<S>));

   //Special operators
   EXPECT_FALSE((ThreeWayComparisonOperator<S>));
   EXPECT_FALSE((MemberAccessOperator<S>));
   EXPECT_FALSE((MemberPointerAccessOperator<S>));
   EXPECT_FALSE((SubscriptOperator<S>));
   EXPECT_FALSE((FunctionOperator<S>));

   //Unary operators
   EXPECT_FALSE((DereferenceOperator<S>));
   EXPECT_FALSE((ComplimentOperator<S>));
   EXPECT_FALSE((IncrementOperator<S>));
   EXPECT_FALSE((DecrementOperator<S>));
   EXPECT_FALSE((NotOperator<S>));
   EXPECT_FALSE((AddressOperator<S>));

   //Must explicitly specify the result of the conversion operator
   //since you can't directly address it
   EXPECT_FALSE((ConversionOperator<S, int>));
  }
 }
}
