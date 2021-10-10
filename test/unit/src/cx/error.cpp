#include <cx/test/common/common.h>

#include <cx/error.h>

//TODO constexpr tests
namespace CX {
 TEST(IsError, custom_error_type_with_describe_member_satisfies_constraint) {
  struct CustomError final {
   constexpr char const * describe() const noexcept {
    return "";
   }
  };
  EXPECT_TRUE((IsError<CustomError>));
 }

 TEST(IsError, invalid_error_types_do_not_satisfy_constraint) {
  struct InvalidError {};
  EXPECT_FALSE((IsError<InvalidError>));
  EXPECT_FALSE((IsError<void *>));
  EXPECT_FALSE((IsError<float>));
 }

 #if !(defined(CX_ERROR_MSG) || defined(CX_ERROR_TRACE))
  TEST(Error, error_without_debug_features_is_zero_size) {
   EXPECT_EQ(sizeof(Error), 0);
  }
 #endif //!(defined(CX_ERROR_MSG) || defined(CX_ERROR_TRACE))

 //Error message behaviour testing
 #ifdef CX_ERROR_MSG
  //Tests when `CX_ERROR_MSG` is enabled
  TEST(Error, error_message_constructor_retains_message) {
   static constexpr char expectedMessage[] = "expected message";
   Error e{expectedMessage};
   EXPECT_STREQ(e.describe(), expectedMessage);
  }

  TEST(Error, error_message_and_cause_constructor_retains_message) {
   static constexpr char expectedMessage[] = "woah something happened!";
   Error cause{expectedMessage};
   Error e{expectedMessage, (Error const&)cause};
   EXPECT_STREQ(e.describe(), expectedMessage);
  }

  TEST(Error, error_copy_constructor_copies_message) {
   static constexpr char expectedMessage[] = "badda bing badda boom";
   Error toCopy{expectedMessage};
   EXPECT_STREQ(toCopy.describe(), expectedMessage);
   Error copied{(Error const&)toCopy};
   EXPECT_STREQ(copied.describe(), expectedMessage);
  }

  TEST(Error, error_move_constructor_moves_message) {
   static constexpr char expectedMessage[] = "beam me up scotty";
   Error toMove{expectedMessage};
   EXPECT_STREQ(toMove.describe(), expectedMessage);
   Error moved{(Error&&)toMove};
   EXPECT_EQ(toMove.describe(), nullptr);
   EXPECT_STREQ(moved.describe(), expectedMessage);
  }

  TEST(Error, error_copy_assignment_operator_copies_message) {
   static constexpr char expectedMessage[]
    = "relentless in the pursuit of affordable quality";
   Error toCopy{expectedMessage};
   EXPECT_STREQ(toCopy.describe(), expectedMessage);
   Error copied{};
   copied = (Error const&)toCopy;
   EXPECT_STREQ(copied.describe(), expectedMessage);
  }

  TEST(Error, error_move_assignment_operator_moves_message) {
   static constexpr char expectedMessage[]
    = "this is your brain on drugs";
   Error toMove{expectedMessage};
   EXPECT_STREQ(toMove.describe(), expectedMessage);
   Error moved{};
   moved = (Error&&)toMove;
   EXPECT_EQ(toMove.describe(), nullptr);
   EXPECT_STREQ(moved.describe(), expectedMessage);
  }

  TEST(Error, error_like_copy_assignment_operator_copies_message) {
   static constexpr char expectedMessage[] = "lets say hypothetically";
   struct ErrorLike {
    constexpr char const * describe() const noexcept {
     return expectedMessage;
    }
   };
   EXPECT_TRUE((IsError<ErrorLike>));
   ErrorLike toCopy;
   Error copied{(ErrorLike const&)toCopy};
   EXPECT_STREQ(copied.describe(), expectedMessage);
  }
 #else
  //Tests when `CX_ERROR_MSG` is disabled
  TEST(Error, error_message_constructor_has_no_effect_on_message) {
   static constexpr char const * expected = nullptr;
   Error e{"This should not be propagated"};
   EXPECT_EQ(e.describe(), expected);
  }

