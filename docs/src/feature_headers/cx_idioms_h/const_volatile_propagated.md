# `CX::ConstVolatilePropagated`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T1, typename T2>
using CX::ConstVolatilePropagated = /*...*/;
```
Utility alias that propagates CV qualifiers from `T1` to `T2`.

## Example Usage
```c++
#include <cx/idioms.h>

//`char const`
using TypeA = CX::ConstVolatilePropagated<int const, char>;
static_assert(CX::SameType<TypeA, char const>);
//`float volatile(&)[]`
using TypeB = CX::ConstVolatilePropagated<short volatile, float(&)[]>;
static_assert(CX::SameType<TypeB, float volatile(&)[]>);
//`double * const volatile`
using TypeC = CX::ConstVolatilePropagated<int const volatile&&, double *>;
static_assert(CX::SameType<TypeC, double * const volatile>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::ConstDecayed`](./const_decayed.md)
 - [`CX::VolatileDecayed`](./volatile_decayed.md)
 - [`CX::ConstVolatileDecayed`](./const_volatile_decayed.md)
