# `CX::UniqueTypes`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename... Types>
concept CX::UniqueTypes = /*...*/;
```
Utility concept that determines whether or not a given parameter pack
of types, `Types...`, does not contain duplicates.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  bool duplicateFound = false;
  for (int i = 0; i < sizeof...(Types) && !duplicateFound; i++) {
   for (int j = 0; j < sizeof...(Types); j++) {
    if (j != i && Types[i] == Types[j]) {
     duplicateFound = true;
     break;
    }
   }
  }
  return !duplicateFound;
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. For any given type in `Types...` at index `I`, there must
     not be an identical type at index `J`, for all indices
     `I != J`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

static_assert(CX::UniqueTypes<int, float, char, void *, double>);
static_assert(!CX::UniqueTypes<void, double, char, void, int>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::UniqueTemplateTypes`](./unique_template_types.md)
 - [`CX::UniqueValues`](./unique_values.md)
