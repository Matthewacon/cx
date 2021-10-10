#include <cx/test/common/common.h>

#include <cx/exit.h>

namespace CX {
 TEST(Exit, exit_without_error_terminates_with_default_message) {
  EXPECT_EXIT_BEHAVIOUR(
   exit(),
   "`CX::exit\\(\\.\\.\\.\\)` invoked without an error\n"
  );
 }

 TEST(Exit, exit_with_error_terminates_with_error_message) {
  EXPECT_EXIT_BEHAVIOUR(
   exit(CX::Error{"Custom message"}),
   ".*`CX::exit\\(\\.\\.\\.\\)` invoked with error:\nCustom message\n.*"
  );
 }

 TEST(Exit, exit_with_custom_handler_invokes_custom_handler) {
  static char const * expectedMsg = "expected message";

  EXPECT_NO_FATAL_FAILURE([] {
   setExitHandler(+[](Error const &err) {
    fprintf(stderr, "%s\n", expectedMsg);
    defaultExitHandler()(err);
   });

   EXPECT_EXIT_BEHAVIOUR(
    exit(Error{/*nullptr*/}),
    ".*expected message\n.*"
   );
  }());
 }

 TEST(DefaultExitHandler, default_exit_handler_terminates) {
  EXPECT_EXIT_BEHAVIOUR(
   defaultExitHandler()(Error{/*nullptr*/}),
   "`CX::exit\\(\\.\\.\\.\\)` invoked without an error\n"
  );
 }

 TEST(SetExitHandler, custom_exit_handler_persists) {
  EXPECT_NO_FATAL_FAILURE([] {
   auto origHandler = getExitHandler();
   setExitHandler(+[](Error const&) {});
   EXPECT_TRUE((getExitHandler() != origHandler));
  }());
 }
}
