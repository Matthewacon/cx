# `CX::MatchAnyType`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T, typename... Types>
concept CX::MatchAnyType = /*...*/;
```
Utility concept that compares `T` against any number of types; `Types...`.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  (T == Types || ...)
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. `T` is present in the type parameter pack `Types`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

static_assert(CX::MatchAnyType<int, float, char, double, int, void>);
static_assert(!CX::MatchAnyType<void, int *>);
static_assert(!CX::MatchAnyType<char8_t>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::SameType`](./same_type.md)
 - [`CX::MatchAnyValue`](./match_any_value.md)
