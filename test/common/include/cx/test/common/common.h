//Pedantic disabling of exceptions does not work with gtest
#define CX_NO_BELLIGERENT_ERRORS
#define CX_NO_BELLIGERENT_REMINDERS

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>

#include <type_traits>
#include <variant>

#include <cx/templates.h>
#include <cx/exit.h>

using namespace ::testing;

namespace CX {
 //TODO get rid of these macros since we have a new error handling system
 //Utility macros for testing fault conditions
 #ifdef CX_STL_SUPPORT
  //`CX_STL_SUPPORT` exit and error behaviour
  #define EXPECT_ERROR_BEHAVIOUR(stmt, except) \
  EXPECT_THROW(stmt, except)

  #define EXPECT_NO_ERROR_BEHAVIOUR(stmt) \
  EXPECT_NO_THROW(stmt)

  #define EXPECT_EXIT_BEHAVIOUR(stmt, regex) \
  EXPECT_DEATH(stmt, regex)

  #define EXPECT_NO_EXIT_BEHAVIOUR(stmt) \
  EXPECT_NO_FATAL_FAILURE(stmt)
 #elif defined(CX_LIBC_SUPPORT)
  //`CX_LIBC_SUPPORT` exit and error behaviour
  #define EXPECT_ERROR_BEHAVIOUR(stmt, except) \
  EXPECT_DEATH(stmt, ".*")

  #define EXPECT_NO_ERROR_BEHAVIOUR(stmt) \
  EXPECT_NO_FATAL_FAILURE(stmt)

  #define EXPECT_EXIT_BEHAVIOUR(stmt, regex) \
  EXPECT_DEATH(stmt, regex)

  #define EXPECT_NO_EXIT_BEHAVIOUR(stmt) \
  EXPECT_NO_FATAL_FAILURE(stmt)
 #else
  //User-defined error and exit behaviour
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-W#warnings"
  #pragma GCC diagnostic ignored "-Wpedantic"
   #warning \
    "Neither `CX_STL_SUPPORT` nor `CX_LIBC_SUPPORT are enabled; "\
    "the behaviour of error/exit tests with user-defined " \
    "error/exit handlers is unspecified, so these tests will be "\
    "skipped."
  #pragma GCC diagnostic pop

  #define EXPECT_ERROR_BEHAVIOUR(stmt, except) \
  printf(\
   "%s:%d: Skipped\n%s\n",\
   __FILE__,\
   __LINE__,\
   "TEST SKIPPED DUE TO USER-DEFINED EXIT/ERROR BEHAVIOUR"\
  );\
  GTEST_SKIP();\
  stmt

  #define EXPECT_NO_ERROR_BEHAVIOUR(stmt) \
  EXPECT_ERROR_BEHAVIOUR(stmt, 0)

  #define EXPECT_EXIT_BEHAVIOUR(stmt, regex) \
  EXPECT_ERROR_BEHAVIOUR(stmt, 0)

  #define EXPECT_NO_EXIT_BEHAVIOUR(stmt) \
  EXPECT_ERROR_BEHAVIOUR(stmt, 0)
 #endif

 //Utility type with configurable copy-move behaviour
 template<
  bool IsCopyConstructible,
  bool IsMoveConstructible,
  bool IsCopyAssignable,
  bool IsMoveAssignable,
  typename... ConstructorArguments
 >
 struct CustomType {
 private:
  using None = CX::ImpossibleType<>;

 public:
  using VariantType = SelectType<
   (sizeof...(ConstructorArguments) > 0),
   std::variant<std::remove_reference_t<ConstructorArguments>...>,
   CX::Dummy<>
  >;

  bool
   defaultConstructed = false,
   copyConstructed    = false,
   moveConstructed    = false,
   copyAssigned       = false,
   moveAssigned       = false;
  VariantType constructorArguments;

  //Explicitly deleted copy/move constructors / assignment operators
  constexpr CustomType(CustomType const&) = delete;
  constexpr CustomType(CustomType&&) = delete;
  constexpr CustomType& operator=(CustomType const&) = delete;
  constexpr CustomType& operator=(CustomType&&) = delete;

  //Default constructor
  constexpr CustomType(ConstructorArguments... args) noexcept :
   defaultConstructed(true),
   constructorArguments{(ConstructorArguments)args...}
  {}

