# `<cx/idioms.h>`
## Header Summary
This header provides the basis for all concepts and type / value
meta-functions for CX.

## Concepts
| Name | Description |
| :- | :- |
| [`CX::SameType`](./cx_idioms_h/same_type.md) | |
| [`CX::MatchAnyType`](./cx_idioms_h/match_any_type.md) | |
| [`CX::SameTemplateType`](./cx_idioms_h/same_template_type.md) | |
| [`CX::SameValue`](./cx_idioms_h/same_type.md) | |
| [`CX::MatchAnyValue`](./cx_idioms_h/match_any_value.md) | |
| [`CX::UniqueTypes`](./cx_idioms_h/unique_types.md) | |
| [`CX::UniqueTemplateTypes`](./cx_idioms_h/unique_template_types.md) | |
| [`CX::UniqueValues`](./cx_idioms_h/unique_values.md) | |
| [`CX::ConvertibleTo`](./cx_idioms_h/convertible_to.md) | |
| [`CX::Constructible`](./cx_idioms_h/constructible.md) | |
| [`CX::Destructible`](./cx_idioms_h/destructible.md) | |
| [`CX::CopyConstructible`](./cx_idioms_h/copy_constructible.md) | |
| [`CX::MoveConstructible`](./cx_idioms_h/move_constructible.md) | |
| [`CX::Const`](./cx_idioms_h/const.md) | |
| [`CX::Volatile`](./cx_idioms_h/volatile.md) | |
| [`CX::Array`](./cx_idioms_h/array.md) | |
| [`CX::SizedArray`](./cx_idioms_h/sized_array.md) | |
| [`CX::UnsizedArray`](./cx_idioms_h/unsized_array.md) | |
| [`CX::Pointer`](./cx_idioms_h/pointer.md) | |
| [`CX::MemberPointer`](./cx_idioms_h/member_pointer.md) | |
| [`CX::LValueReference`](./cx_idioms_h/l_value_reference.md) | |
| [`CX::RValueReference`](./cx_idioms_h/r_value_reference.md) | |
| [`CX::Enum`](./cx_idioms_h/enum.md) | |
| [`CX::Union`](./cx_idioms_h/union.md) | |
| [`CX::Struct`](./cx_idioms_h/struct.md) | |
| [`CX::Integral`](./cx_idioms_h/integral.md) | |
| [`CX::Floating`](./cx_idioms_h/floating.md) | |
| [`CX::Arithmetic`](./cx_idioms_h/arithmetic.md) | |
| [`CX::Signed`](./cx_idioms_h/signed.md) | |
| [`CX::Unsigned`](./cx_idioms_h/unsigned.md) | |
| [`CX::Scalar`](./cx_idioms_h/scalar.md) | |
| [`CX::TriviallyCopyable`](./cx_idioms_h/trivially_copyable.md) | |
| [`CX::HasBase`](./cx_idioms_h/has_base.md) | |
| [`CX::MemberFunction`](./cx_idioms_h/member_function.md) | |
| [`CX::StaticFunction`](./cx_idioms_h/static_function.md) | |
| [`CX::FunctionWithPrototype`](./cx_idioms_h/function_with_prototype.md) | |
| [`CX::VirtualFunction`](./cx_idioms_h/virtual_function.md) | |
| [`CX::VariadicFunction`](./cx_idioms_h/variadic_function.md) | |
| [`CX::NoexceptFunction`](./cx_idioms_h/noexcept_function.md) | |
| [`CX::MemberField`](./cx_idioms_h/member_field.md) | |
| [`CX::AdditionOperator`](./cx_idioms_h/addition_operator.md) | |
| [`CX::SubtractionOperator`](./cx_idioms_h/subtraction_operator.md) | |
| [`CX::DivisionOperator`](./cx_idioms_h/division_operator.md) | |
| [`CX::MultiplicationOperator`](./cx_idioms_h/multiplication_operator.md) | |
| [`CX::ModulusOperator`](./cx_idioms_h/modulus_operator.md) | |
| [`CX::BinaryAndOperator`](./cx_idioms_h/binary_and_operator.md) | |
| [`CX::BinaryOrOperator`](./cx_idioms_h/binary_or_operator.md) | |
| [`CX::LeftShiftOperator`](./cx_idioms_h/left_shift_operator.md) | |
| [`CX::RightShiftOperator`](./cx_idioms_h/right_shift_operator.md) | |
| [`CX::GraterThanOperator`](./cx_idioms_h/greater_than_operator.md) | |
| [`CX::LessThanOperator`](./cx_idioms_h/less_than_operator.md) | |
| [`CX::EqualityOperator`](./cx_idioms_h/equality_operator.md) | |
| [`CX::InequalityOperator`](./cx_idioms_h/inequality_operator.md) | |
| [`CX::GreaterOrEqualThanOperator`](./cx_idioms_h/greater_or_equal_than_operator.md) | |
| [`CX::LessOrEqualThanOperator`](./cx_idioms_h/less_or_equal_than_operator.md) | |
| [`CX::AssignmentOperator`](./cx_idioms_h/assignment_operator.md) | |
| [`CX::AdditionAssignmentOperator`](./cx_idioms_h/addition_assignment_operator.md) | |
| [`CX::SubtractionAssignmentOperator`](./cx_idioms_h/subtraction_assignment_operator.md) | |
| [`CX::MultiplicationOperator`](./cx_idioms_h/multiplication_operator.md) | |
| [`CX::DivisionAssignmentOperator`](./cx_idioms_h/division_assignment_operator.md) | |
| [`CX::ModulusAssignmentOperator`](./cx_idioms_h/modulus_assignment_operator.md) | |
| [`CX::OrAssignmentOperator`](./cx_idioms_h/or_assignment_operator.md) | |
| [`CX::AndAssignmentOperator`](./cx_idioms_h/and_assignment_operator.md) | |
| [`CX::XORAssignmentOperator`](./cx_idioms_h/xor_assignment_operator.md) | |
| [`CX::LeftShiftAssignmentOperator`](./cx_idioms_h/left_shift_assignment_operator.md) | |
| [`CX::RightShiftAssignmentOperator`](./cx_idioms_h/right_shift_assignment_operator.md) | |
| [`CX::CommaOperator`](./cx_idioms_h/comma_operator.md) | |
| [`CX::ThreeWayComparisonOperator`](./cx_idioms_h/three_way_comparison_operator.md) | |
| [`CX::MemberAccessOperator`](./cx_idioms_h/member_access_operator.md) | |
| [`CX::MemberPointerAccessOperator`](./cx_idioms_h/member_pointer_access_operator.md) | |
| [`CX::SubscriptOperator`](./cx_idioms_h/subscript_operator.md) | |
| [`CX::FunctionOperator`](./cx_idioms_h/function_operator.md) | |
| [`CX::DereferenceOperator`](./cx_idioms_h/dereference_operator.md) | |
| [`CX::ComplimentOperator`](./cx_idioms_h/compliment_operator.md) | |
| [`CX::IncrementOperator`](./cx_idioms_h/increment_operator.md) | |
| [`CX::DecrementOperator`](./cx_idioms_h/decrement_operator.md) | |
| [`CX::NotOperator`](./cx_idioms_h/not_operator.md) | |
| [`CX::AddressOperator`](./cx_idioms_h/address_operator.md) | |
| [`CX::ConversionOperator`](./cx_idioms_h/conversion_operator.md) | |

