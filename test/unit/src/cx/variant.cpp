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
  EXPECT_TRUE((CompatibleVariant<Variant<double, char, float>, Variant<float, double, char, int[1234]>>));
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
  EXPECT_FALSE((CompatibleVariant<Variant<float[123]>, Variant<>>));
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

 TEST(Variant, variant_type_disallows_unsized_array_elements) {
  EXPECT_FALSE((Specializable<Variant, int[]>));
  EXPECT_FALSE((Specializable<Variant, char, char[]>));
  EXPECT_FALSE((Specializable<Variant, float, int, double[]>));
 }

 //Constructor tests
 TEST(Constructor, element_copy_constructor_with_copy_constructible_type_properly_initializes_variant) {
  //Test with integral type
  using ExpectedTypeA = char;
  ExpectedTypeA const expectedValueA = 123;
  Variant<int, ExpectedTypeA> v{expectedValueA};
  EXPECT_TRUE((v.has<ExpectedTypeA>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v.get<ExpectedTypeA>()), expectedValueA);
  }()));

  //Test with copy constructible type
  //Ensure element copy constructor is properly invoked
  static bool copyConstructorInvoked;
  //Reset static flag in case test is re-run
  copyConstructorInvoked = false;
  using ExpectedTypeB = struct B {
   int i;

   B(int i) :
    i(i)
   {}

   //Explicitly delete move constructor
   B(B&&) = delete;

   B(B const &a) :
    i(a.i)
   {
    copyConstructorInvoked = true;
   }

   //Explicitly delete copy and move assignment operators
   B& operator=(B const&) = delete;
   B& operator=(B&&) = delete;
  };
  ExpectedTypeB const expectedValueB{54321098};
  Variant<double[1234], ExpectedTypeB> v2{expectedValueB};
  EXPECT_TRUE(copyConstructorInvoked);
  EXPECT_TRUE((v2.has<ExpectedTypeB>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().i), expectedValueB.i);
  }()));
 }

 TEST(Constructor, element_copy_constructor_with_copy_assignable_type_properly_initializes_variant) {
  //TODO test that variant invokes default constructor
  //and copy assigns to element
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, element_move_constructor_with_move_constructible_type_properly_initializes_variant) {
  //Test with integral type
  using ExpectedTypeA = short;
  ExpectedTypeA const expectedValueA = 534;
  Variant<float, ExpectedTypeA> v1{(ExpectedTypeA&&)expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>()), expectedValueA);
  }()));

  //Test with move-constructible type
  //Ensure element move constructor is properly invoked
  static bool moveConstructorInvoked;
  //Reset static flag in case test is re-run
  moveConstructorInvoked = false;
  using ExpectedTypeB = struct B {
   int i;

   B(int i) :
    i(i)
   {}

   //Explicitly delete copy constructor
   B(B const&) = delete;

   B(B &&a) :
    i(a.i)
   {
    a.i = 0;
    moveConstructorInvoked = true;
   }

   //Explicitly delete copy and move assignment operators
   B& operator=(B const&) = delete;
   B& operator=(B&&) = delete;
  };
  int const expectedValueB = 473829;
  ExpectedTypeB containerB{expectedValueB};
  Variant<char, ExpectedTypeB> v2{(ExpectedTypeB&&)containerB};
  EXPECT_TRUE((moveConstructorInvoked));
  EXPECT_EQ(containerB.i, 0);
  EXPECT_TRUE((v2.has<ExpectedTypeB>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().i), expectedValueB);
  }()));
 }

 TEST(Constructor, element_move_constructor_with_move_assignable_type_properly_initializes_variant) {
  //TODO test that variant invokes default constructor
  //and move assigns to element
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, element_copy_constructor_with_trivial_type_properly_initializes_variant) {
  //TODO test that no element member functions are invoked
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, element_move_constructor_with_trivial_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, variant_copy_constructor_properly_initializes_variant) {
  using ExpectedType = int;
  constexpr ExpectedType const expectedValue = 314151617;

  //Construct first variant and ensure it was correctly initialized
  Variant<char, int> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Copy first variant into second variant
  Variant<float, int, char> v2{(decltype(v1)&)v1};

  //Ensure second variant was correctly initialized
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 TEST(Constructor, variant_move_constructor_properly_initializes_variant_and_destructs_moved_variant) {
  using ExpectedType = short;
  constexpr ExpectedType const expectedValue = 1345;

  //Construct first variant and ensure it was initialized correctly
  Variant<double, char, short> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Move first variant into second variant
  Variant<short, double, char, int> v2{(decltype(v1)&&)v1};

  //Ensure first variant was destructed
  EXPECT_FALSE((v1.has<ExpectedType>()));
  EXPECT_THROW(
   ([&] { v1.get<ExpectedType>(); }()),
   VariantTypeError
  );

  //Ensure second variant was correctly initialized
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 //TODO remove
 //Note: Replaced by tests below
 TEST(Constructor, variant_supports_array_element_type_construction) {
  //l-value reference construction
  using ExpectedTypeA = double[3];
  double expectedValueA[3]{ 3, 2, 1 };

  //Invoke Variant element copy constructor and ensure it was correctly
  //initialized
  Variant<ExpectedTypeA> v1{(ExpectedTypeA const&)expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ(
    (memcmp(v1.get<ExpectedTypeA>(), expectedValueA, ArraySize<ExpectedTypeA>)),
    0
   );
  }()));

  //r-value refernece construction
  using ExpectedTypeB = int[2];
  int expectedValueB[2]{ 542987, 3214 };

  //Invoke Variant element move constructor and ensure it was correctly
  //initialized
  Variant<ExpectedTypeB> v2{(ExpectedTypeB&&)expectedValueB};
  EXPECT_TRUE((v2.has<ExpectedTypeB>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ(
    (memcmp(v2.get<ExpectedTypeB>(), expectedValueB, ArraySize<ExpectedTypeB>)),
    0
   );
  }()));
 }

 TEST(Constructor, array_element_copy_constructor_with_copy_constructible_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, array_element_copy_constructor_with_copy_assignabe_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, array_element_move_constructor_with_move_constructible_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, array_element_move_constructor_with_move_assignable_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, array_element_of_trivial_type_copy_constructor_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Constructor, array_element_of_trivial_type_move_constructor_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 //Assignment operator tests
 TEST(Assignment, element_copy_assignment_with_copy_constructible_type_properly_initializes_variant) {
  //Test with integral type
  using ExpectedTypeA = long double;
  constexpr ExpectedTypeA expectedValueA = (ExpectedTypeA)3.1415926535897932384626433826795;
  Variant<int, ExpectedTypeA, long, bool> v1;
  v1 = (ExpectedTypeA&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>()), expectedValueA);
  }()));

  //Test with copy assignable type
  //Ensure element move assignment operator is properly invoked
  static bool copyAssignmentOperatorInvoked;
  //Reset static flag in case test is re-run
  copyAssignmentOperatorInvoked = false;
  using ExpectedTypeB =  struct B {
   char c;
   double d;

   B() {}

   B(char c, double d) :
    c(c),
    d(d)
   {}

   //Explicitly delete copy and move constructors
   B(B const&) = delete;
   B(B&&) = delete;

   B& operator=(B const& other) {
    copyAssignmentOperatorInvoked = true;
    d = other.d;
    return *this;
   }

   //Explicitly delete the move assignment operator
   B& operator=(B&&) = delete;
  };
  ExpectedTypeB const expectedValueB{127, 6.28};
  Variant<char, int, float, ExpectedTypeB> v2{(ExpectedTypeB&)expectedValueB};
  EXPECT_TRUE(copyAssignmentOperatorInvoked);
  EXPECT_TRUE((v2.has<ExpectedTypeB>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().d), expectedValueB.d);
  }()));
 }

 TEST(Assignment, element_copy_assignment_with_copy_assignable_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, element_move_assignment_with_move_constructible_type_properly_initializes_variant) {
  //Test with integral type
  using ExpectedTypeA = bool;
  constexpr ExpectedTypeA expectedValueA = false;
  Variant<float, ExpectedTypeA> v1;
  v1 = (ExpectedTypeA&&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>()), expectedValueA);
  }()));

  //Test with move-assignable type
  static bool moveAssignmentOperatorInvoked;
  //Reset static flag in case test is re-run
  moveAssignmentOperatorInvoked = false;
  using ExpectedTypeB = struct B {
   float f;

   B() {}

   B(float f) :
    f(f)
   {}

   //Explicitly delete copy and move construcors
   B(B const&) = delete;
   B(B&&) = delete;

   //Explicitly delete copy assignment operator
   B& operator=(B const&) = delete;

   B& operator=(B &&other) {
    moveAssignmentOperatorInvoked = true;
    f = other.f;
    other.f = 0;
    return *this;
   }
  };
  float const expectedValueB = 3.1415;
  ExpectedTypeB containerB{expectedValueB};
  Variant<double, ExpectedTypeB> v2{(ExpectedTypeB&&)containerB};
  EXPECT_TRUE((moveAssignmentOperatorInvoked));
  EXPECT_EQ((containerB.f), 0);
  EXPECT_TRUE((v2.has<ExpectedTypeB>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().f), expectedValueB);
  }()));
 }

 TEST(Assignment, element_move_assignment_with_move_assignable_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, element_copy_assignment_with_trivial_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, element_move_assignment_with_trivial_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, variant_copy_assignment_properly_initializes_variant) {
  using ExpectedType = long double;
  constexpr ExpectedType const expectedValue = 6.789012345678901234;

  //Construct first variant and ensure it was correctly initialized
  Variant<long long, ExpectedType, short> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Construct second variant and ensure it was correctly initialized
  Variant<ExpectedType, short, long long> v2 = (decltype(v1) const&)v1;
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 TEST(Assignment, variant_move_assignment_properly_initializes_variant_and_destructs_moved_variant) {
  using ExpectedType = char;
  constexpr ExpectedType const expectedValue = 'F';

  //Construct first variant and ensure it was correctly initialized
  Variant<bool, ExpectedType, int[1]> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Move first variant into second variant
  Variant<ExpectedType, int[1], bool> v2 = (decltype(v1)&&)v1;

  //Ensure first variant was destructed
  EXPECT_FALSE((v1.has<ExpectedType>()));
  EXPECT_THROW(
   ([&] {
    v1.get<ExpectedType>();
   }()),
   VariantTypeError
  );

  //Ensure second variant was corretly initialized
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 //TODO remove
 //Note: Replaced by tests below
 TEST(Assignment, variant_supports_array_element_type_assignment) {
  //l-value reference assignment
  using ExpectedTypeA = double[3];
  double expectedValueA[3]{ 3.14, 2.71, -1 };

  //Invoke Variant element copy assignment operator and ensure it was correctly
  //initialized
  Variant<ExpectedTypeA> v1 = (ExpectedTypeA const&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ(
    (memcmp(v1.get<ExpectedTypeA>(), expectedValueA, ArraySize<ExpectedTypeA>)),
    0
   );
  }()));

  //r-value refernece assignment
  using ExpectedTypeB = int[2];
  int expectedValueB[2]{ 297854, 239876 };

  //Invoke Variant element move assignment operator and ensure it was correctly
  //initialized
  Variant<ExpectedTypeB> v2 = (ExpectedTypeB&&)expectedValueB;
  EXPECT_TRUE((v2.has<ExpectedTypeB>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ(
    (memcmp(v2.get<ExpectedTypeB>(), expectedValueB, ArraySize<ExpectedTypeB>)),
    0
   );
  }()));
 }

 TEST(Assignment, array_element_copy_assignment_with_copy_constructible_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, array_element_copy_assignment_with_copy_assignable_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, array_element_move_assignment_with_move_constructible_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, array_element_move_assignment_with_move_assignable_type_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, array_element_assignment_of_trivial_type_copy_assignment_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Assignment, array_element_assignment_of_trivial_type_move_assignment_properly_initializes_variant) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Destructor, variant_correctly_invokes_element_type_destructor) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(Destructor, varaint_correctly_invokes_destructors_of_all_array_elements) {
  throw std::runtime_error{"Unimplemented"};
 }

 //Member function tests
 template<typename V, typename E>
 concept VariantHasInvokable = requires (V v) { v.template has<E>(); };

 template<typename V, typename E>
 concept VariantDrainInvokable = requires (V v) { v.template drain<E>(); };

 template<typename V, typename E>
 concept VariantRdrainInvokable = requires (V v, E e) { v.template rdrain<E>(e); };

 template<typename V, typename E>
 concept VariantAssignmentOperatorInvokable = requires (V v, E e) { v.template operator=<E>(e); };

 TEST(Variant, member_templates_are_invalid_for_non_element_and_incompatible_variant_types) {
  using VariantType = Variant<float, char>;
  EXPECT_FALSE((VariantHasInvokable<VariantType, void>));
  EXPECT_FALSE((VariantDrainInvokable<VariantType, int>));
  EXPECT_FALSE((VariantRdrainInvokable<VariantType, double>));

  //Element and variant assignment operators
  EXPECT_FALSE((VariantAssignmentOperatorInvokable<VariantType, long double>));
  EXPECT_FALSE((VariantAssignmentOperatorInvokable<VariantType, short&>));
  EXPECT_FALSE((VariantAssignmentOperatorInvokable<VariantType, long&&>));
  EXPECT_FALSE((VariantAssignmentOperatorInvokable<VariantType, Variant<long, short>&>));
  EXPECT_FALSE((VariantAssignmentOperatorInvokable<VariantType, Variant<bool, int, double>&&>));
 }

 TEST(Variant, get_returns_reference_to_stored_element) {
  using ExpectedType = char[123];
  ExpectedType const expectedValue = "Goodbye world!";

  //Construct variant and ensure it was correctly initialized
  ExpectedType const value = "Hello world!";
  Variant<double, bool, ExpectedType, float> v{value};
  EXPECT_TRUE((v.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_STREQ((v.get<ExpectedType>()), value);
  }()));

  //Ensure returned reference is correct
  EXPECT_NO_THROW(([&] {
   auto &ref = v.get<ExpectedType>();
   memcpy(&ref, expectedValue, strlen(expectedValue));
   EXPECT_STREQ(ref, expectedValue);
  }()));
 }

 TEST(Variant, drain_returns_copy_of_stored_element_and_destructs_variant) {
  static bool copyConstructorInvoked;
  using ExpectedType = struct A {
   int i;

   A(int i) :
    i(i)
   {}

   A(A const &a) :
    i(a.i)
   {
    copyConstructorInvoked = true;
   }

   //Explicitly delete move constructor and assignment operator
   A(A&&) = delete;
   A& operator=(A&&) = delete;
  };
  int const expectedValue = 9876543;

  //Construct variant and ensure it was correctly initialized
  Variant<ExpectedType, char, float, bool> v{A{expectedValue}};
  EXPECT_TRUE((v.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v.get<ExpectedType>().i), expectedValue);
  }()));

  //Reset flag in case test is re-run
  copyConstructorInvoked = false;

  //Drain varaint and ensure it was destructed
  EXPECT_NO_THROW(([&] {
   auto value = v.drain<ExpectedType>();

   //Ensure returned value matches the expected value
   EXPECT_TRUE(copyConstructorInvoked);
   EXPECT_EQ(value.i, expectedValue);

   //Ensure the variant was destructed
   EXPECT_FALSE((v.has<ExpectedType>()));
  }()));
 }

 TEST(Variant, rdrain_moves_stored_element_into_argument_and_destructs_variant) {
  static bool moveAssignmentOperatorInvoked;
  using ExpectedType = struct A {
   float f;

   A(float f = 1231324) :
    f(f)
   {}

   //Explicitly delete copy constructor and assignment operator
   A(A const&) = delete;
   A& operator=(A const&) = delete;

   A& operator=(A &&a) {
    moveAssignmentOperatorInvoked = true;
    f = a.f;
    return *this;
   }
  };
  float const expectedValue = 2.718281828459;

  //Construct variant and ensure it was correctly initialized
  Variant<double, short, char, void *, ExpectedType> v{A{expectedValue}};
  EXPECT_TRUE((v.has<ExpectedType>()));
  EXPECT_NO_THROW(([&] {
   EXPECT_EQ((v.get<ExpectedType>().f), expectedValue);
  }()));

  //Reset flag in case test is re-run
  moveAssignmentOperatorInvoked = false;

  //Drain variant and ensure it was destructed
  EXPECT_NO_THROW(([&] {
   A value;
   v.rdrain(value);

   //Ensure value was correctly initialized
   EXPECT_TRUE((moveAssignmentOperatorInvoked));
   EXPECT_EQ(value.f, expectedValue);

   //Ensure variant was destructed
   EXPECT_FALSE((v.has<ExpectedType>()));
  }()));
 }

 //Tests for the empty variant specialization
 TEST(EmptyVariant, has_returns_false_for_all_element_types) {
  Variant<char[124], int, double> empty;
  EXPECT_FALSE(empty.has<char[124]>());
  EXPECT_FALSE(empty.has<int>());
  EXPECT_FALSE(empty.has<double>());
 }

 TEST(EmptyVariant, member_templates_are_invalid_for_all_types_except_empty_variant) {
  using VariantType = Variant<>;

  //Member function templates
  EXPECT_FALSE((VariantDrainInvokable<VariantType, int>));
  EXPECT_FALSE((VariantRdrainInvokable<VariantType, float>));

  //Element assignment operators
  EXPECT_FALSE((VariantAssignmentOperatorInvokable<VariantType, short>));
  EXPECT_FALSE((VariantAssignmentOperatorInvokable<VariantType, long&>));
  EXPECT_FALSE((VariantAssignmentOperatorInvokable<VariantType, double&&>));

  //Variant assignment operators
  EXPECT_TRUE((VariantAssignmentOperatorInvokable<VariantType, VariantType const&>));
  EXPECT_TRUE((VariantAssignmentOperatorInvokable<VariantType, VariantType&&>));
 }

 TEST(EmptyVariant, constructor_templates_are_invalid_for_all_types_except_empty_variant) {
  using VariantType = Variant<>;

  //Default constructor
  EXPECT_TRUE((Constructible<VariantType>));

  //Element constructors
  EXPECT_FALSE((Constructible<VariantType, char>));
  EXPECT_FALSE((Constructible<VariantType, int const&>));
  EXPECT_FALSE((Constructible<VariantType, float&&>));

  //Variant constructors
  EXPECT_TRUE((Constructible<VariantType, VariantType const&>));
  EXPECT_TRUE((Constructible<VariantType, VariantType&&>));
 }
}
