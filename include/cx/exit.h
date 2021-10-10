#pragma once

#include <cx/common.h>
#include <cx/error.h>

//Flags to configure default exit handler behaviour
#ifndef CX_EXIT_HANDLER
 #if defined(CX_STL_SUPPORT)
  #define CX_EXIT_HANDLER 0
 #elif defined(CX_LIBC_SUPPORT)
  #define CX_EXIT_HANDLER 1
 #else
  #define CX_EXIT_HANDLER 2
 #endif //defined(CX_STL_SUPPORT)
#endif //CX_EXIT_HANDLER

//Sanitize `CX_EXIT_HANDLER` flag
#if CX_EXIT_HANDLER < 0 || CX_EXIT_HANDLER > 2
 #error `CX_EXIT_HANDLER` is set to an invalid value. The possible values are:\
  0 - Use STL exit handler;\
  1 - Use LIBC exit handler;\
  2 - Use user-defined exit handler
#endif

//Debug msg for the exit handler being used
#if CX_EXIT_HANDLER == 0
 CX_DEBUG_MSG((`CX::exit(...)` using STL exit handler))
#elif CX_EXIT_HANDLER == 1
 CX_DEBUG_MSG((`CX::exit(...)` using LIBC exit handler))
#elif CX_EXIT_HANDLER == 2
 CX_DEBUG_MSG((
  `CX::exit(...)` using user-defined exit handler. Note: If you have not\
  defined an implementation of `void CX::userDefinedExit(Error const&)`, \
  you will encounter linker errors.
 ))
#endif

//Temporarily disable exception keyword shadowing to avoid breaking STL/libc
//headers
#ifndef CX_NO_BELLIGERENT_ERRORS
 #undef throw
 #undef try
 #undef catch
 #undef finally
#endif

//Conditional STL/libc dependencies
#ifdef CX_STL_SUPPORT
 //Conditional STL dependencies
 #include <exception>
 #include <cstdio>
#elif defined(CX_LIBC_SUPPORT)
 //Conditional libc dependencies
 #include <cstdlib>
 #include <cstdio>
#endif

//Re-enable exception keyword shadowing
#ifndef CX_NO_BELLIGERENT_ERRORS
 //Disable clang warnings about macros shadowing keywords
 #ifdef CX_COMPILER_CLANG_LIKE
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wkeyword-macro"
 #endif

 //Re-define macros to shadow keywords related to exception handling
 #define throw CX_ERROR_EXCEPTIONS_ARE_BAD
 #define try CX_ERROR_EXCEPTIONS_ARE_BAD
 #define catch CX_ERROR_EXCEPTIONS_ARE_BAD
 #define finally CX_ERROR_EXCEPTIONS_ARE_BAD

 //Pop diagnostic context
 #ifdef CX_COMPILER_CLANG_LIKE
  #pragma GCC diagnostic pop
 #endif
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
 /*
 //Utilities for universal exit function (see below)
 namespace Internal {
  #if defined(CX_STL_SUPPORT) || defined(CX_LIBC_SUPPORT)
   inline void printError(char const * funcName, Error const &err) {
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
 */

 struct NoneExitError final {
  constexpr char const * describe() const noexcept {
   return "Exited without an error.";
  }
 };
 static_assert(IsError<NoneExitError>);

 //User-defined exit; for platforms that do not have
 //STL or libc support, a user-defined exit function is
 //required to handle runtime errors
 void userDefinedExit(Error const&);

 //Returns the default exit handler
 inline constexpr auto defaultExitHandler() noexcept {
  #ifdef CX_STL_SUPPORT
   //STL exit handler
   return +[](Error const &err) {
    (void)err;
    //Internal::printError("exit", err);
    std::terminate();
   };
  #elif defined(CX_LIBC_SUPPORT)
   //libc exit handler
   CX_DEBUG_MSG("CX_LIBC_SUPPORT" enabled; using libc exit handler)
   return +[](Error const &err) {
    (void)err;
    //Internal::printError("exit", err);
    abort();
   };
  #else
   //User-defined exit handler
   return userDefinedExit;
  #endif
 }

 //Returns the current CX exit handler function
 inline auto& getExitHandler() noexcept {
  //The CX exit handler
  static CX_CONSTINIT thread_local void (*handler)(Error const&)
   = defaultExitHandler();
  return handler;
 }

 //Sets the CX exit handler function
 void setExitHandler(StaticFunction<void, Error const&> auto f) noexcept {
  getExitHandler() = f;
 }

 //Utilities for `CX::exit(...)`
 namespace Internal {
  //This function is purely for debugging errors in constant-evaluated contexts
  constexpr void exit_was_called_inside_a_constant_evaluated_context__check_your_compiler_error_trace()
  noexcept {
   while(isConstexpr());
  }
 }

 //Universal exit function
 [[noreturn]]
 constexpr void exit(Error err = NoneExitError{}) noexcept {
  Internal::exit_was_called_inside_a_constant_evaluated_context__check_your_compiler_error_trace();
  if (!isConstexpr()) {
   auto const exitFunc = getExitHandler();
   while (true) {
    exitFunc(err);
   }
  }
 }

 //Error for trying to execute constant-evaluated codepaths when
 //`CX_CONSTEXPR_SEMANTICS` is disabled
 struct NoConstexprSemanticsError final {
  constexpr auto& describe() const noexcept {
   return
    "`CX_CONSTEXPR_SEMANTICS` is disabled. To use this construct in a "
    "constant evaluated context, enable `CX_CONSTEXPR_SEMANTICS`.";
  }
 };
 static_assert(IsError<NoConstexprSemanticsError>);
}

//Clean up internal macros
#undef CX_CONSTINIT
