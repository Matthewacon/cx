#include <cx/test/common/common.h>

#include <cx/memory.h>

namespace CX {
 //Note: `AllocDefer` tests require STL support to be enabled
 #ifdef CX_STL_SUPPORT
  TEST(AllocDefer, drain_invokes_deferred_function) {
   static int count;
   static constexpr auto const callback = [] {
    count++;
   };

   //Reset counter in case test is re-run
   count++;

   {
    //Construct deferral mechanism and add deferred function
    AllocDefer deferred;
    deferred += callback;

    //Drain deferral mechanism
    deferred.drain();
   }

   EXPECT_EQ(count, 1);
  }

  TEST(AllocDefer, all_deferred_functions_invoked_when_deferral_mechanism_destructed) {
   static constexpr auto const expectedCount = 12;
   static int count;
   static constexpr auto const callback = [] {
    count++;
   };

   //Reset counter in case test is re-run
   count = 0;

   {
    //Construct deferral mechanism and add deferred functions
    AllocDefer deferred;
    for (int i = 0; i < expectedCount; i++) {
     deferred += callback;
    }
   }

   EXPECT_EQ(count, expectedCount);
  }
 #endif //CX_STL_SUPPORT

 //`Defer` tests
 TEST(Defer, drain_invokes_deferred_function) {
  static int count;
  static constexpr auto const callback = [] {
   count++;
  };

  //Reset counter in case test is re-run
  count++;

  {
   //Construct deferral mechanism and add deferred function
   Defer<1> deferred;
   deferred += callback;

   //Drain deferral mechanism
   deferred.drain();
  }

  EXPECT_EQ(count, 1);
 }

 TEST(Defer, all_deferred_functions_invoked_when_deferral_mechanism_destructed) {
  static constexpr auto const expectedCount = 43;
  static int count;
  static constexpr auto const callback = [] {
   count++;
  };

  //Reset counter in case test is re-run
  count = 0;

  {
   //Construct deferral mechanism and add deferred functions
   Defer<70> deferred;
   for (int i = 0; i < expectedCount; i++) {
    deferred += callback;
   }
  }

  EXPECT_EQ(count, expectedCount);
 }
}

