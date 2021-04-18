#include <cx/test/common/common.h>

#include <cx/vararg.h>
#include <cx/lambda.h>

namespace CX {
 TEST(IsLambda, lambda_types_satisfy_constraint) {
  EXPECT_TRUE((IsLambda<Lambda<void ()>>));
  EXPECT_TRUE((IsLambda<Lambda<void () noexcept>>));
  EXPECT_TRUE((IsLambda<Lambda<void (...)>>));
  EXPECT_TRUE((IsLambda<Lambda<void (...) noexcept>>));
  EXPECT_TRUE((IsLambda<AllocLambda<void ()>>));
  EXPECT_TRUE((IsLambda<AllocLambda<void () noexcept>>));
  EXPECT_TRUE((IsLambda<AllocLambda<void (...)>>));
  EXPECT_TRUE((IsLambda<AllocLambda<void (...) noexcept>>));
 }

 TEST(IsLambda, non_lambda_types_do_not_satisfy_constraint) {
  EXPECT_FALSE((IsLambda<void>));
  EXPECT_FALSE((IsLambda<std::function<void ()>>));
  EXPECT_FALSE((IsLambda<Lambda<void (*)()>>));
 }

 TEST(CompatibleLambda, qualified_lambda_to_unqualified_lambda_satisfies_constraint) {
  //Qualifier demotions
  EXPECT_TRUE((CompatibleLambda<Lambda<void () noexcept>, Lambda<void ()>>));
  EXPECT_TRUE((CompatibleLambda<AllocLambda<void () noexcept>, AllocLambda<void ()>>));
  EXPECT_TRUE((CompatibleLambda<Lambda<void (...) noexcept>, Lambda<void (...)>>));
  EXPECT_TRUE((CompatibleLambda<AllocLambda<void (...) noexcept>, AllocLambda<void (...)>>));

  //Qualifier demotions + conversion between lambda types
  EXPECT_TRUE((CompatibleLambda<Lambda<void () noexcept>, AllocLambda<void ()>>));
  EXPECT_TRUE((CompatibleLambda<Lambda<void (...) noexcept>, AllocLambda<void (...)>>));
  EXPECT_TRUE((CompatibleLambda<AllocLambda<void () noexcept>, Lambda<void ()>>));
  EXPECT_TRUE((CompatibleLambda<AllocLambda<void (...) noexcept>, Lambda<void (...)>>));
 }

 TEST(CompatibleLambda, unqualified_lambda_to_qualified_lambda_does_not_satisfy_constraint) {
  //Qualifier promotions
  EXPECT_FALSE((CompatibleLambda<Lambda<void ()>, Lambda<void () noexcept>>));
  EXPECT_FALSE((CompatibleLambda<AllocLambda<void ()>, AllocLambda<void () noexcept>>));
  EXPECT_FALSE((CompatibleLambda<Lambda<void (...)>, Lambda<void (...) noexcept>>));
  EXPECT_FALSE((CompatibleLambda<AllocLambda<void (...)>, AllocLambda<void (...) noexcept>>));

  //Qualifier promotions + conversion between lambda types
  EXPECT_FALSE((CompatibleLambda<Lambda<void ()>, AllocLambda<void () noexcept>>));
  EXPECT_FALSE((CompatibleLambda<Lambda<void (...)>, AllocLambda<void (...) noexcept>>));
  EXPECT_FALSE((CompatibleLambda<AllocLambda<void ()>, Lambda<void () noexcept>>));
  EXPECT_FALSE((CompatibleLambda<AllocLambda<void (...)>, Lambda<void (...) noexcept>>));
 }

 TEST(CompatibleLambda, alloc_lambda_conversion_to_non_alloc_lambda_satisfies_constraint) {
  EXPECT_TRUE((CompatibleLambda<AllocLambda<void ()>, Lambda<void ()>>));
  EXPECT_TRUE((CompatibleLambda<AllocLambda<void (...)>, Lambda<void (...)>>));
  EXPECT_TRUE((CompatibleLambda<AllocLambda<void () noexcept>, Lambda<void () noexcept>>));
  EXPECT_TRUE((CompatibleLambda<AllocLambda<void (...) noexcept>, Lambda<void (...) noexcept>>));
 }

