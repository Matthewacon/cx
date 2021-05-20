# `CX::AlignType`
## Description
<area id="no-interactive-code"></area>
```c++
using CX::AlignType = decltype(alignof(int));
```
`CX::AlignType` is the unsigned integer result of the expression above.
The actual underlying type is implementation defined.

> ℹ️
> In all compliant compilers, the underling type of `CX::AlignType` should
> be identical to [`CX::SizeType`](./size_type.md).

## Example Usage
```c++
#include <cx/common.h>

void * customAlignedAllocator(CX::SizeType size, CX::AlignType alignment) {
 void * ptr/* = ...*/;
 return ptr;
}

int main() {
 /*...*/
 auto alignedMemory = customAlignedAllocator(32, 512);
 /*...*/
}
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::SizeType`](./size_type.md)