  //Copy constructor
  constexpr CustomType(
   SelectType<IsCopyConstructible, CustomType, None> const& other
  )
  noexcept
  requires true :
   copyConstructed(true),
   constructorArguments{other.constructorArguments}
  {}

  //Move constructor
  constexpr CustomType(
   SelectType<IsMoveConstructible, CustomType, None>&& other
  )
   noexcept
   requires true :
   moveConstructed(true),
   constructorArguments{std::move(other.constructorArguments)}
  {}

  //Default destructor
  virtual constexpr ~CustomType() = default;

  //Copy-assignment operator
  constexpr CustomType& operator=(
   SelectType<IsCopyAssignable, CustomType, None> const& other
  )
  noexcept
  requires true
  {
   copyAssigned = true;
   constructorArguments = other.constructorArguments;
   return *this;
  }

  //Move-assignment operator
  constexpr CustomType& operator=(
   SelectType<IsMoveAssignable, CustomType, None>&& other
  )
  noexcept
  requires true
  {
   moveAssigned = true;
   constructorArguments = std::move(other.constructorArguments);
   return *this;
  }
 };

 //Utility types for testing builtin operations
 using CopyConstructibleType = CustomType<true, false, false, false>;
 static_assert(
  CopyConstructible<CopyConstructibleType>
  && !CopyAssignable<CopyConstructibleType>
  && !MoveConstructible<CopyConstructibleType>
  && !MoveAssignable<CopyConstructibleType>
 );

 using MoveConstructibleType = CustomType<false, true, false, false>;
 static_assert(
  MoveConstructible<MoveConstructibleType>
  && !MoveAssignable<MoveConstructibleType>
  && !CopyConstructible<MoveConstructibleType>
  && !CopyAssignable<MoveConstructibleType>
 );

 using CopyAssignableType = CustomType<false, false, true, false>;
 static_assert(
  CopyAssignable<CopyAssignableType>
  && !CopyConstructible<CopyAssignableType>
  && !MoveConstructible<CopyAssignableType>
  && !MoveAssignable<CopyAssignableType>
 );

 using MoveAssignableType = CustomType<false, false, false, true>;
 static_assert(
  MoveAssignable<MoveAssignableType>
  && !MoveConstructible<MoveAssignableType>
  && !CopyConstructible<MoveAssignableType>
  && !CopyAssignable<MoveAssignableType>
 );

 //Utilities for constant evaluated tests
 struct ConstantEvaluatedTestFailure final {
  constexpr auto& describe() const noexcept {
   return "Test failed";
  }
 };
 static_assert(IsError<ConstantEvaluatedTestFailure>);

 //Shim to gtest's `EXPECT_NO_FATAL_FAILURE`
 #define EXPECT_NO_EXIT(stmt) \
 EXPECT_NO_FATAL_FAILURE(stmt)

 //Gtest macro shim for selecting runtime / compiletime versions of `macro`
 #define CX_GTEST_SHIM(macro, ...) \
 if (!std::is_constant_evaluated()) {\
  macro(__VA_ARGS__);\
 } else {\
  CX_GTEST__##macro(__VA_ARGS__);\
 }

 //Constant evaluated equivalent to gtest's `EXPECT_TRUE`
 #define CX_GTEST__EXPECT_TRUE(cond) \
 if (!(bool)(cond)) {\
  exit(ConstantEvaluatedTestFailure{});\
 }

 //Constant evaluated equivalent to gtest's `EXPECT_FALSE`
 #define CX_GTEST__EXPECT_FALSE(cond) \
 if ((bool)(cond)) {\
  exit(ConstantEvaluatedTestFailure{});\
 }
 
 //Utilities for `CX_GTEST__EXPECT_NO_EXIT`
 namespace Internal {
  template<auto TestBody, auto = TestBody()>
  using EnsureTestDoesNotExit = void;
 }

 //Note: Only works on clang 13+
 //Constant evaluated equivalent to `EXPECT_NO_EXIT`; does nothing
 #define CX_GTEST__EXPECT_NO_EXIT(stmt) \
 []() constexpr noexcept -> Internal::EnsureTestDoesNotExit<\
  [=]() constexpr noexcept { stmt; return 0; }\
 > {}()
}
