# `CX::ConvertibleTo`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T1, typename T2>
concept CX::ConvertibleTo = /*...*/;
```
Utility concept that checks whether or not `T1` is convertible
to `T2`.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  template<typename T1, typename T2>
  concept CX::ConvertibleTo = requires (T1 t1) {
   { (T2)t1 } -> CX::SameType<T2>;
  };
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. For any given type, `T1`, a conversion must exist to another
     type, `T2`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

struct A {
 //Implicit conversion operator
 operator int() {
  return 0;
 }
};

static_assert(CX::ConvertibleTo<int, float>);
static_assert(CX::ConvertibleTo<A, int>);
static_assert(!CX::ConvertibleTo<void *, void (A::*)()>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
