#include <cx/test/common/common.h>

#include <cx/option.h>

namespace CX {
 TEST(Option, unpopulated_option_with_noexcept_constructible_type_is_noexcept_constructible) {
  EXPECT_TRUE((noexcept(Option<char>{})));
 }

 TEST(Option, populated_option_with_noexcept_constructible_type_is_noexcept_constructible) {
  FAIL();
 }

 //TODO remaining `Option` tests

 //TODO `OptionTuple` tests
}
