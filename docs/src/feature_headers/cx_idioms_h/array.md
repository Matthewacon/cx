# `CX::Array`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
concept CX::Array = /*...*/;
```
Array type identity concept. Checks whether or not a given type,
`T`, is am array.

## Caveats


## Example Usage
```c++
#include <cx/idioms.h>

static_assert(CX::Array<int[]>);
static_assert(CX::Array<float[1234]>);
static_assert(CX::Array<char[0]>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::SizedArray`](./sized_array.md)
 - [`CX::UnsizedArray`](./unsized_array.md)
 - [`CX::ArrayDecayed`](./array_decayed.md)
 - [`CX::ArraySize`](./array_size.md)
