# `<cx/lambda.h>`
## Header Summary
This header provides function encapsulation and c-variadic lambda
to function pointer conversion constructs.

## Concepts
| Name | Description |
| :- | :- |
| [`CX::IsLambda`](./cx_lambda_h/is_lambda.md) | |
| [`CX::IsAllocLambda`](./cx_lambda_h/is_alloc_lambda.md) | |
| [`CX::IsNonAllocLambda`](./cx_lambda_h/is_non_alloc_lambda.md) | |
| [`CX::CompatibleLambda`](./cx_lambda_h/compatible_lambda.md) | |

## Types
| Name | Description |
| :- | :- |
| [`CX::Lambda`](./cx_lambda_h/lambda.md) | |
| [`CX::AllocLambda`](./cx_lambda_h/alloc_lambda.md) | |
| [`CX::UninitializedLambdaError`](./cx_lambda_h/uninitialized_lambda_error.md) | |
| [`CX::IncompatibleLambdaError`](./cx_lambda_h/incompatible_lambda_error.md) | |

## Functions
| Name | Description |
| :- | :- |
| [``]() | |
| [``]() | |

## Macros
| Name | Default Value | Overridable | Description |
| :- | :- | :-: | :- |
| [`CX_LAMBDA_BUF_SIZE`](./cx_lambda_h/cx_lambda_buf_size.md) | `sizeof(void *) * 8` | ✔️ |  |
| [`CX_LAMBDA_BUF_ALIGN`](./cx_lambda_h/cx_lambda_buf_align.md) | `sizeof(void *) * 8` | ✔️ |  |