  TEST(Error, error_message_and_cause_constructor_has_no_effect_on_message) {
   static constexpr char const * expected = nullptr;
   Error cause;
   Error e{"whoops, something went wrong!", cause};
   EXPECT_EQ(e.describe(), expected);
  }

  TEST(Error, error_copy_constructor_has_no_effect_on_message) {
   static constexpr char const * expected = nullptr;
   Error toCopy;
   Error e{(Error const&)toCopy};
   EXPECT_EQ(e.describe(), expected);
  }

  TEST(Error, error_move_constructor_has_no_effect_on_message) {
   static constexpr char const * expected = nullptr;
   Error toMove;
   Error e{(Error&&)toMove};
   EXPECT_EQ(e.describe(), expected);
  }

  TEST(Error, error_copy_assignment_operator_has_no_effect_on_message) {
   static constexpr char const * expected = nullptr;
   Error toCopy;
   Error e;
   e = (Error const&)toCopy;
   EXPECT_EQ(e.describe(), expected);
  }

  TEST(Error, error_move_assignment_operator_has_no_effect_on_message) {
   static constexpr char const * expected = nullptr;
   Error toMove;
   Error e;
   e = (Error&&)toMove;
   EXPECT_EQ(e.describe(), expected);
  }

  TEST(Error, error_like_copy_assignment_operator_has_no_effect_on_message) {
   struct ErrorLike {
    constexpr char const * describe() const noexcept {
     return "";
    }
   };
   static constexpr char const * expected = nullptr;
   ErrorLike toCopy;
   Error e{(ErrorLike const&)toCopy};
   EXPECT_EQ(e.describe(), expected);
  }
 #endif //CX_ERROR_MSG

 //Error tracing behaviour testing
 #ifdef CX_ERROR_TRACE
  //Tests when `CX_ERROR_TRACE` is enabled
  TEST(Error, error_message_and_cause_constructor_initializes_cause) {
   Error cause;
   Error e{"", cause};
   EXPECT_TRUE(e.cause());
  }

  TEST(Error, error_copy_constructor_copies_cause) {
   //Copy error with populated cause
   {
    Error cause;
    Error toCopy{"", (Error const&)cause};
    EXPECT_TRUE(toCopy.cause());
    Error e{(Error const&)toCopy};
    EXPECT_TRUE(toCopy.cause());
    EXPECT_TRUE(e.cause());
   }

   //Copy error without populated cause
   {
    Error toCopy;
    EXPECT_FALSE(toCopy.cause());
    Error e{(Error const&)toCopy};
    EXPECT_FALSE(toCopy.cause());
    EXPECT_FALSE(e.cause());
   }
  }

  TEST(Error, error_move_constructor_moves_cause) {
   //Move error with populated cause
   {
    Error cause;
    Error toMove{"", (Error const&)cause};
    EXPECT_TRUE(toMove.cause());
    Error e{(Error&&)toMove};
    EXPECT_FALSE(toMove.cause());
    EXPECT_TRUE(e.cause());
   }

   //Move error without populated cause
   {
    Error toMove;
    EXPECT_FALSE(toMove.cause());
    Error e{(Error&&)toMove};
    EXPECT_FALSE(toMove.cause());
    EXPECT_FALSE(e.cause());
   }
  }

  TEST(Error, error_like_copy_constructor_default_initializes_cause) {
   struct ErrorLike final {
    constexpr char const * describe() const noexcept {
     return "";
    }
   };
   EXPECT_TRUE((IsError<ErrorLike>));
   ErrorLike toCopy;
   Error e{(ErrorLike const&)toCopy};
   EXPECT_FALSE(e.cause());
  }

