#pragma once

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
  "CX_PRAGMA_IMPL" and "CX_PRAGMA_MSG" are not implemented for \
  this compiler.
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

 //Type-pack `void` alias; useful for both SFINAE and concept
 //style meta-functions
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

 //Implementation defined size type; equivalent to `std::size_t`
 using SizeType = decltype(sizeof(0));

 //Implementation defined alignment type
 using AlignType = decltype(alignof(int));

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
}
