#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>

using namespace ::testing;

namespace CX {
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
}
