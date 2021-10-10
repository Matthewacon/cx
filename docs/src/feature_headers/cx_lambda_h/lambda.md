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
   typename
   Prototype
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
<table id="constructor-table-1">
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
   && !CX::Const<F>
   && !CX::Struct<F>
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
<ol id="constructor-list-1">
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

### Member Functions
All of the following member functions are defined for the following
partial specializations of `CX::Lambda`:
<area id="no-interactive-code"></area>
```c++
template<typename R, typename... Args>
//Where `Prototype` is any of:
// - `R (Args...)`
// - `R (Args..., ...)`
// - `R (Args...) noexcept`
// - `R (Args..., ...) noexcept`
struct CX::Lambda<Prototype> {
 /*Member function definition here*/
};
```
<!--Member function definitions-->
<table id="member-function-table">
 <tr><td>

   ```c++
   operator bool() const noexcept;
   ```
   ---
   Presence conversion operator. Used for detecting whether or not
   an instance of `CX::Lambda` has a value.

   #### Example Usage
   ```c++
   #define CX_STL_SUPPORT
   #include <cx/lambda.h>

   //Note: This header is not required to use `CX::Lambda`,
   //it is part of the example
   #include <iostream>

   int main() {
    CX::Lambda l;

    if (!l) {
     std::cout << "`l` does not have a value!" << std::endl;
    }

    l = [] {};

    if (l) {
     std::cout << "`l` has a value!" << std::endl;
    }
   }
   ```

 </td></tr>
 <tr><td>

  ```c++
  void reset();
  ```
  ---
  Destructs the function or functor encapsulated by the `CX::Lambda`
  instance.

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: This header is not required to use `CX::Lambda`,
  //it is part of the example
  #include <iostream>

  int main() {
   CX::Lambda l = [] {};

   l.reset();

   if (!l) {
    std::cout << "`l` does not have a value!" << std::endl;
   }
  }
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<CompatibleLambda L>
  explicit operator L() const;
  ```
  ---
  Implicit lambda conversion operator. Allows conversions to compatible
  lambda types.

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: This header is not required to use `CX::Lambda`,
  //it is part of the example
  #include <iostream>

  int main() {
   int i = 0;
   CX::Lambda l1 = [&] {
    std::cout << "Hello world: " << i++ << std::endl;
   };
   l1();

   auto l2 = (CX::AllocLambda<void ()>)l1;
   l2();
  }
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda& operator=(F const&);
  ```
  ---
  Function and functor copy assignment operator. The behaviour of this
  member function is the same as the
  [copy constructor (2)](#constructor-2).

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: This header is not required to use `CX::Lambda`,
  //it is part of the example
  #include <iostream>

  int main() {
   CX::Lambda l;
   l = [] {
    std::cout << "Hello world!" << std::endl;
   };
   l();
  }
  ```

 </td></tr>
 <tr><td>

  <area id="no-interactive-code"></area>
  ```c++
  template<typename F>
  requires (!IsLambda<F>
   && !CX::Const<F>
   && !CX::Struct<F>
   && SupportedPrototype<F>
  )
  Lambda& operator=(F&&);
  ```
  ---
  Functor move assignment operator. The behaviour of this
  member function is the same as the
  [copy constructor (3)](#constructor-3).

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: These headers are not required to use `CX::Lambda`,
  //they are part of the example
  #include <iostream>
  #include <memory>

  int main() {
   CX::Lambda l;
   auto anonymousLambda = [] {
    std::cout << "Hello world!" << std::endl;
   };

   l = std::move(anonymousLambda);
   l();
  }
  ```

 </td></tr>
 <tr><td>

  ```c++
  Lambda& operator=(Lambda const&);
  ```
  ---
  `CX::Lambda` copy assignment operator. The behaviour of this
  function is the same as the
  [copy constructor (4)](#constructor-4).

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: This header is not required to use `CX::Lambda`,
  //it is part of the example
  #include <iostream>

  int main() {
   CX::Lambda l1 = [] {
    std::cout << "Hello world!" << std::endl;
   };
   CX::Lambda l2 = l1;
   l2();
  }
  ```

 </td></tr>
 <tr><td>

  ```c++
  Lambda& operator=(Lambda&&);
  ```
  ---
  `CX::Lambda` move assignment operator. The behaviour of this
  function is the same as the
  [move constructor (5)](#constructor-5).

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: These headers are not required to use `CX::Lambda`,
  //they are part of the example
  #include <iostream>
  #include <memory>

  int main() {
   CX::Lambda l1 = [] {
    std::cout << "Hello world!" << std::endl;
   };

   CX::Lambda l2 = std::move(l1);
   if (!l1) {
    std::cout << "`l1` does not contain a value!" << std::endl;
   }

   l2();
  }
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<CompatibleLambda L>
  Lambda& operator=(L const&);
  ```
  ---
  The copy assignment operator for compatible function encapsulators.
  The behaviour of this function is the same as the
  [copy constructor (6)](#constructor-6).

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: This header is not required to use `CX::Lambda`,
  //it is part of the example
  #include <iostream>

  int main() {
   CX::AllocLambda l1 = [] {
    std::cout << "Hello world!" << std::endl;
   };
   CX::Lambda l2 = l1;
   l2();
  }
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<CompatibleLambda L>
  requires (!CX::Const<L>)
  Lambda& operator=(L&&);
  ```
  ---
  The move assignment operator for compatible function encapsulators.
  The behaviour of this function is the same as the
  [move constructor (7)](#constructor-7).

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: These headers are not required to use `CX::Lambda`,
  //they are part of the example
  #include <iostream>
  #include <memory>

  int main() {
   CX::AllocLambda l1 = [] {
    std::cout << "Hello world!" << std::endl;
   };

   CX::Lambda l2 = std::move(l1);
   if (!l1) {
    std::cout << "`l1` does not contain a value!" << std::endl;
   }

   l2();
  }
  ```

 </td></tr>
</table>

<!--Member function operators-->
The member function operator definition, `operator()`, is slightly
different for each partial specialization of `CX::Lambda`:

<table id="member-function-table">
 <tr><td>

  ```c++
  template<typename R, typename... Args>
  struct CX::Lambda<R (Args...)> {
   R operator()(Args...);
  };
  ```
  ---
  #### Description
  The member function operator definition for the `CX::Lambda`
  partial specialization for non-c-variadic functions that *may*
  throw.

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: This header is not required to use `CX::Lambda`,
  //it is part of the example
  #include <iostream>

  int main() {
   CX::Lambda l = [](int i, float f) {
    std::cout << "l(" << i << ", " << f << ")" << std::endl;
   };
   l(1234, 5.678);
  }
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<typename R, typename... Args>
  struct CX::Lambda<R (Args..., ...)> {
   template<typename... Varargs>
   R operator()(Args..., Varargs...);
  };
  ```
  ---
  #### Description
  The member function operator definition for the `CX::Lambda`
  partial specialization for c-variadic functions that *may*
  throw.

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: These headers are not required to use `CX::Lambda`,
  //they are part of the example
  #include <iostream>
  #include <cx/vararg.h>

  int main() {
   CX::Lambda l = [](int n, ...) {
    CX::VaList list;
    va_start(list, n);
    for (int i = 0; i < n; i++) {
     std::cout << "[" << i << "]: " << list.arg<int>() << std::endl;
    }
   };

   l(3, 1, 2, 3);
  }
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<typename R, typename... Args>
  struct CX::Lambda<R (Args...) noexcept> {
   R operator()(Args...) noexcept;
  };
  ```
  ---
  #### Description
  The member function operator definition for the `CX::Lambda`
  partial specialization for non-c-variadic functions that
  *should not* throw.

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: This header is not required to use `CX::Lambda`,
  //it is part of the example
  #include <stdexcept>

  int main() {
   CX::Lambda l = [] noexcept {
    throw std::runtime_error{
     "Uh oh! Time to terminate!"
    };
   };
   l();
  }
  ```

 </td></tr>
 <tr><td>

  ```c++
  template<typename R, typename... Args>
  struct CX::Lambda<R (Args..., ...) noexcept> {
   template<typename... Varargs>
   R operator()(Args..., Varargs...) noexcept;
  };
  ```
  ---
  #### Description
  The member function operator definition for the `CX::Lambda` partial
  specialization for c-variadic functions that *should not* throw.

  #### Example Usage
  ```c++
  #define CX_STL_SUPPORT
  #include <cx/lambda.h>

  //Note: These headers are not required to use `CX::Lambda`,
  //they are part of the example
  #include <iostream>
  #include <stdexcept>
  #include <cx/vararg.h>

  int main() {
   CX::Lambda l = [](int n, ...) noexcept {
    CX::VaList list;
    va_start(list, n);
    for (int i = 0; i < n; i++) {
     auto const arg = list.arg<int>();
     std::cout << "[" << i << "]: " << arg << std::endl;
     if (arg == 0xdead) {
      throw std::runtime_error{
       "Uh oh! Time to terminate!"
      };
     }
    }
   };

   l(4, 1, 2, 0xdead, 4);
  }
  ```

 </td></tr>
</table>

## See Also
 - [`<cx/lambda.h>`](../cx_lambda_h.md)
 - [`CX::AllocLambda`](./alloc_lambda.md)
 - [`<cx/error.h>`](../cx_error_h.md)
 - [`<cx/vararg.h>`](../cx_vararg_h.md)
