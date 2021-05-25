# `CX::SameTemplateType`
## Description
<area id="no-interactive-code"></area>
```c++
template<
 template<typename...> typename T1,
 template<typename...> typename T2
>
concept CX::SameTemplateType = /*...*/;
```
Utility concept that compares two unspecialized template types;
`T1` and `T2`.

## Satisfaction Conditions
<table id="member-function-table">
 <tr><td>

  ```c++
  T1 == T2
  ```
  ---
  `T1` and `T2` are the same unspecialized template type.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

//Note: These headers are not required to use
//`CX::SameTemplateType`, they are part of the example
#include <vector>
#include <tuple>

static_assert(CX::SameTemplateType<std::tuple, std::tuple>);
static_assert(!CX::SameTemplateType<std::vector, std::tuple>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::SameType`](./same_type.md)
 - [`CX::SameValue`](./same_value.md)
