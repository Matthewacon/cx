#include <cx/test/common/common.h>

#include <cx/result.h>

namespace CX {
 TEST(Result, result_value_copy_constructor_copy_constructs_value) {
  static bool invoked;
  struct A {
   int i;

   A(int i) noexcept :
    i(i)
   {}

   A(A const& other) noexcept :
    i(other.i)
   {
    invoked = true;
   }
  };
  //Reset flag in-case test is re-run
  invoked = false;

  //Create value
  constexpr int expectedValue = 1234;
  A toCopy{expectedValue};
  //Copy construct result with value
  Result<A> r{(A const&)toCopy};
  //Ensure value copy constructor was invoked
  EXPECT_TRUE(invoked);
  //Ensure result is in expected state
  EXPECT_TRUE(r.hasValue());
  EXPECT_FALSE(r.hasError());
  EXPECT_NO_FATAL_FAILURE(([&] {
   EXPECT_EQ(r.getValue().i, expectedValue);
  }()));
  EXPECT_DEATH(
   ([&] {
    r.getError();
   }()),
   ".*"
  );
 }

 TEST(Result, result_value_move_constructor_move_constructs_value) {
  static bool invoked;
  struct A {
   float f;

   A(float f) noexcept :
    f(f)
   {}

   A(A const&) noexcept = delete;

   A(A&& other) noexcept :
    f(other.f)
   {
    other.f = 0;
    invoked = true;
   }
  };
  //Reset flag in-case test is re-run
  invoked = true;

  //Create value to be moved
  static constexpr float expectedValue = 1.23456;
  A toMove{expectedValue};
  Result r{(A&&)toMove};
  EXPECT_TRUE(invoked);
  EXPECT_FLOAT_EQ(toMove.f, 0);
  EXPECT_TRUE(r.hasValue());
  EXPECT_FALSE(r.hasError());
  EXPECT_NO_FATAL_FAILURE(([&] {
   EXPECT_FLOAT_EQ(r.getValue().f, expectedValue);
  }()));
  EXPECT_DEATH(
   ([&] {
    r.getError();
   }()),
   ".*"
  );
 }

 TEST(Result, result_error_copy_constructor_copy_constructs_error) {
  static constexpr char const expectedValue[] = "abcdefg";
  static bool invoked;
  struct ErrorLike {
   char const * msg;

   constexpr ErrorLike(char const * msg = "") noexcept :
    msg{msg}
   {}

   constexpr ErrorLike(ErrorLike const& other) noexcept :
    msg{other.msg}
   {
    invoked = true;
   }

   constexpr char const * describe() const noexcept {
    return msg;
   }
  };
  EXPECT_TRUE((IsError<ErrorLike>));
  //Reset flag in-case test is re-run
  invoked = false;

  //Create error
  ErrorLike toCopy{expectedValue};
  //Copy-construct result with error
  Result r{(ErrorLike const&)toCopy};
  EXPECT_TRUE(invoked);
  EXPECT_FALSE(r.hasValue());
  EXPECT_TRUE(r.hasError());
  EXPECT_NO_FATAL_FAILURE(([&] {
   EXPECT_STREQ(r.getError().describe(), expectedValue);
  }()));
  EXPECT_DEATH(
   ([&] {
    r.getValue();
   }()),
   ".*"
  );
 }

 TEST(Result, result_error_move_constructor_move_constructs_error) {
  static constexpr char const expectedValue[] = "ENOTOBACCO";
  static bool invoked;
  struct ErrorLike {
   char const * msg;

   constexpr ErrorLike(char const * msg) noexcept :
    msg(msg)
   {}

   constexpr ErrorLike(ErrorLike const&) noexcept = delete;

   constexpr ErrorLike(ErrorLike&& other) noexcept :
    msg(other.msg)
   {
    other.msg = nullptr;
    invoked = true;
   }

   constexpr char const * describe() const noexcept {
    return msg;
   }
  };
  EXPECT_TRUE((IsError<ErrorLike>));
  //Reset flag in-case test is re-run
  invoked = false;

  //Create value
  ErrorLike toMove{expectedValue};
  //Move construct result with value
  Result r{(ErrorLike&&)toMove};
  EXPECT_TRUE(invoked);
  EXPECT_EQ(toMove.describe(), nullptr);
  EXPECT_FALSE(r.hasValue());
  EXPECT_TRUE(r.hasError());
  EXPECT_NO_FATAL_FAILURE(([&] {
   EXPECT_STREQ(r.getError().describe(), expectedValue);
  }()));
  EXPECT_DEATH(
   ([&] {
    r.getValue();
   }()),
   ".*"
  );
 }

 TEST(Result, result_copy_constructor_copy_constructs_result) {
  /*
  //Result copy constructor with value
  static bool
   constructorInvoked,
   destructorInvoked;
  struct A {
   short s;

   A(short s) noexcept :
    s(s)
   {}

   A(A const& other) noexcept :
    s(other.s)
   {
    constructorInvoked = true;
   }

   A(A&&) noexcept = delete;

   ~A() noexcept {
    destructorInvoked = true;
   }
  };

  static constexpr short expectedValue = 124;
  A a{expectedValue};
  Result toCopy{(A const&)a};
  constructorInvoked = false;
  destructorInvoked = false;
  EXPECT_TRUE(toCopy.hasValue());

  Result r{(Result<A, Never> const&)toCopy};
  EXPECT_TRUE(constructorInvoked);
  EXPECT_TRUE(destructorInvoked);

  //TODO Result copy constructor with error
  */
  FAIL();
 }

 TEST(Result, result_move_constructor_move_constructs_result) {
  FAIL();
 }

 TEST(Result, result_copy_assignment_operator_copies_other_result) {
  FAIL();
 }

 TEST(Result, result_value_copy_assignment_operator_copies_value) {
  FAIL();
 }

 TEST(Result, result_value_move_assignment_operator_moves_value) {
  FAIL();
 }

 TEST(Result, result_error_copy_assignment_operator_copies_error) {
  FAIL();
 }

 TEST(Result, result_error_move_assignment_operator_moves_error) {
  FAIL();
 }

 TEST(Result, result_move_assignment_operator_moves_other_result) {
  FAIL();
 }

 TEST(Result, result_destructor_destructs_encapsulated_type) {
  FAIL();
 }

 TEST(Result, result_value_or_immediate_operator_behaves_as_expected) {
  FAIL();
 }

 TEST(Result, result_value_or_lazily_evaluted_result_behaves_as_expected) {
  FAIL();
 }

 /*
 TEST(Result, a) {
  printf("sizeof(Never): %lu\n", sizeof(Never));
  Result a{123};
  Result<Never, Error> b{Error{}}, c{Never{}};
  b = c;

  //TODO `e` should contain the value as well
  constexpr Result d{123};
  constexpr Result e = d;
  constexpr auto value = !d;
  (void)value;
 }
 */

 //TODO
 /*
 Result<int> testing() {
  return ok(123);
 }
 */
}
