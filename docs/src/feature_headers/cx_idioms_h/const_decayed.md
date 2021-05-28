# `CX::ConstDecayed`
## Description
```c++
template<typename T>
using CX::ConstDecayed = /*...*/;
```
Utility alias to remove immediate `const` qualifiers from a given
type, `T`. If `T` is not `const` qualified, it will be
unmodified.

## Ambiguous Const Qualifications
<table id="member-function-table">
 <tr><td>

  ```c++
  //Unmodified
  int const *           -> int const *
  float const * const * -> float const * const *

  //Modified
  char const * const -> char const *
  short * const      -> short *
  ```
  ---
  `const` qualifiers that are not applied to the right-most
  non-`const` qualifier, will be unmodified.

 </td></tr>
 <tr><td>

  ```c++
  T const&  -> T&
  T const&& -> T&&
  ```
  ---
  Since references cannot be `const` qualified, references to `const`
  qualified types will be converted to references to the underlying
  type.

 </td></tr>
 <tr><td>

  ```c++
  T const(&)[]  -> T(&)[]
  T const(&&)[] -> T(&&)[]
  ```
  ---
  Since references to arrays cannot be `const` qualified, references
  to `const` qualified array element types will be converted to
  references to an array with the underlying array element type.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

//`float`
using TypeA = CX::ConstDecayed<float const>;
static_assert(CX::SameType<TypeA, float>);
//`int&&`
using TypeB = CX::ConstDecayed<int const&&>;
static_assert(CX::SameType<TypeB, int&&>);
//`short(&)[]`
using TypeC = CX::ConstDecayed<short const(&&)[]>;
static_assert(CX::SameType<TypeC, short(&&)[]>);
```
## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::Const`](./const.md)
 - [`CX::ConstVolatileDecayed`](./const_volatile_decayed.md)
 - [`CX::ConstVolatilePropagated`](./const_volatile_propagated.md)
