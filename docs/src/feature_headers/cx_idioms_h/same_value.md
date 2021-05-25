# `CX::SameValue`
## Description
<area id="no-interactive-code"></area>
```c++
template<auto V1, auto V2>
concept CX::SameValue = /*...*/;
```
Utility concept that compares two values; `V1` and `V2`.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  decltype(V1) == decltype(V2) && V1 == V2
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. The underlying types of `V1` and `V2` are the same.
  2. The values of `V1` and `V2` the same.

  > ℹ️
  > No implicit conversions are considered in the comparison
  > of `V1` and `V2`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

static_assert(CX::SameValue<0xdeadbeef, 0xdeadbeef>);
static_assert(!CX::SameValue<0x561082, 0x130426>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::SameType`](./same_type.md)
 - [`CX::SameTemplateType`](./same_template_type.md)
