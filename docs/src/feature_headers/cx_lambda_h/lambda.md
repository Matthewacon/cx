# `CX::Lambda`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename Prototype>
struct CX::Lambda {/*...*/};
```
A fast and general-purpose non-allocating function encapsulation abstraction.
`CX::Lambda` is similar to `std::function` but with several improvements:
 - Minimal invocation overhead
 - Faster than [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function)
 - No heap allocation
 - Interoperable with [`CX::AllocLambda`](./alloc_lambda.md)
 - Support for c-variadic functions and functors
 - Support for `noexcept` functions and functors

### ⚠️ Limitations
- `CX::Lambda` instances can only encapsulate types smaller than
  [`CX_LAMBDA_BUF_SIZE`](./cx_lambda_buf_size.md) and with alignment
  less than [`CX_LAMBDA_BUF_ALIGN`](./cx_lambda_buf_align.md). The
  default values for these macros should be sufficient for most
  usages, however, users may override these values to meet their
  needs.

- `CX::AllocLambda` can be both copied and moved into a `CX::Lambda`
  instance, however, trying to copy a `CX::AllocLambda` encapsulating
  a type that does not meet the requirements of
  [`CX_LAMBDA_BUF_SIZE`](./cx_lambda_buf_size.md) and
  [`CX_LAMBDA_BUF_ALIGN`](./cx_lambda_buf_align.md) will raise an
  instance of
  [`CX::IncompatibleLambdaError`](./incompatible_lambda_error.md).

### Template Parameters
<table>
 <tr>
  <td>

   <area id="no-interactive-code"></area>
   ```c++
   typename Prototype   
   ```

  </td>
  <td>

   The function prototype that this lambda interacts with. There are
   4 *special* partial specializations of `CX::Lambda` that
   correspond to special function prototypes:

   1. `R (Args...)`
   2. `R (Args..., ...)`
   3. `R (Args...) noexcept`
   4. `R (Args..., ...) noexcept`

   The parameters `R` and `Args` are deduced as part of the partial
   specializations and compose the `Prototype` parameter. They are
   defined as follows:

   - `R`: The return type
   - `Args...`: The pack of argument types

   The `CX::Lambda` partial specializations have slightly different
   behaviours:

   1. The specialization for a non-c-variadic function that *may*
      throw exceptions. Exceptions thrown while invoking the
      encapsulated function will be passed back to the caller.

   2. The specialization for a c-variadic function that *may*
      throw exceptions. Exception behaviour is the same as the
      2nd partial specialization.

   3. The specialization for a non-c-variadic function that
      *should not* throw exceptions. Exceptions thrown while
      invoking the encapsulated function will invoke
      [`CX::error(CXError&)`](../cx_error_h/cx_error.md).

      > ℹ️
      > `CX::error` implementations that re-throw the passed
      > error will cause the exception runtime to invoke
      > [`std::terminate`](https://en.cppreference.com/w/cpp/error/terminate)
      > due to the `noexcept` specifier on the encapsulated
      > function.

   4. The specialization for a c-variadic function that
      *should not* throw exceptions. Exception behaviour is the
      same as the 3rd partial specialization.

  </td>
 </tr>
</table>

### Constructors
All of the following constructors are defined for the following partial
specializations of `CX::Lambda`:
<area id="no-interactive-code"></area>
```c++
template<typename R, typename... Args>
//Where `Prototype` is any of:
// - `R (Args...)`
// - `R (Args..., ...)`
// - `R (Args...) noexcept`
// - `R (Args..., ...) noexcept`
struct CX::Lambda<Prototype> {
 /*Constructor definition here*/
};
```

<!--Constructor definitions-->
<table id="constructor-table">
 <tr><td>

  ```c++
  Lambda() {/*...*/}
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda(F const&) {/*...*/}
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<typename F>
  requires (!IsLambda<F>
   && !Const<F>
   && !Struct<F>
   && SupportedPrototype<F>
  )
  Lambda(F&&) {/*...*/}
  ```

 </td></tr>
 <tr><td>

  ```c++
  Lambda(Lambda const &) {/*...*/}
  ```

 </td></tr>
 <tr><td>

  ```c++
  Lambda(Lambda&&) {/*...*/}
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<CompatibleLambda<Lambda> L>
  Lambda(L const&) {/*...*/}
  ```

 </td></tr>
 <tr><td>

 ```c++
 template<CompatibleLambda<Lambda> L>
 Lambda(L&&) {/*...*/}
 ```

 </td></tr>
</table>

<!--Constructor descriptions-->
<ol id="constructor-list">
 <li>

   The default `CX::Lambda` constructor; creates an uninitialized
   lambda instance.

   > ⚠️
   > Invoking the lambda created by this constructor will raise
   > an instance of the
   > [`CX::UninitializedLambdaError`](./uninitialized_lambda_error.md)
   > exception.

 </li>
 <li>

   The copy constructor for function pointers, anonymous lambdas and
   classes that define a member function operator with a prototype
   matching `Prototype`.

   > ℹ️
   > This constructor will accept types that are copy-constructible
   > or classes that are both default-constructible and
   > copy-assignable.

 </li>
 <li>

  The move constructor for anonymous lambdas and classes that define
  a member function operator with a prototype matching `Prototype`.

  > ℹ️
  > This constructor will accept types that are move-constructible
  > or classes that are both default-constructible and
  > move-assignable.

 </li>
 <li>

  The `CX::Lambda<Prototype>` copy-constructor.

 </li>
 <li>

  The `CX::Lambda<Prototype>` move-constructor.

 </li>
 <li>

  The copy-constructor for compatible function encapsulators.
  Accepts types that satisfy the
  [`CX::CompatibleLambda`](./compatible_lambda.md) concept.

 </li>
 <li>

  The move-constructor for compatible functoin encapsulators.
  Accepts types that satisfy the
  [`CX::CompatibleLambda`](./compatible_lambda.md) concept.

 </li>
</ol>

### Member functions
| | |
| :- | :- |
| | |

## Example Usage
### `CX::Lambda` Usage
```c++
#define CX_STL_SUPPORT
#include <cx/lambda.h>

//Note: This header is not required to use `CX::Lambda`,
//it is part of the example
#include <iostream>

void f1(float f, char const * str);

int main() {
 CX::Lambda l1{[] {
  std::cout << "Hello from CX::Lambda (l1)!" << std::endl;
 }};
 l1();

 CX::Lambda l2 = [](int i) {
  std::cout << "Hello from CX::Lambda (l2): " << i << "!" << std::endl;
 };
 l2(31415);

 CX::Lambda l3{&f1};
 l3(2.71, "hello cx");
}

void f1(float f, char const * str) {
 std::cout << "Hello from f1(" << f << ", '" << str << "')!" << std::endl;
}
```

### `CX::Lambda` <=> [`CX::AllocLambda`](./alloc_lambda.md) Interaction
```c++
#define CX_STL_SUPPORT
#include <cx/lambda.h>

TODO
```

## See Also
 - [`<cx/lambda.h>`](../cx_lambda_h.md)
 - [`CX::AllocLambda`](./alloc_lambda.md)
 - [`<cx/error.h>`](../cx_error_h.md)
