# `CX::UniqueValues`
## Description
<area id="no-interactive-code"></area>
```c++
template<auto... Values>
concept CX::UniqueValues = /*...*/;
```
Utility concept that determines whether or not a given non-type
parameter pack, `Values...`, does not contain duplicate values.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  bool duplicateFound = false;
  for (int i = 0; i < sizeof...(Values) && !duplicateFound; i++) {
   for (int j = 0; j < sizeof...(Values); j++) {
    if (i != j && Values[i] == Values[j]) {
     duplicateFound = true;
     break;
    }
   }
  }
  return !duplicateFound;
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. For any given value in `Types...` at index `I`, there must not
     be an identical value at index `J`, for all indices `I != J`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

static_assert(CX::UniqueValues<0, 1, 2, 3, 4, 5, 6>);
static_assert(!CX::UniqueValues<0, 1, 2, 3, 2, 1, 0>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::UniqueTypes`](./unique_types.md)
 - [`CX::UniqueTemplateTypes`](./unique_template_types.md)
