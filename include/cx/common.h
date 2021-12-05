#pragma once

//Globally disable useless MSVC warnings
#ifdef CX_COMPILER_MSVC
 //unreferenced inline function has been removed
 #pragma warning(disable : 4514)
 //nonstandard extension used: nameless struct/union
 #pragma warning(disable : 4201)

 //TODO remove this once all of the compiler-specific attributes have
 //been normalized
 //attribute '...' is not recognized
 #pragma warning(disable : 5030)

 //nonstandard extension used: zero-sized arrahy in struct/union
 #pragma warning(disable : 4200)
 //assignment operator was implicitly defined as deleted
 #pragma warning(disable : 4626)
 //move assignment operator was implicitly defined as deleted
 #pragma warning(disable : 5027)
#endif

//Define portable macro to emit pragma messages
#if defined(CX_COMPILER_CLANG_LIKE) || defined(CX_COMPILER_GCC)
 //Clang-like and GCC impl
 #define CX_PRAGMA_IMPL(prgma) \
 _Pragma(#prgma)
 //Equivalent to `#pragma message(#msg)`
 #define CX_PRAGMA_MSG(msg) \
 CX_PRAGMA_IMPL(message #msg)
#elif defined(CX_COMPILER_INTEL)
 //Intel impl
 #define CX_PRAGMA_IMPL(...) \
 #error "CX_PRAGMA_IMPL" on Intel is not yet implemented.
 #define CX_PRAGMA_MSG(...) \
 #error "CX_PRAGMA_MSG" on Intel is not yet implemented.
#elif defined(CX_COMPILER_MSVC)
 //MSVC impl
 #define CX_PRAGMA_IMPL(prgma) \
 _Pragma(#prgma)
 //Equivalent to `#pragma message(#msg)`
 #define CX_PRAGMA_MSG(msg) \
 CX_PRAGMA_IMPL(message (#msg))
#else
 //Unknown compiler, emit error
 #error \
  "CX_PRAGMA_IMPL" and "CX_PRAGMA_MSG" are not implemented for this compiler.
#endif

//Define a macro to emit debug messages during compile time
#ifdef CX_DEBUG
 #if defined(CX_COMPILER_CLANG_LIKE) || defined(CX_COMPILER_GCC)
  //Clang-like and GCC impl
  #define CX_DEBUG_MSG(msg) \
  CX_PRAGMA_MSG(msg)
 #elif defined(CX_COMPILER_INTEL)
  //Intel impl
  #define CX_DEBUG_MSG(...) \
  #error "CX_DEBUG_MSG(...)" on Intel is not yet implemented.
 #elif defined(CX_COMPILER_MSVC)
  //MSVC impl
  #define CX_DEBUG_MSG(msg) \
  CX_PRAGMA_MSG(msg)
 #else
  //Unknown compiler, emit error
  #error "CX_DEBUG_MSG" is not implemented for this compiler.
 #endif
#else
 //Stub definition when not building in debug mode
 #define CX_DEBUG_MSG(...)
#endif

//Implementation defined size type; equivalent to `std::size_t`
//Note: Must be defined before `std::align_val_t`
namespace CX {
 using SizeType = decltype(sizeof(0));
}

//Import or define `std::align_val_t` for differentiation of
//alignment-overloaded memory management functions.
//Note: Must include <new> before disabling exceptions, since STL headers
//depend on exceptions.
#ifdef CX_STL_SUPPORT
 #include <new>
#else
 namespace std {
  enum struct align_val_t : CX::SizeType {};
 }
#endif

//Import or define `std::construct_at` for to enable use of placement new in
//constant-evaluated contexts.
//Note: Must include <memory> before disabling exceptions, since STL headers
//depend on exceptions.
#ifdef CX_STL_SUPPORT
 #include <memory>
#else
 namespace std {
  template<typename T, typename... Args>
  constexpr T * construct_at(T * p, Args... args) noexcept {
   auto vp = const_cast<void *>(static_cast<const volatile void *>(p));
   return ::new (vp) T{(Args)args...};
  }
 }
#endif

//Be as irritating as possible to strongly discourage the use of exceptions
#ifndef CX_NO_BELLIGERENT_ERRORS
 #define CX_ERROR_EXCEPTIONS_ARE_BAD \
 /*Print relevant message*/\
 CX_PRAGMA_MSG((\
  Uh oh, it looks like you are trying to use exceptions! Consider using the\
  error-as-a-value pattern instead; it is faster, safer and, most importatly,\
  portable! Learn more at: [TODO ADD CX DOCS URL HERE].\
 ))\
 /*Cause compiler error*/\
 static_assert(!"Exceptions are prohibited.");

 //Disable clang warnings about macros shadowing keywords
 #ifdef CX_COMPILER_CLANG_LIKE
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wkeyword-macro"
 #endif

 //Shadow keywords related to exceptions with macros that expand to compiler
 //errors
 #define throw CX_ERROR_EXCEPTIONS_ARE_BAD
 #define try CX_ERROR_EXCEPTIONS_ARE_BAD
 #define catch CX_ERROR_EXCEPTIONS_ARE_BAD
 #define finally CX_ERROR_EXCEPTIONS_ARE_BAD

 //Re-enable clang warnings
 #ifdef CX_COMPILER_CLANG_LIKE
  #pragma GCC diagnostic pop
 #endif
#else
 #ifndef CX_NO_BELLIGERENT_REMINDERS
  //Repeatedly remind developers that CX is incompatible with exceptions
  CX_PRAGMA_MSG((
   CX is incompatible with exceptions and will ignore any and all exceptions
   thrown during the lifetime of your application!
  ))
 #endif
#endif

//Conditional identity macro that depends on STL support
#ifdef CX_STL_SUPPORT
 #define CX_STL_SUPPORT_EXPR(expr) expr
#else
 #define CX_STL_SUPPORT_EXPR(...)
#endif

//Substitutes for stl equivalents and additions for missing
//constructs
namespace CX {
 //Value producer meta-function with no external linkage; useful
 //for both SFINAE and concept meta-functions
 //Note: Can only be used in unevaluated contexts
 template<typename T>
 T&& declval() noexcept(true);

 //Value consumer meta-function with no external linkage; useful
 //for both SFINAE and concept meta-functions
 //Note: Can only be used in unevaluated contexts
 template<typename T>
 void expect(T) noexcept(true);

 //Type-pack `void` alias; useful for both SFINAE and concept style
 //meta-functions
 template<typename...>
 using VoidT = void;

 //Value-pack `void` alias; useful for both SFINAE and concept
 //style meta-functions
 template<auto...>
 using VoidA = void;

 //Meta-function truthy base
 struct TrueType {
  static constexpr auto const Value = true;
 };

 //Meta-function falsy base
 struct FalseType {
  static constexpr auto const Value = false;
 };

 //Implementation defined alignment type
 struct AlignType final {
 private:
  std::align_val_t value;

 public:
  constexpr AlignType() noexcept = default;
  constexpr ~AlignType() noexcept = default;

  //Accept any type that is convertible to `SizeType`
  template<typename T>
  requires (requires (T t) {
   (SizeType)t;
  })
  constexpr AlignType(T t) noexcept :
   value{(SizeType)t}
  {}

  //Implicit conversion to `SizeType`
  constexpr operator SizeType() const noexcept {
   return (SizeType)value;
  }

  //Explicit conversion to `std::align_val_t` for aligned-allocation overloads
  explicit constexpr operator std::align_val_t() const noexcept {
   return value;
  }
 };

 //Implementation defined nullptr type
 using NullptrType = decltype(nullptr);

 //Converts type and value meta-functions to stl-compatible equivalents
 namespace MetaFunctions {
  template<typename T>
  struct AsStlCompatible;

  //Type meta-function specialization
  template<typename T>
  requires (requires (VoidT<typename T::Type> * a) { a = nullptr; })
  struct AsStlCompatible<T> {
   using type = typename T::Type;
  };

  //Value meta-function specialization
  template<typename T>
  requires (requires { expect(&T::Value); })
  struct AsStlCompatible<T> {
   static constexpr auto const value = T::Value;
  };
 }

 //Converts CX-style type and value meta-functions to STL-style type and value
 //meta-functions
 template<typename T>
 using AsStlCompatible = MetaFunctions::AsStlCompatible<T>;

 //General purpose dummy template class for nested template argument deduction
 template<typename...>
 struct Dummy {};

 //General purpose dummy template template class
 template<template<typename...> typename...>
 struct DummyTemplate {};

 //General purpose dummy non-type template class
 template<auto...>
 struct DummyValueTemplate {};

 //Unique internal type with no-linkage, used for template meta-functions
 template<typename...>
 struct ImpossibleType;

 //Unique internal type with no-linkage, used for template meta-functions
 template<template<typename...> typename...>
 struct ImpossibleTemplateType;

 //Unique internal type with no-linkage, used for template meta-functions
 template<auto...>
 struct ImpossibleValueTemplateType;

 //Zero-sized utility type
 struct Never {
 private:
  static constexpr auto size
   #ifdef CX_COMPILER_CLANG_LIKE
    //Note: There is an optimizer bug in clang > 10.0.1 that causes
    //miscompilations of zla struct members.
    //See here: https://bugs.llvm.org/show_bug.cgi?id=51120
    = 1;
   #else
    = 0;
   #endif
  unsigned char _[size]{};
  static_assert(&Never::_);

 public:
  constexpr Never() noexcept = default;
  template<typename... Args>
  constexpr Never(Args...) noexcept {}
  constexpr ~Never() noexcept = default;
 };

 //Returns `true` if invoked in a constant-evaluated expression
 constexpr bool isConstexpr() noexcept {
  return __builtin_is_constant_evaluated();
 }
}