  TEST(Error, error_copy_assignment_operator_copies_cause) {
   //Copy error with populated cause
   {
    Error cause;
    Error toCopy{"", (Error const&)cause};
    EXPECT_TRUE(toCopy.cause());
    Error e;
    e = (Error const&)toCopy;
    EXPECT_TRUE(toCopy.cause());
    EXPECT_TRUE(e.cause());
   }

   //Copy error without populated cause
   {
    Error toCopy;
    EXPECT_FALSE(toCopy.cause());
    Error e;
    e = (Error const&)toCopy;
    EXPECT_FALSE(toCopy.cause());
    EXPECT_FALSE(e.cause());
   }
  }

  TEST(Error, error_move_assignment_operator_moves_cause) {
   //Move error with populated cause
   {
    Error cause;
    Error toMove{"", (Error const&)cause};
    EXPECT_TRUE(toMove.cause());
    Error e;
    e = (Error&&)toMove;
    EXPECT_FALSE(toMove.cause());
    EXPECT_TRUE(e.cause());
   }

   //Move error without populated cause
   {
    Error toMove;
    EXPECT_FALSE(toMove.cause());
    Error e;
    e = (Error&&)toMove;
    EXPECT_FALSE(toMove.cause());
    EXPECT_FALSE(e.cause());
   }
  }

  TEST(Error, error_like_copy_assignment_operator_default_initalizes_cause) {
   struct ErrorLike final {
    constexpr char const * describe() const noexcept {
     return "";
    }
   };
   EXPECT_TRUE(IsError<ErrorLike>);
   ErrorLike toCopy;
   Error e;
   e = (ErrorLike const&)toCopy;
   EXPECT_FALSE(e.cause());
  }
 #else
  //Tests when `CX_ERROR_TRACE` is disabled
  TEST(Error, error_message_and_cause_constructor_has_no_effect_on_cause) {
   struct ErrorLike final {
    constexpr char const * describe() const noexcept {
     return "";
    }
   };
   EXPECT_TRUE(IsError<ErrorLike>);
   ErrorLike cause;
   Error e{"", (ErrorLike const&)cause};
   EXPECT_FALSE(e.cause());
  }

  TEST(Error, error_copy_constructor_has_no_effect_on_cause) {
   Error cause;
   Error toCopy{"", (Error const&)cause};
   EXPECT_FALSE(toCopy.cause());
   Error e{(Error const&)toCopy};
   EXPECT_FALSE(toCopy.cause());
  }

  TEST(Error, error_move_constructor_has_no_effect_on_cause) {
   Error cause;
   Error toMove{"", (Error&&)cause};
   EXPECT_FALSE(toMove.cause());
   Error e{(Error&&)toMove};
   EXPECT_FALSE(e.cause());
  }

  TEST(Error, error_like_copy_constructor_has_no_effect_on_cause) {
   struct ErrorLike final {
    constexpr char const * describe() const noexcept {
     return "";
    }
   };
   EXPECT_TRUE(IsError<ErrorLike>);
   ErrorLike toCopy;
   Error e{(ErrorLike const&)toCopy};
   EXPECT_FALSE(e.cause());
  }

  TEST(Error, error_copy_assignment_operator_has_no_effect_on_cause) {
   Error cause;
   Error toCopy{"", (Error const&)cause};
   EXPECT_FALSE(toCopy.cause());
   Error e;
   e = (Error const&)toCopy;
   EXPECT_FALSE(e.cause());
  }

  TEST(Error, error_move_assignment_operator_has_no_effect_on_cause) {
   Error cause;
   Error toMove{"", (Error const&)cause};
   EXPECT_FALSE(toMove.cause());
   Error e;
   e = (Error&&)toMove;
   EXPECT_FALSE(toMove.cause());
   EXPECT_FALSE(e.cause());
  }

  TEST(Error, error_like_copy_assignment_operator_has_no_effect_on_cause) {
   struct ErrorLike final {
    constexpr char const * describe() const noexcept {
     return "";
    }
   };
   EXPECT_TRUE(IsError<ErrorLike>);
   ErrorLike toCopy;
   Error e;
   e = (ErrorLike const&)toCopy;
   EXPECT_FALSE(e.cause());
  }
 #endif //CX_ERROR_TRACE
}
