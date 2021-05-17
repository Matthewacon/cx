# `<cx/common.h>`
## Header Summary
This header provides the base definitions for all other headers in CX.

## Types
| Name | Description |
| :- | :- |
| [`CX::TrueType`](./cx_common_h/true_type.md) | Base for all `bool` value meta-functions with the value `true`. |
| [`CX::FalseType`](./cx_common_h/false_type.md) | Base for all `bool` value meta-functions with the value `false`. |
| [`CX::SizeType`]() | The type of the value returned by `sizeof(...)` expressions. |
| [`CX::AlignType`]() | The type of the value returned by `alignof(...)` expressions. |
| [`CX::NullptrType`]() | The type of `nullptr`. |
| [`CX::Dummy<typename...>`]() | A defined (has linkage) type for deductions in compile-time meta-functions and conversions in runtime contexts. |

## Type Meta-Functions
| Name | Description |
| :- | :- |
| [`CX::VoidT<typename...>`]() | |
| [`CX::VoidA<auto...>`]() | |
| [`CX::AsStlCompatible<typename T>`]() | |

## Other Meta-Functions
| Name | Description |
| :- | :- |
| [`CX::declval<T>`]() | |
| [`CX::expect<T>`]() | |
