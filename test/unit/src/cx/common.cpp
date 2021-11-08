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

 TEST(std__construct_at, construct_at_initializes_stack_allocated_structures) {
  FAIL();
 }

 TEST(std__construct_at, construct_at_initializes_heap_allocated_structures) {
  FAIL();
 }

 TEST(std__construct_at, construct_at_initializes_statically_allocated_structures) {
  FAIL();
 }
}
