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

<table>
 <tr>
  <td><p><area id="no-interactive-code"></area></p>
   <pre><code class="language-c++">Lambda::Lambda();</code></pre>
  </td>
  <td>(1)</td>
 </tr>
 <tr>
 <td><p><area id="no-interactive-code"></area></p>
  <pre><code class="language-c++">template&lt;typename F&gt;
Lambda::Lambda(F const&);</code></pre>
 </td>
 <td>(2)</td>
 </tr>
</table>

### Member functions
| Name | Description |
| :- | :- |
| <ccode>Lambda::Lambda()</ccode> |  |
| <pre><code class="language-c++ hljs">template&lt;typename F&gt;<br>Lambda::Lambda(F const&) </code></pre> | |
| <ccode>template&lt;typename F&gt;<br>Lambda::Lambda(F const&)</ccode> | |
| ```c++ template<typename F> Lambda::Lambda(F&&)``` | |
| `Lambda::Lambda(Lambda const&)` | |
| `Lambda::Lambda(Lambda&&)` | |
| `Lambda::Lambda(CompatibleLambda<Lambda> const&)` | |
| `Lambda::Lambda(CompatibleLambda<Lambda>&&)` | |

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
