#include <cx/test/common/common.h>

#include <cx/unsafe.h>

namespace CX {
 TEST(UnionCast, casting_between_types_of_matching_sizes_yields_expected_value) {
  static constexpr unsigned int const expectedValue = 0x12345;
  auto value = unionCast<int>(expectedValue);
  EXPECT_EQ((unsigned int)value, expectedValue);
 }

 //Note: These `MemberPtr` tests are only for Itanium C++ ABI platforms
 TEST(MemberPtr, componentizing_non_virtual_member_function_pointer_yields_non_virtual_address) {
  FAIL();
 }

 TEST(MemberPtr, componentizing_virtual_member_function_pointer_yields_virtual_address) {
  FAIL();
 }

 TEST(MemberPtr, componentizing_non_virtual_member_function_pointer_with_offset_vtable_yields_non_virtual_address_and_vtable_offset) {
  FAIL();
 }

 TEST(MemberPtr, componentizing_virtual_member_function_pointer_with_offset_vtable_yields_virtual_address_and_vtable_offset) {
  FAIL();
 }
}
