#include <cx/test/common/common.h>

#include <cx/lambda.h>

namespace CX {
 TEST(IsLambda, lambda_types_satisfy_constraint) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(IsLambda, std_function_satisfies_constraint) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(IsLambda, non_lambda_types_do_not_satisfy_constraint) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(CompatibleLambda, qualified_lambda_to_unqualified_lambda_satisfies_constraint) {
  //TODO the following cases should be true
  // Lambda<void () noexcept> -> Lambda<void ()>
  // AllocLambda<void () const noexcept> -> AllocLambda<void ()>
  // Lambda<void () noexcept> -> AllocLambda<void ()>
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(CompatibleLambda, unqualified_lambda_to_qualified_lambda_does_not_satisfy_constraint) {
  //TODO invert cases from true-equivalent test
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(CompatibleLambda, alloc_lambda_conversion_to_non_alloc_lambda_satisfies_constraint) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(CompatibleLambda, non_alloc_lambda_conversion_to_alloc_lambda_satisfies_constraint) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, lambda_default_constructor_initializes_empty_lambda) {
  //Construct lambda and ensure it was correctly initialized
  Lambda l;
  EXPECT_FALSE(l);

  //Invoking an empty lambda should throw an exception
  EXPECT_THROW(
   ([&] {
    l();
   }()),
   UninitializedLambdaError
  );
 }

 TEST(LambdaConstructor, lambda_function_pointer_constructor_correctly_initializes_lambda) {
  static constexpr auto const iExpected = 3249;
  static constexpr auto const cExpected = 'H';
  static constexpr auto const returnExpected = 1.2345;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  //Note: (`+(lambda expr...)` explicitly converts a non-capturing
  //lambda to a function pointer with an equivalent prototype)
  auto const fptr = +[](int i, char c) -> float {
   invoked = true;
   EXPECT_EQ(i, iExpected);
   EXPECT_EQ(c, cExpected);
   return returnExpected;
  };
  Lambda l{(decltype(fptr) const&)fptr};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(iExpected, cExpected);
   EXPECT_TRUE(invoked);
   EXPECT_FLOAT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaConstructor, noexcept_lambda_funcftion_pointer_constructor_correctly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, c_variadic_lambda_function_pointer_constructor_correctly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_function_pointer_constructor_correctly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, lambda_function_copy_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, noexcept_lambda_function_copy_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, c_variadic_lambda_function_copy_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_function_copy_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, lambda_function_move_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, noexcept_lambda_function_move_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, c_variadic_lambda_function_move_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_function_move_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, lambda_copy_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, noexcept_lambda_copy_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, c_variadic_lambda_copy_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_copy_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, lambda_move_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, noexcept_lambda_move_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, c_variadic_lambda_move_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_move_constructor_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, lambda_function_pointer_assignment_operator_correctly_initializes_lambda) {
  static constexpr auto const iExpected = 3249;
  static constexpr auto const cExpected = 'H';
  static constexpr auto const returnExpected = 1.2345;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda empty lambda
  Lambda<float (int, char)> l;

  //Copy assign to lambda and ensure it was correctly initializes
  auto const fptr = +[](int i, char c) -> float {
   invoked = true;
   EXPECT_EQ(i, iExpected);
   EXPECT_EQ(c, cExpected);
   return returnExpected;
  };
  l = (decltype(fptr) const&)fptr;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(iExpected, cExpected);
   EXPECT_TRUE(invoked);
   EXPECT_FLOAT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaAssignment, noexcept_lambda_funcftion_pointer_assignment_operator_correctly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, c_variadic_lambda_function_pointer_assignment_operator_correctly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_function_pointer_assignment_operator_correctly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, lambda_function_copy_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, noexcept_lambda_function_copy_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, c_variadic_lambda_function_copy_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_function_copy_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, lambda_function_move_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, noexcept_lambda_function_move_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, c_variadic_lambda_function_move_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_function_move_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, lambda_copy_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, noexcept_lambda_copy_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, c_variadic_lambda_copy_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_copy_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, lambda_move_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, noexcept_lambda_move_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, c_variadic_lambda_move_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_move_assignment_operator_properly_initializes_lambda) {
  throw std::runtime_error{"Unimplemented"};
 }

 //TODO AllocLambda tests
 //TODO Lambda <-> AllocLambda conversion tests
 //TODO [Lambda, AllocLambda] implicit conversion tests
 //TODO C-Variadic lambda conversion to fptr tests
}
