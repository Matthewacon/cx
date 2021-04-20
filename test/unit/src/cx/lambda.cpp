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
  auto const fptr = +[](double d, char32_t c) noexcept -> float {
   invoked = true;
   EXPECT_EQ(d, dExpected);
   EXPECT_TRUE(c == cExpected);
   return returnExpected;
  };
  Lambda l{(decltype(fptr) const&)fptr};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
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
  auto const lambda = [](uintptr_t v, long double d, unsigned int i, CX::VaList &list) noexcept -> char {
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
  auto const fptr = &VariadicLambdaWrapper<char (decltype(lambda)::*)(uintptr_t, long double, unsigned int, ...) noexcept>::invoke;
  Lambda l{(decltype(fptr) const&)fptr};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
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
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l(sExpected);
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaConstructor, c_variadic_lambda_function_copy_constructor_properly_initializes_lambda) {
  static constexpr long double d1Expected = 1.130211;
  static constexpr double d2Expected = 0.523299;
  static constexpr char16_t returnExpected = 2345;
  static constexpr long long varargs[] {283765, 2093480934875};

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  constexpr auto const lambda = [](long double d1, double d2, ...) {
   CX::VaList list;
   va_start(list, d2);
   invoked = true;
   EXPECT_TRUE(d1 == d1Expected);
   EXPECT_TRUE(d2 == d2Expected);
   for (int i = 0; i < 2; i++) {
    EXPECT_EQ((list.arg<long long>()), (varargs[i]));
   }
   return returnExpected;
  };
  Lambda l{(decltype(lambda) const&)lambda};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(d1Expected, d2Expected, varargs[0], varargs[1]);
   EXPECT_TRUE(invoked);
   EXPECT_TRUE(returned == returnExpected);
  }()));
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_function_copy_constructor_properly_initializes_lambda) {
  static constexpr int const iExpected = 'L';
  static constexpr float returnExpected = 1.129848;
  static constexpr char const varargs[] = "Hello";

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  constexpr auto const lambda = [](int i, ...) noexcept {
   CX::VaList list;
   va_start(list, i);
   invoked = true;
   EXPECT_EQ(i, iExpected);
   //Verify varargs
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_EQ((list.arg<char>()), (varargs[l]));
   }
   return returnExpected;
  };
  Lambda l{(decltype(lambda) const&)lambda};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5]
   );
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaConstructor, lambda_function_move_constructor_properly_initializes_lambda) {
  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto lambda = [] {
   invoked = true;
  };
  Lambda l{(decltype(lambda)&&)lambda};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   l();
   EXPECT_TRUE(invoked);
  }()));
 }

 TEST(LambdaConstructor, noexcept_lambda_function_move_constructor_properly_initializes_lambda) {
  static constexpr int const iExpected = 2435;
  static constexpr float const fExpected = 3463457.254;
  static constexpr double const returnExpected = 3;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto lambda = [](int i, float f) noexcept -> double {
   invoked = true;
   EXPECT_EQ(i, iExpected);
   EXPECT_EQ(f, fExpected);
   return returnExpected;
  };
  Lambda l{(decltype(lambda)&&)lambda};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l(iExpected, fExpected);
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaConstructor, c_variadic_lambda_function_move_constructor_properly_initializes_lambda) {
  static constexpr int const iExpected = 2435;
  static constexpr double const dExpected = 3463457.254;
  static constexpr float const varargs[] = {3.14, 6.28, 2.71};
  static constexpr float const returnExpected = 3;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto lambda = [](int i, double d, ...) -> float {
   CX::VaList list;
   va_start(list, d);
   invoked = true;
   EXPECT_EQ(i, iExpected);
   EXPECT_EQ(d, dExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_FLOAT_EQ((list.arg<float>()), (varargs[l]));
   }
   return returnExpected;
  };
  Lambda l{(decltype(lambda)&&)lambda};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(
    iExpected,
    dExpected,
    varargs[0],
    varargs[1],
    varargs[2]
   );
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_function_move_constructor_properly_initializes_lambda) {
  static constexpr int const iExpected = 4;
  static constexpr long double const varargs[] = {1, 2, 3, 4};

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto lambda = [](int i, ...) noexcept {
   CX::VaList list;
   va_start(list, i);
   invoked = true;
   EXPECT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_FLOAT_EQ((list.arg<long double>()), (varargs[l]));
   }
  };
  Lambda l{(decltype(lambda)&&)lambda};
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   l(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3]
   );
   EXPECT_TRUE(invoked);
  }()));
 }

 TEST(LambdaConstructor, lambda_copy_constructor_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<int (float)> l1;
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1(0);
   }()),
   UninitializedLambdaError
  );

  //Copy construct lambda and ensure it was correctly initialized
  Lambda l2{(decltype(l1) const&)l1};
  EXPECT_FALSE(l2);
  EXPECT_THROW(
   ([&] {
    l2(0);
   }()),
   UninitializedLambdaError
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr float const fExpected = 2463987;
  static constexpr int const returnExpected = 2456;

  Lambda l3{[](float f) {
   invoked++;
   EXPECT_FLOAT_EQ(f, fExpected);
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_THROW(([&] {
   auto returned = l3(fExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Copy construct lambda and ensure it was correctly initialized
  Lambda l4{(decltype(l3) const&)l3};
  EXPECT_TRUE(l4);
  EXPECT_NO_THROW(([&] {
   auto returned = l4(fExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));
 }

 TEST(LambdaConstructor, noexcept_lambda_copy_constructor_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void () noexcept> l1;
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1();
   }()),
   "UninitializedLambdaError"
  );

  //Copy construct lambda and ensure it was correctly initialized
  Lambda l2{(decltype(l1) const&)l1};
  EXPECT_FALSE(l2);
  EXPECT_DEATH(
   ([&] {
    l2();
   }()),
   "UninitializedLambdaError"
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  Lambda l3{[]() noexcept {
   invoked++;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_FATAL_FAILURE(([&] {
   l3();
   EXPECT_EQ(invoked, 1);
  }()));

  //Copy construct lambda and ensure it was correctly initialized
  Lambda l4{(decltype(l3) const&)l3};
  EXPECT_TRUE(l4);
  EXPECT_NO_FATAL_FAILURE(([&] {
   l4();
   EXPECT_EQ(invoked, 2);
  }()));
 }

 TEST(LambdaConstructor, c_variadic_lambda_copy_constructor_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void (...)> l1;
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1(0);
   }()),
   UninitializedLambdaError
  );

  //Copy construct lambda and ensure it was correctly initialized
  Lambda l2{(decltype(l1) const&)l1};
  EXPECT_FALSE(l2);
  EXPECT_THROW(
   ([&] {
    l2(0);
   }()),
   UninitializedLambdaError
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr char const varargs[] = "lambda";
  static constexpr int const iExpected = ArraySize<decltype(varargs)>;
  static constexpr char const returnExpected = 'F';

  Lambda l3{[](int i, ...) -> char {
   CX::VaList list;
   va_start(list, i);
   invoked++;
   EXPECT_FLOAT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<char>() == varargs[l]));
   }
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_THROW(([&] {
   auto returned = l3(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Copy construct lambda and ensure it was correctly initialized
  Lambda l4{(decltype(l3) const&)l3};
  EXPECT_TRUE(l4);
  EXPECT_NO_THROW(([&] {
   auto returned = l4(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_copy_constructor_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void (...) noexcept> l1;
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1(0);
   }()),
   "UninitializedLambdaError"
  );

  //Copy construct lambda and ensure it was correctly initialized
  Lambda l2{(decltype(l1) const&)l1};
  EXPECT_FALSE(l2);
  EXPECT_DEATH(
   ([&] {
    l2(0);
   }()),
   "UninitializedLambdaError"
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr char const varargs[] = "lambda";
  static constexpr int const iExpected = ArraySize<decltype(varargs)>;
  static constexpr char const returnExpected = 'F';

  Lambda l3{[](int i, ...) noexcept -> char {
   CX::VaList list;
   va_start(list, i);
   invoked++;
   EXPECT_FLOAT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<char>() == varargs[l]));
   }
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l3(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Copy construct lambda and ensure it was correctly initialized
  Lambda l4{(decltype(l3) const&)l3};
  EXPECT_TRUE(l4);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l4(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));
 }

 TEST(LambdaConstructor, lambda_move_constructor_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void ()> l1;
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1();
   }()),
   UninitializedLambdaError
  );

  //Move construct lambda and ensure it was correctly initialized
  Lambda l2{(decltype(l1)&&)l1};
  EXPECT_FALSE(l2);
  EXPECT_THROW(
   ([&] {
    l2();
   }()),
   UninitializedLambdaError
  );

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1();
   }()),
   UninitializedLambdaError
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr float const fExpected = 2463987;
  static constexpr int const returnExpected = 2456;

  Lambda l3{[](float f) {
   invoked++;
   EXPECT_FLOAT_EQ(f, fExpected);
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_THROW(([&] {
   auto returned = l3(fExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Move construct lambda and ensure it was correctly initialized
  Lambda l4{(decltype(l3)&&)l3};
  EXPECT_TRUE(l4);
  EXPECT_NO_THROW(([&] {
   auto returned = l4(fExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l3);
  EXPECT_THROW(
   ([&] {
    l3(1254);
   }()),
   UninitializedLambdaError
  );
  EXPECT_EQ(invoked, 2);
 }

 TEST(LambdaConstructor, noexcept_lambda_move_constructor_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void () noexcept> l1;
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1();
   }()),
   "UninitializedLambdaError"
  );

  //Move construct lambda and ensure it was correctly initialized
  Lambda l2{(decltype(l1)&&)l1};
  EXPECT_FALSE(l2);
  EXPECT_DEATH(
   ([&] {
    l2();
   }()),
   "UninitializedLambdaError"
  );

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1();
   }()),
   "UninitializedLambdaError"
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr float const fExpected = 2463987;
  static constexpr long double const dExpected = 932487.827365;
  static constexpr int const returnExpected = -1;

  Lambda l3{[](float f, long double d) noexcept {
   invoked++;
   EXPECT_FLOAT_EQ(f, fExpected);
   EXPECT_DOUBLE_EQ(d, dExpected);
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l3(fExpected, dExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Move construct lambda and ensure it was correctly initialized
  Lambda l4{(decltype(l3)&&)l3};
  EXPECT_TRUE(l4);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l4(fExpected, dExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l3);
  EXPECT_DEATH(
   ([&] {
    l3(-1, 1);
   }()),
   "UninitializedLambdaError"
  );
  EXPECT_EQ(invoked, 2);
 }

 TEST(LambdaConstructor, c_variadic_lambda_move_constructor_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void (...)> l1;
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1(0);
   }()),
   UninitializedLambdaError
  );

  //Move construct lambda and ensure it was correctly initialized
  Lambda l2{(decltype(l1)&&)l1};
  EXPECT_FALSE(l2);
  EXPECT_THROW(
   ([&] {
    l2(0);
   }()),
   UninitializedLambdaError
  );

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1(0);
   }()),
   UninitializedLambdaError
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr char const varargs[] = "lambda";
  static constexpr int const iExpected = ArraySize<decltype(varargs)>;
  static constexpr char const returnExpected = 'F';

  Lambda l3{[](int i, ...) -> char {
   CX::VaList list;
   va_start(list, i);
   invoked++;
   EXPECT_FLOAT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<char>() == varargs[l]));
   }
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_THROW(([&] {
   auto returned = l3(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Move construct lambda and ensure it was correctly initialized
  Lambda l4{(decltype(l3)&&)l3};
  EXPECT_TRUE(l4);
  EXPECT_NO_THROW(([&] {
   auto returned = l4(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));

  //Ensure moved lambda is uninitialized
  EXPECT_THROW(
   ([&] {
    l3(0);
   }()),
   UninitializedLambdaError
  );
 }

 TEST(LambdaConstructor, noexcept_c_variadic_lambda_move_constructor_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void (...) noexcept> l1;
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1(0);
   }()),
   "UninitializedLambdaError"
  );

  //Move construct lambda and ensure it was correctly initialized
  Lambda l2{(decltype(l1)&&)l1};
  EXPECT_FALSE(l2);
  EXPECT_DEATH(
   ([&] {
    l2(0);
   }()),
   "UninitializedLambdaError"
  );

  //Ensure moved lambda is unitialized
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1(0);
   }()),
   "UninitializedLambdaError"
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Move lambda and ensure it was correctly initialized
  static constexpr char const varargs[] = "lambda";
  static constexpr int const iExpected = ArraySize<decltype(varargs)>;
  static constexpr char const returnExpected = 'F';

  Lambda l3{[](int i, ...) noexcept -> char {
   CX::VaList list;
   va_start(list, i);
   invoked++;
   EXPECT_FLOAT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<char>() == varargs[l]));
   }
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l3(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Move construct lambda and ensure it was correctly initialized
  Lambda l4{(decltype(l3)&&)l3};
  EXPECT_TRUE(l4);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l4(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));

  //Ensure moved lambda is uninitialized
  EXPECT_DEATH(
   ([&] {
    l3(0);
   }()),
   "UninitializedLambdaError"
  );
 }

 TEST(LambdaAssignment, lambda_function_pointer_assignment_operator_correctly_initializes_lambda) {
  static constexpr auto const iExpected = 3249;
  static constexpr auto const cExpected = 'H';
  static constexpr auto const returnExpected = 1.2345;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Copy assign to lambda and ensure it was correctly initializes
  Lambda<float (int, char)> l = +[](int i, char c) -> float {
   invoked = true;
   EXPECT_EQ(i, iExpected);
   EXPECT_EQ(c, cExpected);
   return returnExpected;
  };
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l(iExpected, cExpected);
   EXPECT_TRUE(invoked);
   EXPECT_FLOAT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaAssignment, noexcept_lambda_funcftion_pointer_assignment_operator_correctly_initializes_lambda) {
  static constexpr double const dExpected = 3249;
  static constexpr char const cExpected = 'A';
  static constexpr float const returnExpected = 32467.3;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Copy assign to lambda and ensure it was correctly initialized
  Lambda l = +[](double d, char c) noexcept -> float {
   invoked = true;
   EXPECT_EQ(d, dExpected);
   EXPECT_EQ(c, cExpected);
   return returnExpected;
  };
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l(dExpected, cExpected);
   EXPECT_TRUE(invoked);
   EXPECT_FLOAT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaAssignment, c_variadic_lambda_function_pointer_assignment_operator_correctly_initializes_lambda) {
  static constexpr auto const iExpected = 7;
  static constexpr short const varargs[] = {1, 2, 3, 4, 5, 6, 7};

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Default construct empty lambda
  Lambda<void (int, ...)> l;


  auto const lambda = [](int i, CX::VaList &list) {
   invoked = true;
   EXPECT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<short>() == varargs[l]));
   }
  };

  //Copy assign to lambda and ensure it was correctly initialized
  l = &VariadicLambdaWrapper<void (decltype(lambda)::*)(int, ...)>::invoke;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   l(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_TRUE(invoked);
  }()));
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_function_pointer_assignment_operator_correctly_initializes_lambda) {
  static constexpr int const i1Expected = 324780;
  static constexpr int const i2Expected = 6;
  static constexpr char const varargs[] = "hello";
  static constexpr char const returnExpected = 'Z';

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Default construct empty lambda
  Lambda<char (int, int, ...) noexcept> l;

  auto const lambda = [](int i1, int i2, CX::VaList &list) noexcept -> char {
   invoked = true;
   EXPECT_EQ(i1, i1Expected);
   EXPECT_EQ(i2, i2Expected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<char>() == varargs[l]));
   }
   return returnExpected;
  };

  //Copy assign to lambda and ensure it was correctly initialized
  l = &VariadicLambdaWrapper<char (decltype(lambda)::*)(int, int, ...) noexcept>::invoke;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l(
    i1Expected,
    i2Expected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5]
   );
   EXPECT_TRUE(invoked);
   EXPECT_TRUE(returned == returnExpected);
  }()));
 }

 TEST(LambdaAssignment, lambda_function_copy_assignment_operator_properly_initializes_lambda) {
  static constexpr bool const bExpected = false;
  static constexpr char16_t cExpected = 'A';

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto const lambda = [](bool b, char16_t c) {
   invoked = true;
   EXPECT_EQ(b, bExpected);
   EXPECT_TRUE(c == cExpected);
  };
  Lambda<void (bool, char16_t)> l;
  l = (decltype(lambda) const&)lambda;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   l(bExpected, cExpected);
   EXPECT_TRUE(invoked);
  }()));
 }

 TEST(LambdaAssignment, noexcept_lambda_function_copy_assignment_operator_properly_initializes_lambda) {
  static constexpr char32_t const cExpected = 'd';
  static constexpr short const sExpected = 354;
  static constexpr float const returnExpected = 3.24152;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto const lambda = [](char32_t c, short s) noexcept -> float {
   invoked = true;
   EXPECT_TRUE(c == cExpected);
   EXPECT_EQ(s, sExpected);
   return returnExpected;
  };
  Lambda l = (decltype(lambda) const&)lambda;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l(cExpected, sExpected);
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaAssignment, c_variadic_lambda_function_copy_assignment_operator_properly_initializes_lambda) {
  static constexpr long double d1Expected = 1.130211;
  static constexpr double d2Expected = 0.523299;
  static constexpr char16_t returnExpected = 2345;
  static constexpr long long varargs[] {283765, 2093480934875};

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  constexpr auto const lambda = [](long double d1, double d2, ...) {
   CX::VaList list;
   va_start(list, d2);
   invoked = true;
   EXPECT_TRUE(d1 == d1Expected);
   EXPECT_TRUE(d2 == d2Expected);
   for (int i = 0; i < 2; i++) {
    EXPECT_EQ((list.arg<long long>()), (varargs[i]));
   }
   return returnExpected;
  };
  Lambda l = (decltype(lambda) const&)lambda;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(d1Expected, d2Expected, varargs[0], varargs[1]);
   EXPECT_TRUE(invoked);
   EXPECT_TRUE(returned == returnExpected);
  }()));
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_function_copy_assignment_operator_properly_initializes_lambda) {
  static constexpr int const iExpected = 'L';
  static constexpr float returnExpected = 1.129848;
  static constexpr char const varargs[] = "Hello";

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  constexpr auto const lambda = [](int i, ...) noexcept {
   CX::VaList list;
   va_start(list, i);
   invoked = true;
   EXPECT_EQ(i, iExpected);
   //Verify varargs
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_EQ((list.arg<char>()), (varargs[l]));
   }
   return returnExpected;
  };
  Lambda l = (decltype(lambda) const&)lambda;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5]
   );
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaAssignment, lambda_function_move_assignment_operator_properly_initializes_lambda) {
  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto lambda = [] {
   invoked = true;
  };
  Lambda l = (decltype(lambda)&&)lambda;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   l();
   EXPECT_TRUE(invoked);
  }()));
 }

 TEST(LambdaAssignment, noexcept_lambda_function_move_assignment_operator_properly_initializes_lambda) {
  static constexpr int const iExpected = 2435;
  static constexpr float const fExpected = 3463457.254;
  static constexpr double const returnExpected = 3;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto lambda = [](int i, float f) noexcept -> double {
   invoked = true;
   EXPECT_EQ(i, iExpected);
   EXPECT_EQ(f, fExpected);
   return returnExpected;
  };
  Lambda l = (decltype(lambda)&&)lambda;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l(iExpected, fExpected);
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaAssignment, c_variadic_lambda_function_move_assignment_operator_properly_initializes_lambda) {
  static constexpr int const iExpected = 2435;
  static constexpr double const dExpected = 3463457.254;
  static constexpr float const varargs[] = {3.14, 6.28, 2.71};
  static constexpr float const returnExpected = 3;

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto lambda = [](int i, double d, ...) -> float {
   CX::VaList list;
   va_start(list, d);
   invoked = true;
   EXPECT_EQ(i, iExpected);
   EXPECT_EQ(d, dExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_FLOAT_EQ((list.arg<float>()), (varargs[l]));
   }
   return returnExpected;
  };
  Lambda l = (decltype(lambda)&&)lambda;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_THROW(([&] {
   auto returned = l(
    iExpected,
    dExpected,
    varargs[0],
    varargs[1],
    varargs[2]
   );
   EXPECT_TRUE(invoked);
   EXPECT_EQ(returned, returnExpected);
  }()));
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_function_move_assignment_operator_properly_initializes_lambda) {
  static constexpr int const iExpected = 4;
  static constexpr long double const varargs[] = {1, 2, 3, 4};

  static bool invoked;

  //Reset flag in case test is re-run
  invoked = false;

  //Construct lambda and ensure it was correctly initialized
  auto lambda = [](int i, ...) noexcept {
   CX::VaList list;
   va_start(list, i);
   invoked = true;
   EXPECT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_FLOAT_EQ((list.arg<long double>()), (varargs[l]));
   }
  };
  Lambda l = (decltype(lambda)&&)lambda;
  EXPECT_TRUE(l);

  //Invoke lambda, ensure all values match expected and ensure
  //lambda was actually invoked
  EXPECT_NO_FATAL_FAILURE(([&] {
   l(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3]
   );
   EXPECT_TRUE(invoked);
  }()));
 }

 TEST(LambdaAssignment, lambda_copy_assignment_operator_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<int (float)> l1;
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1(0);
   }()),
   UninitializedLambdaError
  );

  //Copy assign to lambda and ensure it was correctly initialized
  Lambda l2 = (decltype(l1) const&)l1;
  EXPECT_FALSE(l2);
  EXPECT_THROW(
   ([&] {
    l2(0);
   }()),
   UninitializedLambdaError
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr float const fExpected = 2463987;
  static constexpr int const returnExpected = 2456;

  Lambda l3{[](float f) {
   invoked++;
   EXPECT_FLOAT_EQ(f, fExpected);
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_THROW(([&] {
   auto returned = l3(fExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Copy assign to lambda and ensure it was correctly initialized
  Lambda l4 = (decltype(l3) const&)l3;
  EXPECT_TRUE(l4);
  EXPECT_NO_THROW(([&] {
   auto returned = l4(fExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));
 }

 TEST(LambdaAssignment, noexcept_lambda_copy_assignment_operator_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void () noexcept> l1;
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1();
   }()),
   "UninitializedLambdaError"
  );

  //Copy assign to lambda and ensure it was correctly initialized
  Lambda l2 = (decltype(l1) const&)l1;
  EXPECT_FALSE(l2);
  EXPECT_DEATH(
   ([&] {
    l2();
   }()),
   "UninitializedLambdaError"
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  Lambda l3{[]() noexcept {
   invoked++;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_FATAL_FAILURE(([&] {
   l3();
   EXPECT_EQ(invoked, 1);
  }()));

  //Copy assign to lambda and ensure it was correctly initialized
  Lambda l4 = (decltype(l3) const&)l3;
  EXPECT_TRUE(l4);
  EXPECT_NO_FATAL_FAILURE(([&] {
   l4();
   EXPECT_EQ(invoked, 2);
  }()));
 }

 TEST(LambdaAssignment, c_variadic_lambda_copy_assignment_operator_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void (...)> l1;
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1(0);
   }()),
   UninitializedLambdaError
  );

  //Copy assign to lambda and ensure it was correctly initialized
  Lambda l2 = (decltype(l1) const&)l1;
  EXPECT_FALSE(l2);
  EXPECT_THROW(
   ([&] {
    l2(0);
   }()),
   UninitializedLambdaError
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr char const varargs[] = "lambda";
  static constexpr int const iExpected = ArraySize<decltype(varargs)>;
  static constexpr char const returnExpected = 'F';

  Lambda l3{[](int i, ...) -> char {
   CX::VaList list;
   va_start(list, i);
   invoked++;
   EXPECT_FLOAT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<char>() == varargs[l]));
   }
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_THROW(([&] {
   auto returned = l3(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Copy assign to lambda and ensure it was correctly initialized
  Lambda l4 = (decltype(l3) const&)l3;
  EXPECT_TRUE(l4);
  EXPECT_NO_THROW(([&] {
   auto returned = l4(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_copy_assignment_operator_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void (...) noexcept> l1;
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1(0);
   }()),
   "UninitializedLambdaError"
  );

  //Copy assign to lambda and ensure it was correctly initialized
  Lambda l2 = (decltype(l1) const&)l1;
  EXPECT_FALSE(l2);
  EXPECT_DEATH(
   ([&] {
    l2(0);
   }()),
   "UninitializedLambdaError"
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr char const varargs[] = "lambda";
  static constexpr int const iExpected = ArraySize<decltype(varargs)>;
  static constexpr char const returnExpected = 'F';

  Lambda l3{[](int i, ...) noexcept -> char {
   CX::VaList list;
   va_start(list, i);
   invoked++;
   EXPECT_FLOAT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<char>() == varargs[l]));
   }
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l3(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Copy assign to lambda and ensure it was correctly initialized
  Lambda l4 = (decltype(l3) const&)l3;
  EXPECT_TRUE(l4);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l4(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));
 }

 TEST(LambdaAssignment, lambda_move_assignment_operator_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void ()> l1;
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1();
   }()),
   UninitializedLambdaError
  );

  //Move assign to lambda and ensure it was correctly initialized
  Lambda l2 = (decltype(l1)&&)l1;
  EXPECT_FALSE(l2);
  EXPECT_THROW(
   ([&] {
    l2();
   }()),
   UninitializedLambdaError
  );

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1();
   }()),
   UninitializedLambdaError
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr float const fExpected = 2463987;
  static constexpr int const returnExpected = 2456;

  Lambda l3{[](float f) {
   invoked++;
   EXPECT_FLOAT_EQ(f, fExpected);
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_THROW(([&] {
   auto returned = l3(fExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Move assign to lambda and ensure it was correctly initialized
  Lambda l4 = (decltype(l3)&&)l3;
  EXPECT_TRUE(l4);
  EXPECT_NO_THROW(([&] {
   auto returned = l4(fExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l3);
  EXPECT_THROW(
   ([&] {
    l3(1254);
   }()),
   UninitializedLambdaError
  );
  EXPECT_EQ(invoked, 2);
 }

 TEST(LambdaAssignment, noexcept_lambda_move_assignment_operator_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void () noexcept> l1;
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1();
   }()),
   "UninitializedLambdaError"
  );

  //Move assign to lambda and ensure it was correctly initialized
  Lambda l2 = (decltype(l1)&&)l1;
  EXPECT_FALSE(l2);
  EXPECT_DEATH(
   ([&] {
    l2();
   }()),
   "UninitializedLambdaError"
  );

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1();
   }()),
   "UninitializedLambdaError"
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr float const fExpected = 2463987;
  static constexpr long double const dExpected = 932487.827365;
  static constexpr int const returnExpected = -1;

  Lambda l3{[](float f, long double d) noexcept {
   invoked++;
   EXPECT_FLOAT_EQ(f, fExpected);
   EXPECT_DOUBLE_EQ(d, dExpected);
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l3(fExpected, dExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Move assign to lambda and ensure it was correctly initialized
  Lambda l4 = (decltype(l3)&&)l3;
  EXPECT_TRUE(l4);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l4(fExpected, dExpected);
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l3);
  EXPECT_DEATH(
   ([&] {
    l3(-1, 1);
   }()),
   "UninitializedLambdaError"
  );
  EXPECT_EQ(invoked, 2);
 }

 TEST(LambdaAssignment, c_variadic_lambda_move_assignment_operator_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void (...)> l1;
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1(0);
   }()),
   UninitializedLambdaError
  );

  //Move assign to lambda and ensure it was correctly initialized
  Lambda l2 = (decltype(l1)&&)l1;
  EXPECT_FALSE(l2);
  EXPECT_THROW(
   ([&] {
    l2(0);
   }()),
   UninitializedLambdaError
  );

  //Ensure moved lambda is uninitialized
  EXPECT_FALSE(l1);
  EXPECT_THROW(
   ([&] {
    l1(0);
   }()),
   UninitializedLambdaError
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Construct lambda and ensure it was correctly initialized
  static constexpr char const varargs[] = "lambda";
  static constexpr int const iExpected = ArraySize<decltype(varargs)>;
  static constexpr char const returnExpected = 'F';

  Lambda l3{[](int i, ...) -> char {
   CX::VaList list;
   va_start(list, i);
   invoked++;
   EXPECT_FLOAT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<char>() == varargs[l]));
   }
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_THROW(([&] {
   auto returned = l3(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Move assign to lambda and ensure it was correctly initialized
  Lambda l4 = (decltype(l3)&&)l3;
  EXPECT_TRUE(l4);
  EXPECT_NO_THROW(([&] {
   auto returned = l4(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));

  //Ensure moved lambda is uninitialized
  EXPECT_THROW(
   ([&] {
    l3(0);
   }()),
   UninitializedLambdaError
  );
 }

 TEST(LambdaAssignment, noexcept_c_variadic_lambda_move_assignment_operator_properly_initializes_lambda) {
  //Test with uninitialized lambda
  //Construct empty lambda and ensure it was correctly initialized
  Lambda<void (...) noexcept> l1;
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1(0);
   }()),
   "UninitializedLambdaError"
  );

  //Move assign to lambda and ensure it was correctly initialized
  Lambda l2 = (decltype(l1)&&)l1;
  EXPECT_FALSE(l2);
  EXPECT_DEATH(
   ([&] {
    l2(0);
   }()),
   "UninitializedLambdaError"
  );

  //Ensure moved lambda is unitialized
  EXPECT_FALSE(l1);
  EXPECT_DEATH(
   ([&] {
    l1(0);
   }()),
   "UninitializedLambdaError"
  );

  //Test with initialized lambda
  static int invoked;

  //Reset counter in case test is re-run
  invoked = 0;

  //Move lambda and ensure it was correctly initialized
  static constexpr char const varargs[] = "lambda";
  static constexpr int const iExpected = ArraySize<decltype(varargs)>;
  static constexpr char const returnExpected = 'F';

  Lambda l3{[](int i, ...) noexcept -> char {
   CX::VaList list;
   va_start(list, i);
   invoked++;
   EXPECT_FLOAT_EQ(i, iExpected);
   for (unsigned long l = 0; l < ArraySize<decltype(varargs)>; l++) {
    EXPECT_TRUE((list.arg<char>() == varargs[l]));
   }
   return returnExpected;
  }};
  EXPECT_TRUE(l3);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l3(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 1);
  }()));

  //Move assign to lambda and ensure it was correctly initialized
  Lambda l4 = (decltype(l3)&&)l3;
  EXPECT_TRUE(l4);
  EXPECT_NO_FATAL_FAILURE(([&] {
   auto returned = l4(
    iExpected,
    varargs[0],
    varargs[1],
    varargs[2],
    varargs[3],
    varargs[4],
    varargs[5],
    varargs[6]
   );
   EXPECT_EQ(returned, returnExpected);
   EXPECT_EQ(invoked, 2);
  }()));

  //Ensure moved lambda is uninitialized
  EXPECT_DEATH(
   ([&] {
    l3(0);
   }()),
   "UninitializedLambdaError"
  );
 }

 //TODO
 // - AllocLambda tests
 // - Lambda <-> AllocLambda conversion tests
 // - [Lambda, AllocLambda] implicit conversion tests
 // - [Lambda, AllocLambda] noexcept-qualified lambda inst/lambda/fptr assignment to
 //   unqualified lambda tests
 // - Lambda construction with types that are:
 //   - default constructible, copy-assignable
 //   - default constructible, move-assignable
 // - C-Variadic lambda conversion to fptr tests
 // - C-Variadic lambda unsafe conversion to fptr tests
}
