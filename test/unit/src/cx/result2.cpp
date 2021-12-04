#include <cx/test/common/common.h>

#include <cx/result2.h>

namespace CX {
 //Utility error type for Result tests
 template<typename Base = Never>
 struct ExampleCrtpError final : Base {
  using Base::Base;

  constexpr ExampleCrtpError() noexcept :
   Base{0}
  {}

  static constexpr auto& describe() noexcept {
   return "Example error.";
  }
 };

 TEST(Result, uninitialized_result_is_in_intermediate_state) {
  constexpr auto const body = []() constexpr noexcept {
   Result<int, ExampleCrtpError<>> r;

   CX_GTEST_SHIM(EXPECT_FALSE, r.hasValue());
   CX_GTEST_SHIM(EXPECT_FALSE, r.hasError());

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 TEST(ResultConstruction, copyable_value_initializes_result_value) {
  constexpr auto const body = []() constexpr noexcept {
   using TypeA = CustomType<true, false, false, false, int>;

   int expected = 293847;

   //Result value copy-construction
   TypeA toCopyA{expected};

   Result<TypeA, ExampleCrtpError<>> r1{copy(toCopyA)};

   CX_GTEST_SHIM(EXPECT_TRUE, r1.hasValue());
   CX_GTEST_SHIM(EXPECT_FALSE, r1.hasError());
   auto& v1 = r1.getValue();
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<int>(v1.constructorArguments) == expected
   );
   CX_GTEST_SHIM(EXPECT_TRUE, v1.copyConstructed);

   //Result value default construction and copy-assignment
   using TypeBParent = CustomType<false, false, true, false, int>;
   using TypeB = struct B : TypeBParent {
    using TypeBParent::TypeBParent;

    constexpr B() noexcept :
     TypeBParent{0}
    {}
   };

   TypeB toCopyB{expected};

   Result<TypeB, ExampleCrtpError<>> r2{copy(toCopyB)};

   CX_GTEST_SHIM(EXPECT_TRUE, r2.hasValue());
   CX_GTEST_SHIM(EXPECT_FALSE, r2.hasError());
   auto& v2 = r2.getValue();
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<int>(v2.constructorArguments) == expected
   );
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    v2.defaultConstructed && v2.copyAssigned
   );

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 TEST(ResultConstruction, movable_value_initalizes_result_value) {
  constexpr auto const body = []() constexpr noexcept {
   using TypeA = CustomType<false, true, false, false, short>;

   short expected = 1234;

   //Result value move-construction
   TypeA toMoveA{expected};

   Result<TypeA, ExampleCrtpError<>> r1{move(toMoveA)};

   CX_GTEST_SHIM(EXPECT_TRUE, r1.hasValue());
   CX_GTEST_SHIM(EXPECT_FALSE, r1.hasError());
   auto& v1 = r1.getValue();
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<short>(v1.constructorArguments) == expected
   );
   CX_GTEST_SHIM(EXPECT_TRUE, v1.moveConstructed);

   //Result value default construction and copy-assignment
   using TypeBParent = CustomType<false, false, false, true, short>;
   using TypeB = struct B : TypeBParent {
    using TypeBParent::TypeBParent;

    constexpr B() noexcept :
     TypeBParent{0}
    {}
   };

   TypeB toMoveB{expected};

   Result<TypeB, ExampleCrtpError<>> r2{move(toMoveB)};

   CX_GTEST_SHIM(EXPECT_TRUE, r2.hasValue());
   CX_GTEST_SHIM(EXPECT_FALSE, r2.hasError());
   auto& v2 = r2.getValue();
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<short>(v2.constructorArguments) == expected
   );
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    v2.defaultConstructed && v2.moveAssigned
   );

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }


 TEST(ResultConstruction, copyable_error_initializes_result_error) {
  constexpr auto const body = []() constexpr noexcept {
   //Result error copy-construction
   using TypeA = ExampleCrtpError<CustomType<true, false, false, false, int>>;

   int expected = 31415;
   TypeA toCopyA{expected};
   Result <Never, TypeA> r1{copy(toCopyA)};

   CX_GTEST_SHIM(EXPECT_FALSE, r1.hasValue());
   CX_GTEST_SHIM(EXPECT_TRUE, r1.hasError());
   auto& e1 = r1.getError();
   CX_GTEST_SHIM(EXPECT_TRUE, e1.copyConstructed);
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<int>(e1.constructorArguments) == expected
   );

   //Result error copy-assignment
   using TypeB = ExampleCrtpError<CustomType<false, false, true, false, int>>;
   TypeB toCopyB{expected};
   Result<Never, TypeB> r2{copy(toCopyB)};

   CX_GTEST_SHIM(EXPECT_FALSE, r2.hasValue());
   CX_GTEST_SHIM(EXPECT_TRUE, r2.hasError());
   auto& e2 = r2.getError();
   CX_GTEST_SHIM(EXPECT_TRUE, e2.copyAssigned);
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<int>(e2.constructorArguments) == expected
   );

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 TEST(ResultConstruction, movable_error_initializes_result_error) {
  constexpr auto const body = []() constexpr noexcept {
   //Result error move-construction
   using TypeA = ExampleCrtpError<CustomType<false, true, false, false, int>>;

   int expected = 1242546;
   TypeA toMoveA{expected};
   Result <Never, TypeA> r1{move(toMoveA)};

   CX_GTEST_SHIM(EXPECT_FALSE, r1.hasValue());
   CX_GTEST_SHIM(EXPECT_TRUE, r1.hasError());
   auto& e1 = r1.getError();
   CX_GTEST_SHIM(EXPECT_TRUE, e1.moveConstructed);
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<int>(e1.constructorArguments) == expected
   );

   //Result error move-assignment
   using TypeB = ExampleCrtpError<CustomType<false, false, false, true, int>>;
   TypeB toMoveB{expected};
   Result<Never, TypeB> r2{move(toMoveB)};

   CX_GTEST_SHIM(EXPECT_FALSE, r2.hasValue());
   CX_GTEST_SHIM(EXPECT_TRUE, r2.hasError());
   auto& e2 = r2.getError();
   CX_GTEST_SHIM(EXPECT_TRUE, e2.moveAssigned);
   CX_GTEST_SHIM(
    EXPECT_TRUE,
    std::get<int>(e2.constructorArguments) == expected
   );

   return 0;
  };

  constexpr auto const ct = body();
  volatile auto rt = body();
  (void)ct;
  (void)rt;
 }

 TEST(ResultConstruction, result_copy_constructor_copies_result_with_value) {
  //TODO Test with copyable value

  //TODO Test with movable value
  using TypeA = CustomType<true, false, false, false, int>;

  int expected = 213985;
  TypeA valueToMove{expected};
  Result<TypeA, ExampleCrtpError<>> resultToMove{copy(valueToMove)};
  //TODO verify `resultToMove` state
  //Result<TypeA, ExampleCrtpError<>> r{move(resultToMove)};
 }

 TEST(ResultConstruction, result_move_constructor_moves_result_with_value) {
  FAIL();
 }

 TEST(ResultConstruction, result_copy_constructor_copies_result_with_error) {
  FAIL();
 }

 TEST(ResultConstruction, result_move_constructor_moves_result_with_error) {
  FAIL();
 }
}
