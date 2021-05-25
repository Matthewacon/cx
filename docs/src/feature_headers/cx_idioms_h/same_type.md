# `CX::SameType`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T1, typename T2>
concept CX::SameType = /*...*/;
```
Utility concept that compares two types; `T1` and `T2`.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  T1 == T2
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. `T1` and `T2` are the same type.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

static_assert(CX::SameType<int, int>);
static_assert(!CX::SameType<void, char>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::SameTemplateType`](./same_template_type.md)
 - [`CX::SameValue`](./same_value.md)
