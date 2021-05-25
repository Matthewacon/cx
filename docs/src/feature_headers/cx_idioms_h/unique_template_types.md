# `CX::UniqueTemplateTypes`
## Description
<area id="no-interactive-code"></area>
```c++
template<template<typename...> typename... Types>
concept CX::UniqueTemplateTypes = /*...*/;
```
Utility concept that determines whether or not a given parameter pack
of unspecialized template types, `Types...`, does not contain
duplicates.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  bool duplicateFound = false;
  for (int i = 0; i < sizeof...(Types) && !duplicateFound; i++) {
   for (int j = 0; j < sizeof...(Types); j++) {
    if (i != j && Types[i] == Types[j]) {
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

//Note: These headers are not required to use
//`CX::UniqueTemplateTypes`, they are part of the
//example
#include <vector>
#include <tuple>
#include <type_traits>

static_assert(CX::UniqueTemplateTypes<std::vector, std::tuple, std::is_same>);
static_assert(!CX::UniqueTemplateTypes<std::is_integral, std::vector, std::is_integral>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::UniqueTypes`](./unique_types.md)
 - [`CX::UniqueValues`](./unique_values.md)
