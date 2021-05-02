#include <cx/test/common/common.h>

#include <cx/error.h>

namespace CX {
 TEST(Exit, exit_without_error_terminates_with_default_message) {
  EXPECT_DEATH(
   exit(),
   "`CX::exit\\(\\.\\.\\.\\)` invoked without an error\n"
  );
 }

 TEST(Exit, exit_with_error_terminates_with_error_message) {
  EXPECT_DEATH(
   exit(CX::CXError{"Custom message"}),
   ".*`CX::exit\\(\\.\\.\\.\\)` invoked with error:\nCustom message\n.*"
  );
 }

 TEST(Exit, exit_with_custom_handler_invokes_custom_handler) {
  static char const * errMsg = "";

  EXPECT_NO_FATAL_FAILURE([] {
   char const * const expectedMsg = "expected message";

   setExitHandler(+[](CXError const &err) {
    errMsg = err.what();
   });

   EXPECT_NO_THROW(
    exit(CXError{expectedMsg})
   );

   //EXPECT_STREQ(errMsg, expectedMsg);
  }());
 }

 TEST(DefaultExitHandler, default_exit_handler_terminates) {
  EXPECT_DEATH(
   defaultExitHandler()(CXError{nullptr}),
   "`CX::exit\\(\\.\\.\\.\\)` invoked without an error\n"
  );
 }

 TEST(SetExitHandler, custom_exit_handler_persists) {
  EXPECT_NO_FATAL_FAILURE([] {
   auto origHandler = getExitHandler();
   setExitHandler(+[](CXError const&) {});
   EXPECT_TRUE((getExitHandler() != origHandler));
  }());
 }

 #ifdef CX_STL_SUPPORT
  TEST(Error, error_rethrows_given_error) {
   EXPECT_THROW(
    error(CXError{""}),
    CXError
   );
  }

  TEST(DefaultErrorHandler, default_error_handler_rethrows_error) {
   EXPECT_THROW(
    defaultErrorHandler()(CXError{""}),
    CXError
   );
  }
 #else
  TEST(Error, error_terminates) {
   EXPECT_DEATH(
    error(CXError{"Some error"}),
    ".*`CX::exit\\(\\.\\.\\.\\)` invoked with error:\nSome error\n.*"
   );
  }

  TEST(DefaultErrorHandler, default_error_handler_terminates) {
   EXPECT_NO_FATAL_FAILURE(
    EXPECT_DEATH(
     defaultErrorHandler()(CXError{"Some error"}),
     ".*`CX::exit\\(\\.\\.\\.\\)` invoked with error:\nSome error\n.*"
    )
   );
  }
 #endif

 TEST(Error, error_with_custom_handler_invokes_custom_handler) {
  static char const * errMsg = "";

  EXPECT_NO_FATAL_FAILURE([] {
   char const * const expectedMsg = "expected message";

   setErrorHandler(+[](CXError const &err) {
    errMsg = err.what();
   });

   EXPECT_NO_THROW(error(CXError{expectedMsg}));
  }());
 }

 TEST(SetErrorHandler, custom_error_handler_persists) {
  EXPECT_NO_FATAL_FAILURE([] {
   auto origHandler = getErrorHandler();
   setErrorHandler(+[](CXError const&) {});
   EXPECT_TRUE((getExitHandler() != origHandler));
  }());
 }
}
