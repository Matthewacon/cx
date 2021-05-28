# `CX::ConstVolatileDecayed`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
using CX::ConstVolatileDecayed = /*...*/;
```
Utility alias to remove immediate `const` and or `volatile`
(CV) qualifiers from a given type, `T`. If `T` is not
CV-qualified, it will be unmodified.

## Ambiguous CV Qualifications
<table id="member-function-table">
 <tr><td>

  ```c++
  //Unmodified
  int const volatile *     -> int const volatile *
  float const * volatile * -> float const * volatile *

  //Modified
  char const * volatile -> char const *
  short * const         -> short *
  ```
  ---
  CV-qualifiers that are not applied to the right-most
  non-CV-qualifiers, will be unmodified.

 </td></tr>
 <tr><td>

  ```c++
  T const volatile&  -> T&
  T const volatile&& -> T&&
  ```
  ---
  Since references cannot be CV-qualified, references to CV-qualified
  types will be converted to the underlying type.

 </td></tr>
 <tr><td>

  ```c++
  T const volatile(&)[]  -> T(&)[]
  T const volatile(&&)[] -> T(&&)[]
  ```
  ---
  Since references to arrays cannot be CV-qualified, references to
  CV-qualified array element types will be converted to references
  to an array with the underlying array element type.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

//`void *`
using TypeA = CX::ConstVolatileDecayed<void * const volatile>;
static_assert(CX::SameType<TypeA, void *>);
//`void (CX::Dummy<>::*)()`
using TypeB = CX::ConstVolatileDecayed<void (CX::Dummy<>::* const volatile)()>;
static_assert(CX::SameType<TypeB, void (CX::Dummy<>::*)()>);
//`int(&&)[]`
using TypeC = CX::ConstVolatileDecayed<int const volatile(&&)[]>;
static_assert(CX::SameType<TypeC, int(&&)[]>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::ConstDecayed`](./const_decayed.md)
 - [`CX::VolatileDecayed`](./volatile_decayed.md)
