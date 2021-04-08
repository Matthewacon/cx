#include <cx/test/common/common.h>

#include <cx/lambda.h>

namespace CX {
 TEST(IsLambda, lambda_types_satisfy_constraint) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(IsLambda, std_function_satisfies_constraint) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(IsLambda, non_lambda_types_do_not_satisfy_constraint) {
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(CompatibleLambda, qualified_lambda_to_unqualified_lambda_satisfies_constraint) {
  //TODO the following cases should be true
  // Lambda<void () noexcept> -> Lambda<void ()>
  // AllocLambda<void () const noexcept> -> AllocLambda<void ()>
  // Lambda<void () noexcept> -> AllocLambda<void ()>
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(CompatibleLambda, unqualified_lambda_to_qualified_lambda_does_not_satisfy_constraint) {
  //TODO invert cases from true-equivalent test
  throw std::runtime_error{"Unimplemented"};
 }

 TEST(CompatibleLambda, alloc_lambda_conversion_to_and_from_no_alloc_lambda_satisfies_constraint) {
  throw std::runtime_error{"Unimplemented"};
 }
}
