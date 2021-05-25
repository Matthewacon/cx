# `CX::MatchAnyValue`
## Description
<area id="no-interactive-code"></area>
```c++
template<auto V, auto... Values>
concept CX::MatchAnyValue = /*...*/;
```
Utility concept that compares `V` against any number of values;
`Values...`.

## Satisfaction Conditions
<table id="member-function-table">
 <tr><td>

  ```c++
  (decltype(V) == decltype(Values) || ...) && (V == Values || ...)
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. The underlying types of `V` and any element of `Values` are the
     same.
  2. The value of `V` and any of the values in `Values` are the
     same.

  > ℹ️
  > No implicit conversions are considered in the comparison
  > of `V` and `Values`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

static_assert(CX::MatchAnyValue<0, 1, 2, 3, 4, 5, 6, 0, 7, 8, 9, 10, -11>);
static_assert(!CX::MatchAnyValue<-1, 1, 2, 3, 2, 1>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::SameValue`](./same_value.md)
 - [`CX::MatchAnyType`](./match_any_type.md)