 TEST(CompatibleLambda, non_alloc_lambda_conversion_to_alloc_lambda_satisfies_constraint) {
  EXPECT_TRUE((CompatibleLambda<Lambda<void ()>, AllocLambda<void ()>>));
  EXPECT_TRUE((CompatibleLambda<Lambda<void (...)>, AllocLambda<void (...)>>));
  EXPECT_TRUE((CompatibleLambda<Lambda<void () noexcept>, AllocLambda<void () noexcept>>));
  EXPECT_TRUE((CompatibleLambda<Lambda<void (...) noexcept>, AllocLambda<void (...) noexcept>>));
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
  static constexpr double const dExpected = 0.571082;
  static constexpr char32_t const cExpected = 1130426;
  static constexpr float const returnExpected = dExpected * cExpected;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto const fptr = +[](double d, char32_t c) -> float {
   invoked = true;
   EXPECT_EQ(d, dExpected);
   EXPECT_TRUE(c == cExpected);
   return returnExpected;
  };
  Lambda l{(decltype(fptr) const&)fptr};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(dExpected, cExpected);
   EXPECT_TRUE(invoked);
   EXPECT_FLOAT_EQ(returned, returnExpected);
  }()));
 }

 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wvarargs"

 template<typename L>
 struct VariadicLambdaWrapper;

 template<typename T, typename R, typename... Args>
 requires (sizeof...(Args) > 0)
 struct VariadicLambdaWrapper<R (T::*)(Args..., ...)> {
  static R invoke(Args... args, ...) {
   CX::VaList list;
   va_start(list, argumentAtIndex<0>(args...));
   T t;
   return t(args..., list);
  }
 };

 template<typename T, typename R, typename... Args>
 requires (sizeof...(Args) > 0)
 struct VariadicLambdaWrapper<R (T::*)(Args..., ...) noexcept> {
  static R invoke(Args... args, ...) noexcept {
   CX::VaList list;
   va_start(list, argumentAtIndex<0>(args...));
   T t;
   return t(args..., list);
  }
 };

 #pragma GCC diagnostic pop

 TEST(LambdaConstructor, c_variadic_lambda_function_pointer_constructor_correctly_initializes_lambda) {
  static constexpr uintptr_t const vExpected = (uintptr_t)0x571046;
  static constexpr long double const dExpected = 1.130238;
  static constexpr unsigned int const iExpected = 2615074;
  static constexpr float const varargs[] = {-.276349, -.195284, 1.097302, 0.337187};
  static constexpr char const returnExpected = 100;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto const lambda = [](uintptr_t v, long double d, unsigned int i, CX::VaList &list) -> char {
   invoked = true;
   EXPECT_EQ(v, vExpected);
   EXPECT_TRUE(d == dExpected);
   EXPECT_EQ(i, iExpected);

   //Verify va-list arguments
   for (auto i = 0; i < 4; i++) {
    EXPECT_FLOAT_EQ((list.arg<float>()), (varargs[i]));
   }

   return returnExpected;
  };
  auto const fptr = &VariadicLambdaWrapper<char (decltype(lambda)::*)(uintptr_t, long double, unsigned int, ...)>::invoke;
  Lambda l{(decltype(fptr) const&)fptr};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(vExpected, dExpected, iExpected, varargs[0], varargs[1], varargs[2], varargs[3]);
   EXPECT_TRUE(invoked);
   EXPECT_TRUE(returned == returnExpected);
  }()));
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_function_pointer_constructor_correctly_initializes_lambda) {
  static constexpr uintptr_t const vExpected = (uintptr_t)0x571046;
  static constexpr long double const dExpected = 1.130238;
  static constexpr unsigned int const iExpected = 2615074;
  static constexpr float const varargs[] = {-.276349, -.195284, 1.097302, 0.337187};
  static constexpr char const returnExpected = 100;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto const lambda = [](uintptr_t v, long double d, unsigned int i, CX::VaList &list) -> char {
   invoked = true;
   EXPECT_EQ(v, vExpected);
   EXPECT_TRUE(d == dExpected);
   EXPECT_EQ(i, iExpected);

   //Verify va-list arguments
   for (auto i = 0; i < 4; i++) {
    EXPECT_FLOAT_EQ((list.arg<float>()), (varargs[i]));
   }

   return returnExpected;
  };
  auto const fptr = &VariadicLambdaWrapper<char (decltype(lambda)::*)(uintptr_t, long double, unsigned int, ...)>::invoke;
  Lambda l{(decltype(fptr) const&)fptr};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(vExpected, dExpected, iExpected, varargs[0], varargs[1], varargs[2], varargs[3]);
   EXPECT_TRUE(invoked);
   EXPECT_TRUE(returned == returnExpected);
  }()));
 }

 TEST(LambdaConstructor, lambda_function_copy_constructor_properly_initializes_lambda) {
  static auto const returnExpected = (void *)0x409420;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto const lambda = []() -> void * {
   invoked = true;
   return returnExpected;
  };
  Lambda l{(decltype(lambda) const&)lambda};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l();
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaConstructor, noexcept_lambda_function_copy_constructor_properly_initializes_lambda) {
  static constexpr unsigned short const sExpected = 34876;
  static constexpr int const returnExpected = 456914;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto const lambda = [](unsigned short s) noexcept -> int {
   invoked = true;
   EXPECT_EQ(s, sExpected);
   return returnExpected;
  };
  Lambda l{(decltype(lambda) const&)lambda};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(sExpected);
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
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

 //TODO
 // - AllocLambda tests
 // - Lambda <-> AllocLambda conversion tests
 // - [Lambda, AllocLambda] implicit conversion tests
 // - [Lambda, AllocLambda] noexcept-qualified lambda inst/lambda/fptr assignment to
 //   unqualified lambda tests
 // - C-Variadic lambda conversion to fptr tests
}
