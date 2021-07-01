#pragma once

#include <cx/common.h>
#include <cx/idioms.h>

#ifdef CX_STL_SUPPORT
 //Conditional STL dependencies
 #include <exception>
 #include <cstdio>
#elif defined(CX_LIBC_SUPPORT)
 //Conditional libc dependencies
 #include <cstdlib>
 #include <cstdio>
#endif

//Conditional support for `constinit` statements.
//GCC does not support this yet
//TODO test with MSVC and Intel
#ifdef CX_COMPILER_CLANG_LIKE
 #define CX_CONSTINIT constinit
#else
 #define CX_CONSTINIT
#endif

namespace CX {
 //CX error base
 struct CXError CX_STL_SUPPORT_EXPR(: std::exception) {
  char const * message;

  CXError(char const * message = "") :
   message(message)
  {}

  virtual char const * what() const noexcept CX_STL_SUPPORT_EXPR(override) {
   return message;
  };

  virtual ~CXError() = default;
 };

 //Utilities for universal exit function (see below)
 namespace Internal {
  #if defined(CX_STL_SUPPORT) || defined(CX_LIBC_SUPPORT)
   inline void printError(char const * funcName, CXError const &err) {
    char const
     * msg = err.what(),
     * fmt;
    #ifdef CX_COMPILER_MSVC
     #pragma warning(push)
     #pragma warning(disable : 4774)
    #endif
    if (!msg) {
     fmt = "\"CX::%s(...)\" invoked without an error\n";
    } else {
     fmt = "\"CX::%s(...)\" invoked with error:\n%s\n";
    }
    fprintf(stderr, fmt, funcName, msg);
    #ifdef CX_COMPILER_MSVC
     #pragma warning(pop)
    #endif
   }
  #endif
 }

 //User-defined exit; for platforms that do not have
 //STL or libc support, a user-defined exit function is
 //required to handle runtime errors
 void userDefinedExit(CXError const&);

 //Returns the default exit handler
 inline constexpr auto defaultExitHandler() noexcept {
  #ifdef CX_STL_SUPPORT
   //STL exit handler
   CX_DEBUG_MSG("CX_STL_SUPPORT" enabled; using STL exit handler)
   return +[](CXError const &err) {
    Internal::printError("exit", err);
    std::terminate();
   };
  #elif defined(CX_LIBC_SUPPORT)
   //libc exit handler
   CX_DEBUG_MSG("CX_LIBC_SUPPORT" enabled; using libc exit handler)
   return +[](CXError const &err) {
    Internal::printError("exit", err);
    abort();
   };
  #else
   //User-defined exit handler
   CX_DEBUG_MSG((
    Neither "CX_STL_SUPPORT" nor "CX_LIBC_SUPPORT" are enabled; \
    using user-defined exit handler. Note: If you have not defined \
    an implementation of `void CX::userDefinedExit(CXError const&)`, \
    you will encounter linker errors.
   ))
   return userDefinedExit;
  #endif
 }

 //Returns the current CX exit handler function
 inline auto& getExitHandler() noexcept {
  //The CX exit handler
  static CX_CONSTINIT thread_local void (*handler)(CXError const&) = defaultExitHandler();
  return handler;
 }

 //Sets the CX exit handler function
 void setExitHandler(StaticFunction<void, CXError const&> auto f) noexcept {
  getExitHandler() = f;
 }

 //Universal exit function
 template<typename Error = NullptrType>
 [[noreturn]]
 void exit(Error err = nullptr) noexcept {
  auto const exitFunc = getExitHandler();
  while (true) {
   if constexpr (HasBase<decltype(err), CXError>) {
    //Propagate user-provided error
    exitFunc(err);
   } else {
    //Exit with default error
    CXError abruptExit{nullptr};
    exitFunc(abruptExit);
   }
  }
 }

 //Returns the default error handler
 inline auto defaultErrorHandler() noexcept {
  #if defined(CX_STL_SUPPORT) && defined(__cpp_exceptions)
   //STL error handler
   CX_DEBUG_MSG("CX_STL_SUPPORT" enabled; using STL error handler)
   return +[](CXError const &err) {
    throw err;
   };
  #elif defined(CX_LIBC_SUPPORT)
   //libc error handler
   CX_DEBUG_MSG(
    "CX_LIBC_SUPPORT" enabled or "__cpp_exceptions" disabled; using \
    LIBC error handler
   )
   return +[](CXError const &err) noexcept {
    Internal::printError("error", err);
    abort();
   };
  #else
   //If neither `CX_STL_SUPPORT` nor `CX_LIBC_SUPPORT` are
   //enabled, use exit
   CX_DEBUG_MSG(
    Neither "CX_STL_SUPPORT" nor "CX_LIBC_SUPPORT" are enabled; \
    using exit handler instead. Note: You can change this behaviour \
    by setting the error handler for the current thread with \
    `CX::setErrorHandler(...)`
   )
   return getExitHandler();
  #endif
 }

 //Return the current CX error handler
 inline auto& getErrorHandler() noexcept {
  static thread_local void (*handler)(CXError const&) = defaultErrorHandler();
  return handler;
 }

 //Set the CX error handler
 void setErrorHandler(StaticFunction<void, CXError const&> auto f) noexcept {
  getErrorHandler() = f;
 }

 //Universal error function
 //Note: Return type present to prevent compiler errors for
 //`error` invocations in non-returning contexts
 #if defined(CX_COMPILER_CLANG_LIKE) || defined(CX_COMPILER_GCC)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wreturn-type"
 #elif defined(CX_COMPILER_MSVC)
  #pragma warning(push)
  #pragma warning(disable : 4716)
 #endif
 template<typename R = void>
 R error(auto err) {
  getErrorHandler()(err);
 }
 #if defined(CX_COMPILER_CLANG_LIKE) || defined(CX_COMPILER_GCC)
  #pragma GCC diagnostic pop
 #elif defined(CX_COMPILER_MSVC)
  #pragma warning(pop)
 #endif
}

//Clean up internal macros
#undef CX_CONSTINIT
