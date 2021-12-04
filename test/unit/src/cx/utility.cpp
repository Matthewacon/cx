#include <cx/test/common/common.h>
#include <cx/utility.h>

namespace CX {
 TEST(newInPlace, placement_new_initializes_trivial_type) {
  constexpr auto const body = []() constexpr noexcept {
   union {
    int i;
   } u;

   int const expected = 1234;
   auto r = newInPlace<int, int const&>(u.i, expected);
   CX_GTEST_SHIM(EXPECT_TRUE, (r == expected));
   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 TEST(newInPlace, placement_new_initializes_non_trivial_type) {
  constexpr auto const body = []() constexpr {
   struct A : CopyConstructibleType {
    int data[23];

    constexpr A(int i) noexcept :
     data{0}
    {
     data[i] = i;
    }

    constexpr A(A const&) noexcept = default;

    constexpr virtual ~A() noexcept {}
   };

   union U {
    A a;
    constexpr U() noexcept {}
    constexpr ~U() noexcept {}
   } u;

   int const i = 13;
   A const expected{i};
   auto& result = newInPlace<A, A const&>(u.a, copy(expected));
   CX_GTEST_SHIM(EXPECT_TRUE, (result.data[i] == expected.data[i]));
   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 //Utility type for concept tests
 template<typename Base>
 struct DefaultConstructibleCrtp : Base {
  using Base::Base;

  constexpr DefaultConstructibleCrtp() noexcept {}
 };

 //Non-literal utility type (implicitly deleted copy/move
 //constructors/operators, default constructor and default destructor)
 union NonLiteralUnion {
  struct A {
   constexpr virtual ~A() noexcept {}
  } a;
 };

 TEST(IsCopyable, copyable_types_satisfy_constraint) {
  //Assignable type
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsCopyable<CustomType<false, false, true, false, int>>)
  );

  //Constructible type
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsCopyable<CustomType<true, false, false, false>>)
  );

  //Default constructible and assignable type
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsCopyable<
    DefaultConstructibleCrtp<CustomType<false, false, true, false, int>>
   >)
  );
 }

 TEST(IsCopyable, non_copyable_types_do_not_satisfy_constraint) {
  //Non-copyable type
  CX_GTEST_SHIM(
   EXPECT_FALSE,
   (IsCopyable<CustomType<false, false, false, false>>)
  );

  //Non-literal type
  CX_GTEST_SHIM(
   EXPECT_FALSE,
   IsCopyable<NonLiteralUnion>
  );
 }

 TEST(IsMovable, movable_types_satisfy_constraint) {
  //Assignable type
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsMovable<CustomType<false, false, false, true, int>>)
  );

  //Constructible type
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsMovable<CustomType<false, true, false, false>>)
  );

  //Default constructible and assignable type
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsMovable<
    DefaultConstructibleCrtp<CustomType<false, false, false, true, int>>
   >)
  );
 }

 TEST(IsMovable, non_movable_types_do_not_satisfy_constraint) {
  //Non-movable type
  CX_GTEST_SHIM(
   EXPECT_FALSE,
   (IsMovable<CustomType<false, false, false, false>>)
  );

  //Non-literal type
  CX_GTEST_SHIM(
   EXPECT_FALSE,
   IsMovable<NonLiteralUnion>
  );
 }

 TEST(IsCopyableOrMovable, copyable_or_movable_types_satisfy_constraint) {
  //Assignable types
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsCopyableOrMovable<CustomType<false, false, true, false, int>>)
  );
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsCopyableOrMovable<CustomType<false, false, false, true, int>>)
  );

  //Constructible types
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsCopyableOrMovable<CustomType<true, false, false, false>>)
  );
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsCopyableOrMovable<CustomType<false, true, false, false>>)
  );

  //Default constructible and assignable types
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsCopyableOrMovable<
    DefaultConstructibleCrtp<CustomType<false, false, true, false, int>>
   >)
  );
  CX_GTEST_SHIM(
   EXPECT_TRUE,
   (IsCopyableOrMovable<
    DefaultConstructibleCrtp<CustomType<false, false, false, true, int>>
   >)
  );
 }

 TEST(IsCopyableOrMovable, non_copyable_and_non_movable_types_do_not_satisfy_constraint) {
  //Non-copyable and non-movable type
  CX_GTEST_SHIM(
   EXPECT_FALSE,
   (IsCopyableOrMovable<CustomType<false, false, false, false>>)
  );

  //Non-literal type
  CX_GTEST_SHIM(
   EXPECT_FALSE,
   IsCopyableOrMovable<NonLiteralUnion>
  );
 }

 TEST(copyOrMove, non_present_constructible_type_is_constructed) {
  constexpr auto body = []() constexpr noexcept {
   using Type = CustomType<true, true, false, false>;

   union U {
    Type t;

    constexpr U() noexcept {}
    constexpr ~U() noexcept {}
   } u;

   //Test copy-construction
   Type toCopy;
   auto op = copyOrMove(u.t, copy(toCopy), false);

   CX_GTEST_SHIM(EXPECT_TRUE, op == CopyOrMoveOperation::CONSTRUCT);
   CX_GTEST_SHIM(EXPECT_TRUE, u.t.copyConstructed);

   //Clean up resources between tests
   u.t.~Type();

   //Test move-construction
   Type toMove;
   op = copyOrMove(u.t, move(toMove), false);

   CX_GTEST_SHIM(EXPECT_TRUE, op == CopyOrMoveOperation::CONSTRUCT);
   CX_GTEST_SHIM(EXPECT_TRUE, u.t.moveConstructed);

   //Clean up resources after test
   u.t.~Type();

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 TEST(copyOrMove, present_constructible_type_is_destructed_and_constructed) {
  constexpr auto const body = []() constexpr noexcept {
   struct A {
    bool& destructed;
    bool copyConstructed, moveConstructed;

    constexpr A(bool& destructed) noexcept :
     destructed{destructed},
     copyConstructed{false},
     moveConstructed{false}
    {}

    constexpr A(A const& other) noexcept :
     destructed{other.destructed},
     copyConstructed{true},
     moveConstructed{false}
    {}

    constexpr A(A&& other) noexcept :
     destructed{other.destructed},
     copyConstructed{false},
     moveConstructed{true}
    {}

    constexpr ~A() noexcept {
     destructed = true;
    }
   };

   union U {
    A a;

    constexpr U() noexcept {}
    constexpr ~U() noexcept {}
   } u;

   bool destructed = false;

   //Test destruction and copy-construction
   A toCopy{destructed};
   newInPlace<A, A const&>(u.a, copy(toCopy));
   auto op = copyOrMove(u.a, copy(toCopy), true);

   CX_GTEST_SHIM(EXPECT_TRUE, op == CopyOrMoveOperation::CONSTRUCT);
   CX_GTEST_SHIM(EXPECT_TRUE, destructed);
   CX_GTEST_SHIM(EXPECT_TRUE, u.a.copyConstructed);

   //Clean up resources between tests
   u.a.~A();
   destructed = false;

   //Test destruction and move-construction
   A toMove{destructed};
   newInPlace<A, A const&>(u.a, copy(toMove));
   op = copyOrMove(u.a, move(toMove), true);

   CX_GTEST_SHIM(EXPECT_TRUE, op == CopyOrMoveOperation::CONSTRUCT);
   CX_GTEST_SHIM(EXPECT_TRUE, destructed);
   CX_GTEST_SHIM(EXPECT_TRUE, u.a.moveConstructed);

   //Clean up resources after test
   u.a.~A();

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 TEST(copyOrMove, present_assignable_type_is_assigned) {
  constexpr auto const body = []() constexpr noexcept {
   using Type = CustomType<false, false, true, true, int>;

   union U {
    Type t;

    constexpr U() noexcept {}
    constexpr ~U() noexcept {}
   } u;

   //Test copy-assignment
   int expected = 123;
   Type toCopy{move(expected)};
   newInPlace(u.t, 0);
   auto op = copyOrMove(u.t, copy(toCopy), true);

   CX_GTEST_SHIM(EXPECT_TRUE, op == CopyOrMoveOperation::ASSIGN);
   CX_GTEST_SHIM(EXPECT_TRUE, u.t.copyAssigned);
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<int>(u.t.constructorArguments) == expected
   );

   //Clean up resources between tests
   u.t.~Type();

   //Test move-assignment
   Type toMove{move(expected)};
   newInPlace(u.t, 0);
   op = copyOrMove(u.t, move(toMove), true);

   CX_GTEST_SHIM(EXPECT_TRUE, op == CopyOrMoveOperation::ASSIGN);
   CX_GTEST_SHIM(EXPECT_TRUE, u.t.moveAssigned);
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<int>(u.t.constructorArguments) == expected
   );

   //Clean up resources after test
   u.t.~Type();

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 TEST(copyOrMove, non_present_assignable_type_is_default_constructed_and_assigned) {
  constexpr auto const body = []() constexpr noexcept {
   struct A {
    unsigned long long int i;
    bool
     defaultConstructed = false,
     copyAssigned = false,
     moveAssigned = false;

    constexpr A() noexcept :
     i{0},
     defaultConstructed{true}
    {}

    constexpr A(unsigned long long int i) noexcept :
     i{i},
     defaultConstructed{true}
    {}

    constexpr ~A() noexcept {}

    constexpr A& operator=(A const& other) noexcept {
     i = other.i;
     copyAssigned = true;
     return *this;
    }

    constexpr A& operator=(A&& other) noexcept {
     i = other.i;
     moveAssigned = true;
     return *this;
    }
   };

   union U {
    A a;

    constexpr U() noexcept {}
    constexpr ~U() noexcept {}
   } u;

   unsigned long long int expected = 123411524532;

   //Test copy-assignment
   A toCopy{expected};
   auto op = copyOrMove(u.a, copy(toCopy), false);

   CX_GTEST_SHIM(
    EXPECT_TRUE,
    op == CopyOrMoveOperation::DEFAULT_CONSTRUCT_AND_ASSIGN
   );
   CX_GTEST_SHIM(EXPECT_TRUE, u.a.defaultConstructed);
   CX_GTEST_SHIM(EXPECT_TRUE, u.a.copyAssigned);
   CX_GTEST_SHIM(EXPECT_TRUE, u.a.i == expected);

   //Clean up resources between tests
   u.a.~A();

   //Test move-assignment
   A toMove{expected};
   op = copyOrMove(u.a, move(toMove), false);

   CX_GTEST_SHIM(
    EXPECT_TRUE,
    op == CopyOrMoveOperation::DEFAULT_CONSTRUCT_AND_ASSIGN
   );
   CX_GTEST_SHIM(EXPECT_TRUE, u.a.defaultConstructed);
   CX_GTEST_SHIM(EXPECT_TRUE, u.a.moveAssigned);
   CX_GTEST_SHIM(EXPECT_TRUE, u.a.i == expected);

   //Clean up resources after test
   u.a.~A();

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 TEST(copyOrMove, non_present_only_assignable_type_yields_none) {
  constexpr auto const body = []() constexpr noexcept {
   using Type = CustomType<false, false, true, true, int>;

   union U {
    Type t;

    constexpr U() noexcept {}
    constexpr ~U() noexcept {}
   } u;

   int i = 0;

   //Test copy-assignment
   Type toCopy{move(i)};
   auto op = copyOrMove(u.t, copy(toCopy), false);

   CX_GTEST_SHIM(EXPECT_TRUE, op == CopyOrMoveOperation::NONE);

   //Test move-assignment
   Type toMove{move(i)};
   op = copyOrMove(u.t, move(toMove), false);
   CX_GTEST_SHIM(EXPECT_TRUE, op == CopyOrMoveOperation::NONE);

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }
}
