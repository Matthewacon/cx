# `CX::Lambda`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename Prototype>
struct CX::Lambda {/*...*/};
```
A fast and general-purpose non-allocating function encapsulation abstraction.
`CX::Lambda` is similar to `std::function` but with several improvements:
 - Less invocation overhead
 - No heap allocation

`CX::Lambda` is interchangable with [`CX::AllocLambda`](./alloc_lambda.md).

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
  Lambda();
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
</table>

<!--Constructor descriptions-->
<ol id="constructor-list">
 <li>

   The default `CX::Lambda` constructor; creates an uninitialized
   lambda instance. Invoking the lambda created by this constructor
   will raise a runtime exception.

 </li>
 <li>

   The copy constructor for function pointers, anonymous lambdas and
   copy-constructible classes that define a member function operator
   with a prototype matching `Prototype`.

 </li>
 <li>

  The move constructor for movable anonymous lambdas and

 </li>
</ol>

### Member functions
| | |
| :- | :- |
| | |

## Example Usage
### `CX::Lambda` usage
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

### `CX::Lambda` <=> [`CX::AllocLambda`](./alloc_lambda.md) interaction
```c++
#define CX_STL_SUPPORT
#include <cx/lambda.h>

TODO
```

## See Also
 - [`<cx/lambda.h>`](../cx_lambda_h.md)
 - [`CX::AllocLambda`](./alloc_lambda.md)
