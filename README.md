# CX
CX is a collection of constexpr tools for diverting non-operating boilerplate abstractions to compile-time. It serves 
to act as a replacement for, and expansion of, the meta-utilities provided by the STL.  

## Current Features:
| Header | Function |
| :--- | :--- |
| [`common.h`](https://github.com/Matthewacon/CX/blob/master/cx/common.h) | The base set of identity traits used throughout CX. |
| [`idioms.h`](https://github.com/Matthewacon/CX/blob/master/cx/idioms.h) | A set of general detection traits. |
| [`indirection.h`](https://github.com/Matthewacon/CX/blob/master/cx/indirection.h) | A set of traits to manipulate type qualifiers and variadic templates. |
| [`templates.h`](https://github.com/Matthewacon/CX/blob/master/cx/templates.h) | A set of partially specialized templates for manipulating other templates and assembling template types. |
| [`classes.h`](https://github.com/Matthewacon/CX/blob/master/cx/classes.h) | A set of traits that can be used to specify explicit casting routes and prevent obscure dynamic runtime casts. |
| [`tuple.h`](https://github.com/Matthewacon/CX/blob/master/cx/tuple.h) | A constexpr constructable type-safe tuple with a suite of constexpr utility functions for assessing stored elements. |
| [`strings.h`](https://github.com/Matthewacon/CX/blob/master/cx/strings.h) | A collection of constexpr tools for string manipulation. |
| [`array.h`](https://github.com/Matthewacon/CX/blob/master/cx/array.h) | A constexpr constructable homogeneous array that does not explicitly require the size for construction. Functionality extends from `CX::Tuple`. |
| [`lambda.h`](https://github.com/Matthewacon/CX/blob/master/cx/lambda.h) | A drop-in replacement for `std::function` that is functionally identical and half the size. |
| [`vararg.h`](https://github.com/Matthewacon/CX/blob/master/cx/vararg.h) | An implementation-agnostic set of utilities and types that can be used to consistently handle C-style varargs. |
| [`unsafe.h`](https://github.com/Matthewacon/CX/blob/master/cx/unsafe.h) | A collection of utility functions and types for reinterpreting memory. |

## Examples: (wip)
#### [`common.h`](https://github.com/Matthewacon/CX/blob/master/cx/common.h):
```cpp
#include <cx/common.h>

//Same as std::declval<T>(), but with no type restrictions
//This function has no linkage and can only be used in mata-functions
T&& declval<T>();

//An identity type for trait truthiness. Exposes a member `::value` which is true
CX::true_type

//Like CX::true_type, except `::value` is false  
CX::false_type

//A language-level metafunction for use with template traits
CX::void_t<...>

//Same as CX::void_t, but for non-type template arguments
CX::void_a<...> 

//A general purpose template class. Can be used for template parameter deduction or member pointer assembly
CX::Dummy<...> 
```

#### [`idioms.h`](https://github.com/Matthewacon/CX/blob/master/cx/idioms.h):

```cpp
#include <cx/idioms.h>

//Detect if X operator exists for type T
//See table below for all operator detector traits
constexpr const auto exists = CX::XOperatorExists<SomeType>::value;

//True if T1 and T2 are the same
constexpr const auto same = CX::IsSame<T1, T2>::value;

//True if template types T1<...> and T2<...> are the same
//Does not require T1 or T2 to be explicitly specialized
constexpr const auto same = CX::TemplateIsSame<T1, T2>::value;

//True if T is a pointer type
constexpr const auto isPointer = CX::IsPointer<T>::value;

//True if T is an l-value reference
constexpr const auto isLRef = CX::IsLValueReference<T>::value;

//True if T is an r-value reference
constexpr const auto isRRef = CX::IsRValueReference<T>::value;

//True if any type arguments following T1 are equal to T1
constexpr const auto anyMatch = CX::MatchAny<T1, ...>::value;

//True if T1 is explicitly convertable to T2
constexpr const auto convertable = CX::IsCastable<T1, T2>::value;

//True if Target exposes a constructor that takes Args...
constexpr const auto hasConstructor = CX::HasConstructor<Target, Args...>::value;

//True if Function is a member function
constexpr const auto isMemberFunction = CX::IsMemberFunction<Function>::value;

//True if Function is a global or static member function
constexpr const auto isStaticFunction = CX::IsStaticFunction<Function>::value;

//True if T is a function type
constexpr const auto isFunction = CX::IsFunction<T>::value;

//True if Function is a virtual member function
constexpr const auto isVirtualFunction = CX::IsVirtualFunction<Function>::value;

//True if Field is a member field
constexpr const auto isMemberField = CX::IsMemberField<Field>::value;

//True if Field is a global field or a member field
constexpr const auto isField = CX::IsField<Field>::value;
```

##### Defined Operator Detection Traits:
| X | Operator |
| :--- | :--- |
| Plus | + |
| Minus | - |
| Div | / |
| Mult | * |
| Mod | % |
| And | && |
| Or | || |
| LShift | << |
| RShift | >> |
| GThan | > |
| LThan | < |
| Equality | == |
| InEquality | != |
| GEthan | >= |
| LEthan | <= |
| Assign | = |
| OrAssign | |= |
| AndAssign | &= |
| XORAssign | &= |
| LShiftAssign | <<= |
| RShiftAssign | >>= |
| Comma | , |
| Compl | ~ |
| Incr | ++ |
| Decr | -- |
| Not | ! |
| AddrOf | & |
| Deref | * |
| Function | () |
| Subscript | [] |
| Conversion | type() |

#### [`indirection.h`](https://github.com/Matthewacon/CX/blob/master/cx/indirection.h):
```cpp
#include <cx/indirection.h>

//Strip all type qualifiers from T
using resolver = CX::ComponentTypeResolver<T>;
//The number of pointer, reference and array qualifiers stripped off of T
constexpr const auto indirectionCount = resolver::indirectionCount;
//The number of const qualifications on type qualifiers, including the component type
constexpr const auto constCount = resolver::constCount;
//The stripped type
using componentType = resolver::type;

//Append a pointer qualifier to type T
using pointerQualified = CX::AppendPointer<T>::type;

//Append a reference qualifier to type T
using referenceQualified = CX::AppendReference<T>::type;

//Decompose template type T
using decomposer = CX::Decompose<T>
//The majority type for the type arguments of T
using majorityType = decomposer::type;
//The number of type arguments for template type T
constexpr const auto argc = decomposer::recursion_depth;
//True if the type arguments for the template type T are homogeneous
constexpr const auto isHomogeneous = decomposer::is_homogeneous;
```

#### [`templates.h`](https://github.com/Matthewacon/CX/blob/master/cx/templates.h):
```cpp
#include <cx/templates.h>

//Get the Nth type argument in Args...
using nthType = CX::TemplateTypeIterator<N, Args...>::type;

//Get the Nth non-type template argument in Args...
constexpr const auto nthValue = CX::TemplateAutoIterator<N, Args...>::value;

//Produces a specialized Target<...> with the template arguments Args... from 'Start' to 'End' inclusive
using resultantType = CX::TemplateTypeAssembler<Start, End, Target, Args...>::type;

//Same as TemplateTypeAssembler, but for non-type template arguments Args...
using resultantType = CX::TemplateAutoAssembler<Start, End, Target, Args...>::type;

//Produces a specialized Target<...> with the template parameters Args... right-shifted Cycle times, with 
//Crop arguments truncated, post cycling
using resultantType = CX::TempalteTypeArgCycler<Crop, Cycle, Target, Args...>::type;

//Same as TemplateTypeAssembler, but for non-type template arguments Args...
using resultantType = CX::TemplateAutoAssembler<Crop, Cycle, Target, Args...>::type;

//Produces Success if non-type template arguments Arg == Match
using resultantType = CX::select_if_match_T<Match, Arg, Success, Failure>::type;

//Same as select_if_match_T, but for non-type template arguments Success and Failure
constexpr const auto resultantValue = CX::select_if_match_A<Match, Arg, Success, Failure>::result;

//Produces Success if Arg is true
using resultantType = CX::select_if_true<Arg, Success, Failure>::type;
```

#### [`classes.h`](https://github.com/Matthewacon/CX/blob/master/cx/classes.h):
```cpp
#include <cx/classes.h>

//Produces the template type Target, specialized with Args... in reverse order
using resultantType = CX::ReverseSpecialize<Target, Args...>::type;

//True if T is a template template
constexpr const auto isTemplateTempate = CX::IsTemplateTemplate<T>::value;

//Produces a meta type that exposes two functions: `target_u_cast` and `target_d_cast` that upcast and
//downcast, respectively. This will prevent dynamic casts between distantly related types by explicitly casting
//to adjacent types, specified by Args... 
using explicitCastingRoute = CX::ExplicitCastGenerator<Target, Args..., End>;
//Upcast
End upCasted = explicitCastingRoute::target_u_cast(Target);
//Downcast
Target downCasted = explicitCastingRoute::target_d_cast(End);
```

#### [`tuple.h`](https://github.com/Matthewacon/CX/blob/master/include/tuple.h):
```cpp
#include <cx/tuple.h>

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

#### [`strings.h`](https://github.com/Matthewacon/CX/blob/master/cx/strings.h):
```cpp
#include <cx/strings.h>

//Produces a statically allocated c-string that is the concatenated result of Strings
constexpr const auto result = CX::Concat<const char *... Strings>::result; //or CX::concat<const char *... Strings>();

//Produces a statically allocated array of c-strings that is the delimited result of S (WIP)
constexpr const auto result = CX::Split<const char * S, const char Delimiter>::result; //or CX::split<const char * S, const char Delimiter>()

//Produces a statically allocated c-string that is the formatted result of Format and Args... (WIP)
//See printf specifiers for supported formatting specifiers
constexpr const auto result = CX::Format<const char * Format, auto... Args>::result; //or CX::format<const char * Format, auto... Args>()
```

#### [`array.h`](https://github.com/Matthewacon/CX/blob/master/cx/array.h):
This utility is still wip.

#### [`lambda.h`](https://github.com/Matthewacon/CX/blob/master/cx/lambda.h):
```cpp
#include <cx/lambda.h>

int main(int argc, char **argv) {
 CX::Lambda<int (int)> l = [&](int i) -> int {
  return argc + 1;
 };
 return l(10);
}
```

#### [`vararg.h`](https://github.com/Matthewacon/CX/blob/master/cx/vararg.h):
Why use `CX::va_list_t`? `va_list` is an implementation specific type, meaning that on non-standard systems
`va_list` may be defined as 
```c
typedef struct {
 //...
} va_list;
```
or
```c
typedef struct {
 //...
} va_list[1];
```
On systems implementing the latter, such as x86_64 and AMD64, reference semantics cannot be used. On systems
implementing the former, reference semantics CAN be used but doing so will result in platform specific code as systems
using the latter will not produce legal type definitions. Furthermore, on systems using the latter, `va_list`s cannot be
returned as values or references. `CX::va_list_t` solves all of these issues by using a portable internal wrapper type 
that encapsulates a `va_list` type with all qualifiers automatically stripped away. See the following examples:

This snippet of code produces different behaviour depending on the platform you're compiling for:
 - On x86_64, AMD64 and other platforms, `va_list` is defined as `va_list[1]` and implicitly converted to `va_list*` 
 when passed to `va_X` macros to that mutate state, meaning the principal list is always modified and no unintentional 
 copies are made.  
 - On AArch64 and other platforms, `va_list` is defined as a plain struct. All `va_list` arguments in the example below
 create unintentional copies of the principal `va_list`, causing every invocation to repeatedly consume the first
 argument from every copy. This can be prevented by declaring all `va_list` arguments as references, however, doing so
 will prevent you from compiling for platforms that do not define `va_list` as a plain struct.
```cpp
#include <cstdarg>
#include <cstdio>

void example1(int argc, va_list list);

void example2(int argc, va_list list) {
 if (argc > 0) {
  example1(argc - 1, list);
  printf("%s\n", va_arg(list, char *));
 }
}

void example1(int argc, va_list list) {
 if (argc > 0) {
  example2(argc - 1, list);
  printf("%s\n", va_arg(list, char *));
 }
}

void vararg(int argc, ...) {
 va_list list;
 va_start(list);
 example(argc, list);
 va_end(list);
}

int main() {
 vararg(3, "Hello", "World", "!");
 return 0;
}
```
Using `CX::va_list_t&` resolves of the aforementioned issues and produces portable code that behaves identically,
regardless of the platform that you are compiling for. Furthermore, `CX::safe_va_arg<T>(CX::va_list_t&)` performs static
checks on the type `T`, to prevent UB from arising at runtime.
```cpp
#include <cx/vararg.h>

#include <cstdarg>
#include <cstdio>

void example1(int argc, CX::va_list_t& list);

void example2(int argc, CX::va_list_t& list) {
 if (argc > 0) {
  example1(argc - 1, list);
  printf("%s\n", CX::safe_va_arg<char *>(list));
 }
}

void example1(int argc, CX::va_list_t& list) {
 if (argc > 0) {
  example2(argc - 1, list);
  printf("%s\n", CX::safe_va_arg<char *>(list));
 }
}

void vararg(int argc, ...) {
 CX::va_list_t list;
 va_start(list);
 example(argc, list);
 va_end(list);
}

int main() {
 vararg(3, "Hello", "World", "!");
 return 0;
}
```

#### [`unsafe.h`](https://github.com/Matthewacon/CX/blob/master/cx/unsafe.h):
```cpp
//common.h is not required to use unsafe.h, it is just used in this example
#include <cx/common.h>
#include <cx/unsafe.h>

char data[100];

struct Example {
 char d1[50];
 char d2[50];
} __attribute__((packed));

//Can be used to reinterpret memory as another type
//Does not check that sizeof(R) == sizeof(T) in CX::union_cast<R>(T)
auto reinterpreted = CX::union_cast<Example>(data);

struct Example2 {
 void func() {}
};

//Convert a member function pointer to a componentized struct
auto memberPtrComponents = CX::union_cast<CX::member_ptr_align_t>(&Example::func);

//Create an arbitrary member function pointer
auto fakePointer = CX::union_cast<void (Dummy<>::*)()>(CX::member_ptr_align_t{0xDEADBEEF, 0xCAFEBABE});
```

## License
This project is licensed under the [M.I.T. License](https://github.com/Matthewacon/CX/blob/master/LICENSE)
