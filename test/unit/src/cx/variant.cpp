#include <cx/test/common/common.h>

#include <cx/variant.h>

namespace CX::Testing {
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
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
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
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().i), expectedValueB.i);
  }()));
 }

 TEST(Constructor, element_copy_constructor_with_copy_assignable_type_properly_initializes_variant) {
  //Test with copy assignable type
  static bool
   copyAssignmentOperatorInvoked,
   defaultConstructorInvoked;
  //Reset flags in case test is re-run
  copyAssignmentOperatorInvoked = false;
  defaultConstructorInvoked = false;
  using ExpectedTypeA = struct A {
   int i;

   A() :
    i(789)
   {
    defaultConstructorInvoked = true;
   }

   A(int i) :
    i(i)
   {}

   A& operator=(A const &a) {
    copyAssignmentOperatorInvoked = true;
    i = a.i;
    return *this;
   }

   //Explicitly delete unused copy/move constructors/operators
   A(A const &a) = delete;
   A(A&&) = delete;
   A& operator=(A&&) = delete;
  };
  A const expectedValueA{123};

  //Construct variant and ensure it was correctly initialized
  Variant<char8_t, ExpectedTypeA> v1{(ExpectedTypeA const&)expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>().i), expectedValueA.i);
  }()));

  //Ensure element default constructor and copy assignment operator
  //were invoked
  EXPECT_TRUE((copyAssignmentOperatorInvoked));
  EXPECT_TRUE((defaultConstructorInvoked));
 }

 TEST(Constructor, element_move_constructor_with_move_constructible_type_properly_initializes_variant) {
  //Test with integral type
  using ExpectedTypeA = short;
  ExpectedTypeA const expectedValueA = 534;
  Variant<float, ExpectedTypeA> v1{(ExpectedTypeA&&)expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
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
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().i), expectedValueB);
  }()));
 }

 TEST(Constructor, element_move_constructor_with_move_assignable_type_properly_initializes_variant) {
  //Test with move assignable type
  static bool
   moveAssignmentOperatorInvoked,
   defaultConstructorInvoked;
  //Reset flags in case test is re-run
  moveAssignmentOperatorInvoked = false;
  defaultConstructorInvoked = false;
  using ExpectedTypeA = struct A {
   float f;

   A() :
    f(3.141)
   {
    defaultConstructorInvoked = true;
   }

   A(float f) :
    f(f)
   {}

   A& operator=(A &&a) {
    moveAssignmentOperatorInvoked = true;
    f = a.f;
    a.f = 0;
    return *this;
   }

   //Explicitly delete unused constructors / assignment operators
   A(A const&) = delete;
   A(A&&) = delete;
   A& operator=(A const&) = delete;
  };
  float const expectedValueA = 2.71;
  A temp{expectedValueA};

  //Construct variant and ensure it was correctly initialized
  Variant<void *, char, ExpectedTypeA> v1{(ExpectedTypeA&&)temp};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>().f), expectedValueA);
  }()));

  //Ensure moved temporary has been reset
  EXPECT_EQ(temp.f, 0);

  //Ensure default constructor and move assignment operator
  //were invoked
  EXPECT_TRUE((defaultConstructorInvoked));
  EXPECT_TRUE((moveAssignmentOperatorInvoked));
 }

 TEST(Constructor, element_copy_constructor_with_trivial_type_properly_initializes_variant) {
  using ExpectedTypeA = struct A {
   struct {
    char c;
   } b;
  };
  ExpectedTypeA expectedValueA{{'f'}};

  //Ensure `ExpectedTypeA` is trivial
  EXPECT_TRUE((Trivial<ExpectedTypeA>));

  //Construct variant and ensure it was correctly initialized
  Variant<ExpectedTypeA, int (Dummy<>::*)> v1{(ExpectedTypeA const&)expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>().b.c), expectedValueA.b.c);
  }()));
 }

 TEST(Constructor, element_move_constructor_with_trivial_type_properly_initializes_variant) {
  using ExpectedTypeA = struct A {
   float data[123];
  };
  float expectedValueA[123]{9.87654};
  ExpectedTypeA temp{{9.87654}};

  //Ensure `ExpectedTypeA` is trivial
  EXPECT_TRUE((Trivial<ExpectedTypeA>));

  //Construct variant and ensure it was correctly initialized
  Variant<double, ExpectedTypeA, int> v1{(ExpectedTypeA&&)temp};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>().data, &expectedValueA, sizeof(float) * 123)),
    0
   );
  }()));
 }

 TEST(Constructor, variant_copy_constructor_properly_initializes_variant) {
  using ExpectedType = int;
  constexpr ExpectedType const expectedValue = 314151617;

  //Construct first variant and ensure it was correctly initialized
  Variant<char, int> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Copy first variant into second variant
  Variant<float, int, char> v2{(decltype(v1)&)v1};

  //Ensure second variant was correctly initialized
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 TEST(Constructor, variant_move_constructor_properly_initializes_variant_and_destructs_moved_variant) {
  using ExpectedType = short;
  constexpr ExpectedType const expectedValue = 1345;

  //Construct first variant and ensure it was initialized correctly
  Variant<double, char, short> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Move first variant into second variant
  Variant<short, double, char, int> v2{(decltype(v1)&&)v1};

  //Ensure first variant was destructed
  EXPECT_FALSE((v1.has<ExpectedType>()));
  EXPECT_ERROR_BEHAVIOUR(
   ([&] { v1.get<ExpectedType>(); }()),
   CXError 
  );

  //Ensure second variant was correctly initialized
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 TEST(Constructor, array_element_copy_constructor_with_copy_constructible_type_properly_initializes_variant) {
  static int copyConstructorInvoked;
  //Reset counter in case test is re-run
  copyConstructorInvoked = 0;
  using ExpectedTypeA = struct A {
   double d;

   A(double d) :
    d(d)
   {}

   A(A const &a) :
    d(a.d)
   {
    copyConstructorInvoked++;
   }

   //Delete unused constructors / assignment operators
   A(A&&) = delete;
   A& operator=(A const&) = delete;
   A& operator=(A&&) = delete;
  }[2];
  ExpectedTypeA const expectedValueA{
   {3.21},
   {1.23}
  };

  //Construct variant and ensure it was correctly initialized
  Variant<ExpectedTypeA, char, void (Dummy<>::*)()> v1{(ExpectedTypeA const&)expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>(), &expectedValueA, sizeof(ArrayElementType<ExpectedTypeA>) * 2)),
    0
   );
  }()));

  //Ensure all array elements were copy constructed
  EXPECT_EQ(copyConstructorInvoked, 2);
 }

 TEST(Constructor, array_element_copy_constructor_with_copy_assignabe_type_properly_initializes_variant) {
  static int
   defaultConstructorInvoked,
   copyAssignmentOperatorInvoked;
  //Reset counters in case test is re-run
  defaultConstructorInvoked = 0;
  copyAssignmentOperatorInvoked = 0;
  using ExpectedTypeA = struct A {
   int i;

   A(int i) :
    i(i)
   {}

   A() :
    i(0)
   {
    defaultConstructorInvoked++;
   }

   A& operator=(A const &a) {
    copyAssignmentOperatorInvoked++;
    i = a.i;
    return *this;
   }

   //Delete unused constructors / assignment operators
   A(A const&) = delete;
   A(A&&) = delete;
   A& operator=(A&&) = delete;
  }[3];
  ExpectedTypeA const expectedValueA{
   {7},
   {8},
   {9}
  };

  //Construct variant and ensure it was correctly initialized
  Variant<float[12], ExpectedTypeA> v1{(ExpectedTypeA const&)expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>(), &expectedValueA, sizeof(ArrayElementType<ExpectedTypeA>) * 3)),
    0
   );
  }()));

  //Ensure element default constructors and copy assignment operators
  //were invoked
  EXPECT_EQ(defaultConstructorInvoked, 3);
  EXPECT_EQ(copyAssignmentOperatorInvoked, 3);
 }

 TEST(Constructor, array_element_move_constructor_with_move_constructible_type_properly_initializes_variant) {
  static int moveConstructorsInvoked;
  //Reset counter in case test is re-run
  moveConstructorsInvoked = 0;
  using ExpectedTypeA = struct A {
   int i;

   A(int i) :
    i(i)
   {}

   A(A &&a) :
    i(a.i)
   {
    moveConstructorsInvoked++;
    a.i = 0;
   }

   //Delete unused constructors and assignment operators
   A(A const&) = delete;
   A& operator=(A const&) = delete;
   A& operator=(A&&) = delete;
  }[5];
  ExpectedTypeA const expectedValueA{
   {1}, {2}, {3}, {4}, {5}
  };
  ExpectedTypeA temp{
   {1}, {2}, {3}, {4}, {5}
  };

  //Construct variant and ensure it was correctly initialized
  Variant<int (Dummy<>::*), ExpectedTypeA> v1{(ExpectedTypeA&&)temp};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>(), &expectedValueA, sizeof(ArrayElementType<ExpectedTypeA>) * 5)),
    0
   );
  }()));

  //Ensure temporary elements were moved
  for (auto &t : temp) {
   EXPECT_EQ(t.i, 0);
  }

  //Ensure all move constructors were invoked
  EXPECT_EQ(moveConstructorsInvoked, 5);
 }

 TEST(Constructor, array_element_move_constructor_with_move_assignable_type_properly_initializes_variant) {
  static int
   defaultConstructorsInvoked,
   moveAssignmentOperatorsInvoked;
  //Reset counters in case test is re-run
  defaultConstructorsInvoked = 0;
  moveAssignmentOperatorsInvoked = 0;
  using ExpectedTypeA = struct A {
   char c;

   A(char c) :
    c(c)
   {}

   A() :
    c(0)
   {
    defaultConstructorsInvoked++;
   }

   A& operator=(A &&a) {
    moveAssignmentOperatorsInvoked++;
    c = a.c;
    a.c = 0;
    return *this;
   }

   //Delete unused constructors and assignment operators
   A(A const&) = delete;
   A(A&&) = delete;
   A& operator=(A const &) = delete;
  }[2];
  ExpectedTypeA const expectedValueA{
   {'a'}, {'b'}
  };
  ExpectedTypeA temp{
   {'a'}, {'b'}
  };

  //Construct variant and ensure it was correctly initialized
  Variant<ExpectedTypeA> v1{(ExpectedTypeA&&)temp};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>(), &expectedValueA, sizeof(ArrayElementType<ExpectedTypeA>) * 2)),
    0
   );
  }()));

  //Ensure temporary elements were moved
  for (auto &e : temp) {
   EXPECT_EQ(e.c, 0);
  }

  //Ensure default constructors and move assignment operators
  //were invoked
  EXPECT_EQ(defaultConstructorsInvoked, 2);
  EXPECT_EQ(moveAssignmentOperatorsInvoked, 2);
 }

 TEST(Constructor, array_element_of_trivial_type_copy_constructor_properly_initializes_variant) {
  using ExpectedTypeA = struct A {
   float f;
  };
  ExpectedTypeA const expectedValueA{4.567};

  //Construct variant and ensure it was correctly initialized
  Variant<ExpectedTypeA, float> v1{(ExpectedTypeA const&)expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>().f), expectedValueA.f);
  }()));
 }

 TEST(Constructor, array_element_of_trivial_type_move_constructor_properly_initializes_variant) {
  using ExpectedTypeA = struct A {
   long double d;
  };
  ExpectedTypeA expectedValueA{823746.187624};

  //Construct variant and ensure it was correctly initialized
  Variant<ExpectedTypeA, float> v1{(ExpectedTypeA&&)expectedValueA};
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>().d), expectedValueA.d);
  }()));
 }

 //Assignment operator tests
 TEST(Assignment, element_copy_assignment_with_copy_constructible_type_properly_initializes_variant) {
  //Test with integral type
  using ExpectedTypeA = long double;
  constexpr ExpectedTypeA expectedValueA = (ExpectedTypeA)3.1415926535897932384626433826795;
  Variant<int, ExpectedTypeA, long, bool> v1;
  v1 = (ExpectedTypeA&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
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
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().d), expectedValueB.d);
  }()));
 }

 TEST(Assignment, element_copy_assignment_with_copy_assignable_type_properly_initializes_variant) {
  //Test with copy assignable type
  static bool
   copyAssignmentOperatorInvoked,
   defaultConstructorInvoked;
  //Reset flags in case test is re-run
  copyAssignmentOperatorInvoked = false;
  defaultConstructorInvoked = false;
  using ExpectedTypeA = struct A {
   char32_t c;

   A() :
    c(1927836)
   {
    defaultConstructorInvoked = true;
   }

   A(char32_t c) :
    c(c)
   {}

   A& operator=(A const &a) {
    copyAssignmentOperatorInvoked = true;
    c = a.c;
    return *this;
   }

   //Explicitly delete unused copy/move constructors/operators
   A(A const &a) = delete;
   A(A&&) = delete;
   A& operator=(A&&) = delete;
  };
  A const expectedValueA{12314};

  //Construct variant and ensure it was correctly initialized
  Variant<char8_t, ExpectedTypeA> v1;
  v1 = (ExpectedTypeA const&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   //Note can't use `EXPECT_EQ` since gtest does not support
   //extended char types in its printer specializations
   EXPECT_TRUE((v1.get<ExpectedTypeA>().c == expectedValueA.c));
  }()));

  //Ensure element default constructor and copy assignment operator
  //were invoked
  EXPECT_TRUE((copyAssignmentOperatorInvoked));
  EXPECT_TRUE((defaultConstructorInvoked));
 }

 TEST(Assignment, element_move_assignment_with_move_constructible_type_properly_initializes_variant) {
  //Test with integral type
  using ExpectedTypeA = bool;
  constexpr ExpectedTypeA expectedValueA = false;
  Variant<float, ExpectedTypeA> v1;
  v1 = (ExpectedTypeA&&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
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
  Variant<double, ExpectedTypeB> v2;
  v2 = (ExpectedTypeB&&)containerB;
  EXPECT_TRUE((moveAssignmentOperatorInvoked));
  EXPECT_EQ((containerB.f), 0);
  EXPECT_TRUE((v2.has<ExpectedTypeB>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v2.get<ExpectedTypeB>().f), expectedValueB);
  }()));
 }

 TEST(Assignment, element_move_assignment_with_move_assignable_type_properly_initializes_variant) {
  //Test with move assignable type
  static bool
   moveAssignmentOperatorInvoked,
   defaultConstructorInvoked;
  //Reset flags in case test is re-run
  moveAssignmentOperatorInvoked = false;
  defaultConstructorInvoked = false;
  using ExpectedTypeA = struct A {
   short s;

   A() :
    s(132)
   {
    defaultConstructorInvoked = true;
   }

   A(short s) :
    s(s)
   {}

   A& operator=(A &&a) {
    moveAssignmentOperatorInvoked = true;
    s = a.s;
    a.s = 0;
    return *this;
   }

   //Explicitly delete unused constructors / assignment operators
   A(A const&) = delete;
   A(A&&) = delete;
   A& operator=(A const&) = delete;
  };
  short const expectedValueA = 23;
  A temp{expectedValueA};

  //Construct variant and assign to it; ensure it was correctly initialized
  Variant<double[123], ExpectedTypeA> v1;
  v1 = (ExpectedTypeA&&)temp;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>().s), expectedValueA);
  }()));

  //Ensure moved temporary has been reset
  EXPECT_EQ(temp.s, 0);

  //Ensure default constructor and move assignment operator
  //were invoked
  EXPECT_TRUE((defaultConstructorInvoked));
  EXPECT_TRUE((moveAssignmentOperatorInvoked));
 }

 TEST(Assignment, element_copy_assignment_with_trivial_type_properly_initializes_variant) {
  using ExpectedTypeA = struct A {
   char16_t c;
  };
  ExpectedTypeA expectedValueA{'f'};

  //Ensure `ExpectedTypeA` is trivial
  EXPECT_TRUE((Trivial<ExpectedTypeA>));

  //Construct variant and assign to it; ensure it was correctly initialized
  Variant<ExpectedTypeA, int (Dummy<>::*)> v1;
  v1 = (ExpectedTypeA const&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_TRUE((v1.get<ExpectedTypeA>().c == expectedValueA.c));
  }()));
 }

 TEST(Assignment, element_move_assignment_with_trivial_type_properly_initializes_variant) {
  using ExpectedTypeA = struct A {
   long double data[123];
  };
  long double expectedValueA[123]{1545364356.2342345};
  ExpectedTypeA temp{{1545364356.2342345}};

  //Ensure `ExpectedTypeA` is trivial
  EXPECT_TRUE((Trivial<ExpectedTypeA>));

  //Construct variant and assign to it; ensure it was correctly initialized
  Variant<double, ExpectedTypeA, int> v1;
  v1 = (ExpectedTypeA&&)temp;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>().data, &expectedValueA, sizeof(long double) * 123)),
    0
   );
  }()));
 }

 TEST(Assignment, variant_copy_assignment_properly_initializes_variant) {
  using ExpectedType = long double;
  constexpr ExpectedType const expectedValue = 6.789012345678901234;

  //Construct first variant and ensure it was correctly initialized
  Variant<long long, ExpectedType, short> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Construct second variant and ensure it was correctly initialized
  Variant<ExpectedType, short, long long> v2;
  v2 = (decltype(v1) const&)v1;
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 TEST(Assignment, variant_move_assignment_properly_initializes_variant_and_destructs_moved_variant) {
  using ExpectedType = char;
  constexpr ExpectedType const expectedValue = 'F';

  //Construct first variant and ensure it was correctly initialized
  Variant<bool, ExpectedType, int[1]> v1{expectedValue};
  EXPECT_TRUE((v1.has<ExpectedType>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedType>()), expectedValue);
  }()));

  //Move first variant into second variant
  Variant<ExpectedType, int[1], bool> v2 = (decltype(v1)&&)v1;

  //Ensure first variant was destructed
  EXPECT_FALSE((v1.has<ExpectedType>()));
  EXPECT_ERROR_BEHAVIOUR(
   ([&] {
    v1.get<ExpectedType>();
   }()),
   CXError
  );

  //Ensure second variant was corretly initialized
  EXPECT_TRUE((v2.has<ExpectedType>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v2.get<ExpectedType>()), expectedValue);
  }()));
 }

 TEST(Assignment, array_element_copy_assignment_with_copy_constructible_type_properly_initializes_variant) {
  static int copyConstructorInvoked;
  //Reset counter in case test is re-run
  copyConstructorInvoked = 0;
  using ExpectedTypeA = struct A {
   bool b;

   A(bool b) :
    b(b)
   {}

   A(A const &a) :
    b(a.b)
   {
    copyConstructorInvoked++;
   }

   //Delete unused constructors / assignment operators
   A(A&&) = delete;
   A& operator=(A const&) = delete;
   A& operator=(A&&) = delete;
  }[2];
  ExpectedTypeA const expectedValueA{
   {true},
   {false}
  };

  //Construct variant and assign to it; ensure it was correctly initialized
  Variant<ExpectedTypeA, char, void (Dummy<>::*)()> v1;
  v1 = (ExpectedTypeA const&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>(), &expectedValueA, sizeof(ArrayElementType<ExpectedTypeA>) * 2)),
    0
   );
  }()));

  //Ensure all array elements were copy constructed
  EXPECT_EQ(copyConstructorInvoked, 2);
 }

 TEST(Assignment, array_element_copy_assignment_with_copy_assignable_type_properly_initializes_variant) {
  static int
   defaultConstructorInvoked,
   copyAssignmentOperatorInvoked;
  //Reset counters in case test is re-run
  defaultConstructorInvoked = 0;
  copyAssignmentOperatorInvoked = 0;
  using ExpectedTypeA = struct A {
   int i;

   A(int i) :
    i(i)
   {}

   A() :
    i(0)
   {
    defaultConstructorInvoked++;
   }

   A& operator=(A const &a) {
    copyAssignmentOperatorInvoked++;
    i = a.i;
    return *this;
   }

   //Delete unused constructors / assignment operators
   A(A const&) = delete;
   A(A&&) = delete;
   A& operator=(A&&) = delete;
  }[3];
  ExpectedTypeA const expectedValueA{
   {324987},
   {38},
   {547879}
  };

  //Construct variant and assign to it; ensure it was correctly initialized
  Variant<float[12], ExpectedTypeA> v1;
  v1 = (ExpectedTypeA const&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>(), &expectedValueA, sizeof(ArrayElementType<ExpectedTypeA>) * 3)),
    0
   );
  }()));

  //Ensure element default constructors and copy assignment operators
  //were invoked
  EXPECT_EQ(defaultConstructorInvoked, 3);
  EXPECT_EQ(copyAssignmentOperatorInvoked, 3);
 }

 TEST(Assignment, array_element_move_assignment_with_move_constructible_type_properly_initializes_variant) {
  static int moveConstructorsInvoked;
  //Reset counter in case test is re-run
  moveConstructorsInvoked = 0;
  using ExpectedTypeA = struct A {
   int i;

   A(int i) :
    i(i)
   {}

   A(A &&a) :
    i(a.i)
   {
    moveConstructorsInvoked++;
    a.i = 0;
   }

   //Delete unused constructors and assignment operators
   A(A const&) = delete;
   A& operator=(A const&) = delete;
   A& operator=(A&&) = delete;
  }[5];
  ExpectedTypeA const expectedValueA{
   {1349875}, {2342}, {3}, {458}, {25}
  };
  ExpectedTypeA temp{
   {1349875}, {2342}, {3}, {458}, {25}
  };

  //Construct variant and assign to it; ensure it was correctly initialized
  Variant<int (Dummy<>::*), ExpectedTypeA> v1;
  v1 = (ExpectedTypeA&&)temp;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>(), &expectedValueA, sizeof(ArrayElementType<ExpectedTypeA>) * 5)),
    0
   );
  }()));

  //Ensure temporary elements were moved
  for (auto &t : temp) {
   EXPECT_EQ(t.i, 0);
  }

  //Ensure all move constructors were invoked
  EXPECT_EQ(moveConstructorsInvoked, 5);
 }

 TEST(Assignment, array_element_move_assignment_with_move_assignable_type_properly_initializes_variant) {
  static int
   defaultConstructorsInvoked,
   moveAssignmentOperatorsInvoked;
  //Reset counters in case test is re-run
  defaultConstructorsInvoked = 0;
  moveAssignmentOperatorsInvoked = 0;
  using ExpectedTypeA = struct A {
   wchar_t c;

   A(wchar_t c) :
    c(c)
   {}

   A() :
    c(0)
   {
    defaultConstructorsInvoked++;
   }

   A& operator=(A &&a) {
    moveAssignmentOperatorsInvoked++;
    c = a.c;
    a.c = 0;
    return *this;
   }

   //Delete unused constructors and assignment operators
   A(A const&) = delete;
   A(A&&) = delete;
   A& operator=(A const &) = delete;
  }[2];
  ExpectedTypeA const expectedValueA{
   {'H'}, {'Q'}
  };
  ExpectedTypeA temp{
   {'H'}, {'Q'}
  };

  //Construct variant and assign to it; ensure it was correctly initialized
  Variant<ExpectedTypeA> v1;
  v1 = (ExpectedTypeA&&)temp;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ(
    (memcmp(&v1.get<ExpectedTypeA>(), &expectedValueA, sizeof(ArrayElementType<ExpectedTypeA>) * 2)),
    0
   );
  }()));

  //Ensure temporary elements were moved
  for (auto &e : temp) {
   EXPECT_EQ(e.c, 0);
  }

  //Ensure default constructors and move assignment operators
  //were invoked
  EXPECT_EQ(defaultConstructorsInvoked, 2);
  EXPECT_EQ(moveAssignmentOperatorsInvoked, 2);

 }

 TEST(Assignment, array_element_assignment_of_trivial_type_copy_assignment_properly_initializes_variant) {
  using ExpectedTypeA = struct A {
   float f;
  };
  ExpectedTypeA const expectedValueA{283764.3};

  //Construct variant and assign to it; ensure it was correctly initialized
  Variant<ExpectedTypeA, float> v1;
  v1 = (ExpectedTypeA const&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>().f), expectedValueA.f);
  }()));
 }

 TEST(Assignment, array_element_assignment_of_trivial_type_move_assignment_properly_initializes_variant) {
  using ExpectedTypeA = struct A {
   long double d;
  };
  ExpectedTypeA expectedValueA{1.2345678901};

  //Construct variant and assign to it; ensure it was correctly initialized
  Variant<ExpectedTypeA, float> v1;
  v1 = (ExpectedTypeA&&)expectedValueA;
  EXPECT_TRUE((v1.has<ExpectedTypeA>()));
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v1.get<ExpectedTypeA>().d), expectedValueA.d);
  }()));
 }

 TEST(Destructor, variant_correctly_invokes_element_type_destructor) {
  static bool destructorInvoked;
  //Reset flag in case test is re-run
  destructorInvoked = false;
  using ExpectedTypeA = struct A {
   ~A() {
    destructorInvoked = true;
   }
  };
  ExpectedTypeA const expectedValueA{};

  //Construct variant
  [&] {
   Variant<ExpectedTypeA> v{(ExpectedTypeA const&)expectedValueA};
  }();

  //Ensure variant element type was destructed along with variant
  EXPECT_TRUE(destructorInvoked);
 }

 TEST(Destructor, varaint_correctly_invokes_destructors_of_all_array_elements) {
  static int destructorsInvoked;
  //Reset counter in case test is re-run
  destructorsInvoked = 0;
  using ExpectedTypeA = struct A {
   ~A() {
    destructorsInvoked++;
   }
  }[4];
  ExpectedTypeA const expectedValueA{
   {}, {}, {}, {}
  };

  //Construct variant
  [&] {
   Variant<ExpectedTypeA> v{(ExpectedTypeA const&)expectedValueA};
  }();

  //Ensure all array elements were destructed along with variant
  EXPECT_EQ(destructorsInvoked, 4);
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
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_STREQ((v.get<ExpectedType>()), value);
  }()));

  //Ensure returned reference is correct
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
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
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v.get<ExpectedType>().i), expectedValue);
  }()));

  //Reset flag in case test is re-run
  copyConstructorInvoked = false;

  //Drain varaint and ensure it was destructed
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
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
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
   EXPECT_EQ((v.get<ExpectedType>().f), expectedValue);
  }()));

  //Reset flag in case test is re-run
  moveAssignmentOperatorInvoked = false;

  //Drain variant and ensure it was destructed
  EXPECT_NO_ERROR_BEHAVIOUR(([&] {
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
