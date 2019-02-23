# CX
A collection of conxtexpr tools for diverting non-operating boilerplate abstractions to compile-time.

## Current Features:
| Header | Function |
| :--- | :--- |
| `tuple.h` | A constexpr constructable type-safe tuple with a suite of constexpr utility functions for assessing stored elements. |
| `strings.h` | A collection of constexpr tools for string manipulation. |
| `templates.h` | A set of partially specialized templates for manipulating other templates and assembling template types. |
| `array.h` | A constexpr constructable homogeneous array that does not explicitly require the size for construction. Functionality extends from `CX::Tuple`. |

## Examples: (wip)
#### [`CX::Tuple`](https://github.com/Matthewacon/CX/blob/master/include/conway.h):
```cpp
#include "tuple.h"

//Explicit construction
constexpr auto my_tuple = CX::Tuple<int, char, unsigned int, const char*>(-1, 'a', 1, "Hello World!");
//Implicit construction
constexpr auto my_other_tuple = CX::tuple(0xDEADBEEF, -3.14, "Bjarne");

//Element retrieval
constexpr unsigned int my_int = my_tuple.get<1>();
constexpr const char* my_name = my_other_tuple.get<2>();

//Iteration
//Define a class with a static callback template function 
class ExampleIterator {
public:
 template<typename T>
 static constexpr void process(T value) {
  if constexpr(std::is_same<T, float>::value) {
   //Do something with my float
  } else {
   //Do something else
  }
 }
};

my_tuple.iterate<ExampleIterator>();
```

## License
This project is licensed under the [M.I.T. License](https://github.com/Matthewacon/CX/blob/master/LICENSE)
