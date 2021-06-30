# `<cx/common.h>`
## Header Summary
This header provides the base definitions for all other headers in CX.

## Types
| Name | Description |
| :- | :- |
| [`CX::TrueType`](./cx_common_h/true_type.md) | Base for all `bool` value meta-functions with the value `true`. |
| [`CX::FalseType`](./cx_common_h/false_type.md) | Base for all `bool` value meta-functions with the value `false`. |
| [`CX::SizeType`](./cx_common_h/size_type.md) | The type of the value returned by `sizeof(...)` expressions. |
| [`CX::AlignType`](./cx_common_h/align_type.md) | The type of the value returned by `alignof(...)` expressions. |
| [`CX::NullptrType`](./cx_common_h/nullptr_type.md) | The type of `nullptr`. |
| [`CX::Dummy<typename...>`](./cx_common_h/dummy.md) | A defined (has linkage) type for deductions in compile-time meta-functions and conversions in runtime contexts. |
| [`CX::DummyTemplate<template<typename...> typename...>`](./cx_common_h/dummy_template.md) | A defined (has linkage) type for deductions in compile-time meta-functions and conversions in runtime contexts. |
| [`CX::DummyValueTemplate<auto...>`](./cx_common_h/dummy_value_template.md) | A defined (has linkage) type for deductions in compile-time meta-functions and conversions in runtime contexts. |
| [`CX::ImpossibleType<typename...>`](./cx_common_h/impossible_type.md) | An undefined (no linkage) type for compile-time meta-functions. |
| [`CX::ImpossibleTemplateType<template<typename...> typename...>`](./cx_common_h/impossible_template_type.md) | An undefined (no linkage) type for compile-time meta-functions. |
| [`CX::ImpossibleValueTemplateType<auto...>`](./cx_common_h/impossible_value_template_type.md) | An undefined (no linkage) type for compile-time meta-functions. |

## Type Meta-Functions
| Name | Description |
| :- | :- |
| [`CX::VoidT<typename...>`](./cx_common_h/void_t.md) |A compile-time meta-funciton that maps a pack of any types to the type `void`. |
| [`CX::VoidA<auto...>`](./cx_common_h/void_a.md) | A compile-time meta-function that maps a pack of any values to the type `void`. |
| [`CX::AsStlCompatible<typename T>`](./cx_common_h/as_stl_compatible.md) | A compile-time meta-function that converts CX-style type and value meta-functions to their STL-style equivalents. |

## Other Meta-Functions
| Name | Description |
| :- | :- |
| [`CX::declval<T>`](./cx_common_h/declval.md) | A compile-time meta-function that produces the value resulting from the type expression `T&&`. |
| [`CX::expect<T>`](./cx_common_h/expect.md) | A compile-time meta-function that consumes a value of the type `T`. |

## Function-Like Macros
| Name | Description |
| :- | :- |
| [`CX_PRAGMA_IMPL`](./cx_common_h/cx_pragma_impl.md) | A portable shim to the compiler/platform equivalent of `_Pragma(...)`. |
| [`CX_PRAGMA_MSG`](./cx_common_h/cx_pragma_msg.md) | A portable macro to emit messages during compile time. |
| [`CX_DEBUG_MSG`](./cx_common_h/cx_debug_msg.md) | A portable macro to emit debug messages during compile time. |
| [`CX_STL_SUPPORT_EXPR`](./cx_common_h/cx_stl_support_expr.md) | An macro that only yields the supplied expression if CX is built with STL support enabled. |
