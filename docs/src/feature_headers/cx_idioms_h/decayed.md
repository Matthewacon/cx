# `CX::Decayed`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
using CX::Decayed = /*...*/;
```
Utility type meta-function that removes the right-most type qualifier
from the type `T`.

## Ambiguous Qualifier Precedence
<table id="member-function-table">
 <tr><td>

  ```c++
  T const volatile -> T volatile
  ```
  ---
  For types that have both `const` and `volatile` qualifiers, the
  `const` qualifier will be taken as the highest precedence.

 </td></tr>
 <tr><td>

  ```c++
  T(&)[]  -> T[]
  T(&&)[] -> T[]
  ```
  ---
  For array-reference types, the reference qualifier will be taken
  as the highest precedence.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

//`int`
using TypeA = CX::Decayed<int const>;
static_assert(CX::SameType<TypeA, int>);
//`void **`
using TypeB = CX::Decayed<void ***>;
static_assert(CX::SameType<TypeB, void **>);
//`char[]`
using TypeC = CX::Decayed<char(&)[]>;
static_assert(CX::SameType<TypeC, char[]>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