## Type Meta-Functions
| Name | Description |
| :- | :- |
| [`CX::Unqualified`](./cx_idioms_h/unqualified.md) | |
| [`CX::Decayed`](./cx_idioms_h/decayed.md) | |
| [`CX::ConstDecayed`](./cx_idioms_h/const_decayed.md) | |
| [`CX::VolatileDecayed`](./cx_idioms_h/volatile_decayed.md) | |
| [`CX::ConstVolatileDecayed`](./cx_idioms_h/const_volatile_decayed.md) | |
| [`CX::ConstVolatilePropagated`](./cx_idioms_h/const_volatile_propagated.md) | |
| [`CX::ArrayDecayed`](./cx_idioms_h/array_decayed.md) | |
| [`CX::LValueReferenceDecayed`](./cx_idioms_h/l_value_reference_decayed.md) | |
| [`CX::RValueReferenceDecayed`](./cx_idioms_h/r_value_reference_decayed.md) | |
| [`CX::ReferenceDecayed`](./cx_idioms_h/reference_decayed.md) | |
| [`CX::SignDecayed`](./cx_idioms_h/sign_decayed.md) | |
| [`CX::SignPromoted`](./cx_idioms_h/sign_promoted.md) | |
| [`CX::FunctionPrototype`](./cx_idioms_h/function_prototype.md) | |
| [`CX::MemberFunctionPrototype`](./cx_idioms_h/member_function_prototype.md) | |

## Value Meta-Functions
| Name | Description |
| :- | :- |
| [`CX::ArraySize`](./cx_idioms_h/array_size.md) | |

