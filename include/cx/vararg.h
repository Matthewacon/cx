#pragma once

#include <cx/common.h>
#include <cx/idioms.h>
#include <cx/error.h>

//Conditional stl dependencies if built with stl support
#ifdef CX_STL_SUPPORT
 #include <exception>
#endif

//TODO change conditions to use toolchain compiler defines
//Conditional libc dependencies if built with libc support
#ifdef CX_LIBC_SUPPORT
 #include <cstdarg>

 //Use default libc va_list macros
 #define CX_VA_START(list, arg) va_start(list, arg)
 #define CX_VA_END(list) va_end(list)
 #define CX_VA_ARG(list, type) va_arg(list, type)
#else
 //Compiler specific builtins
 #if defined(_MSC_VER)
  //MSVC support
  #define CX_VA_START(list, arg) __crt_va_start(list, arg)
  #define CX_VA_ARG(list, type) __crt_va_arg(list, type)
  #define CX_VA_END(list) __crt_va_end(list)
 #elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
  //Clang, GCC and ICC support
  #define CX_VA_START(list, arg) __builtin_va_start(list, arg)
  #define CX_VA_ARG(list, type) __builtin_va_arg(list, type)
  #define CX_VA_END(list) __builtin_va_end(list)
 #endif
#endif

//Support for va_list intrensics without libc
#ifdef CX_VARARG_INTRENSICS
 //Warn if libc macros are defined
 #if defined(va_start) || defined(va_end) || defined(va_arg)
  #error \
   'CX_VARARG_INTRENSICS' is enabled but one of [va_start va_end va_arg] \
   is already defined; are you sure you meant to enable this flag?
 #endif
 //Define libc va_list macros
 #define va_start(list, arg) CX_VA_START(list, arg)
 #define va_end(list) CX_VA_END(list)
 #define va_arg(list, type) CX_VA_ARG(list, type)
#endif

//Push diagnostic conetxt to silence gcc attribute parser bugs
#if defined(__GNUC__) && !defined(__clang__)
 #define CX_GCC
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wignored-attributes"
 #pragma GCC diagnostic ignored "-Wattributes"
#endif

namespace CX {
 struct InvalidPlatformVaListError CX_STL_SUPPORT_EXPR(: std::exception) {
  char const * message;

  InvalidPlatformVaListError(char const * message) :
   message(message)
  {}

  char const * what() const noexcept {
   return message;
  }
 };

 namespace Internal {
  //Default type promotions for va_list argument types
  namespace Meta {
   template<typename T, typename = ConstVolatileDecayed<T>>
   struct VarargPromoted {
    using Type = T;
   };

   template<typename T>
   requires (Integral<T> && sizeof(T) <= sizeof(int))
   struct VarargPromoted<T> {
    using Type = ConstVolatilePropagated<T, int>;
   };

   template<typename T>
   struct VarargPromoted<T, float> {
    using Type = ConstVolatilePropagated<T, double>;
   };
  }

  template<typename T>
  using VarargPromoted = typename Meta
   ::VarargPromoted<T>
   ::Type;

  //Note: `Unused` parameter prevents compiler errors from partial
  //specializations for other platforms, as they will never be
  //instantiated.
  template<auto Unused, typename T = va_list>
  struct VaListWrapper final {
   using UnderlyingListType = ImpossibleType<>;
   using PlatformListType = ImpossibleType<>;

   static_assert(
    !SameType<T, va_list>,
    "Your platform is using an unregistered 'va_list' type specialization. "
    "Please file a bug report to https://github.com/Matthewacon/CX so support "
    "can be added for your platform."
   );

   //Stub to prevent incomprehensible compiler backtraces
   operator va_list&() {
    return error<va_list&>(InvalidPlatformVaListError{
     "Unsupported platform"
    });
   }
  };

  //clang, gcc, icx: AArch64
  template<auto Unused>
  struct VaListWrapper<Unused, Unqualified<va_list>> final {
   using UnderlyingListType = Unqualified<va_list>;
   using PlatformListType = UnderlyingListType;

   UnderlyingListType list;

   VaListWrapper() = default;

   VaListWrapper(PlatformListType list) :
    list(list)
   {}

   ~VaListWrapper() {
    CX_VA_END(toPlatform());
   }

   [[gnu::always_inline]]
   PlatformListType& toPlatform() {
    return list;
   }

   [[gnu::always_inline]]
   operator PlatformListType&() {
    return toPlatform();
   }

   template<typename T>
   [[gnu::always_inline]]
   T arg() {
    using Promoted = VarargPromoted<T>;
    return (T)CX_VA_ARG(toPlatform(), Promoted);
   }
  };

  //clang, gcc, icx: AMD64
  template<auto Unused>
  struct VaListWrapper<Unused, Unqualified<va_list>[1]> {
   using UnderlyingListType = Unqualified<va_list>;
   using PlatformListType = UnderlyingListType *;

   UnderlyingListType list;
   PlatformListType platformList = &list;

   VaListWrapper() = default;

   VaListWrapper(PlatformListType list) :
    list([&] {
     if (!list) {
      error(InvalidPlatformVaListError{
       "Null platform va_list"
      });
     }
     return *list;
    }())
   {}

   ~VaListWrapper() {
    CX_VA_END(toPlatform());
   }

   [[gnu::always_inline]]
   PlatformListType& toPlatform() {
    return platformList;
   }

   [[gnu::always_inline]]
   operator PlatformListType&() {
    return toPlatform();
   }

   template<typename T>
   [[gnu::always_inline]]
   T arg() {
    using Promoted = VarargPromoted<T>;
    return (T)CX_VA_ARG(toPlatform(), Promoted);
   }
  };

  //clang, gcc, icx: x86
  //msvc: arm64, x64, x86
  template<auto Unused>
  struct VaListWrapper<Unused, char *> {
   using UnderlyingListType = char *;
   using PlatformListType = UnderlyingListType;

   UnderlyingListType list;

   VaListWrapper() = default;

   VaListWrapper(PlatformListType list) :
    list(list)
   {}

   ~VaListWrapper() {
    CX_VA_END(toPlatform());
   }

   [[gnu::always_inline]]
   PlatformListType& toPlatform() {
    return list;
   }

   [[gnu::always_inline]]
   operator PlatformListType&() {
    return toPlatform();
   }

   //`__crt_va_start(list, arg)` takes a reference to the `list` parameter
   //so the default implicit conversion operators do not work
   #ifdef CX_COMPILER_MSVC
    auto operator&() {
     return &toPlatform();
    }
   #endif

   template<typename T>
   [[gnu::always_inline]]
   T arg() {
    using Promoted = VarargPromoted<T>;
    return (T)CX_VA_ARG(toPlatform(), Promoted);
   }
  };
 }

 using VaList = Internal::VaListWrapper<0>;
}

//Pop diagnostic context
#ifdef CX_GCC
 #undef CX_GCC
 #pragma GCC diagnostic pop
#endif
