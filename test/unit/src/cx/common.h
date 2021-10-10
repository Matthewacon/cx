#include <cx/test/common/common.h>

#include <cx/common.h>

namespace CX {
 TEST(isConstexpr, non_constexpr_contexts_yield_false) {
  EXPECT_FALSE((isConstexpr()));
 }

 TEST(isConstexpr, constexpr_contexts_yield_true) {
  constexpr auto value = isConstexpr();
  EXPECT_TRUE(value);
 }
}
