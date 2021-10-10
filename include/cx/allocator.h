#include <cx/idioms.h>
#include <cx/error.h>

//Flags to configure default allocator behaviour
#if defined(CX_ALLOC_IMPL_USER)
 #define CX_ALLOC_IMPL 0
 CX_DEBUG_MSG((<cx/allocator.h> using user-defined allocator as default))
#elif defined(CX_STL_SUPPORT)
 #define CX_ALLOC_IMPL 1
 CX_DEBUG_MSG((<cx/allocator.h> using STL allocator as default))
#elif defined(CX_LIBC_SUPPORT)
 #define CX_ALLOC_IMPL 2
 CX_DEBUG_MSG((<cx/allocator.h> using LIBC allocator as default))
#else
 #define CX_ALLOC_IMPL 3
 CX_DEBUG_MSG((<cx/allocator.h> disabling default allocator))
#endif //defined(CX_ALLOC_IMPL_USER)

//TODO See `std::allocator` named requirements
namespace CX {
 //TODO Allocator identity concept
 template<template<typename...> typename MaybeAllocator>
 concept IsAllocator = false;

 //TODO Typed allocator identity concept
 template<template<typename...> typename MaybeTypedAllocator>
 concept IsTypedAllocator = false;

 //Define default allocator backends,
 namespace Internal {
  #if CX_ALLOC_IMPL == 0
   //TODO User-defined default allocator impl
   // - Check against `IsAllocator` and `IsTypedAllocator`
  #elif CX_ALLOC_IMPL == 1
   //TODO STL-backed default allocator impl
  #elif CX_ALLOC_IMPL == 2
   //TODO LIBC-backed default allocator impl
  #elif CX_ALLOC_IMPL == 3
   //TODO No default allocator impl, error on use
  #endif
 }

 template<typename T = void>
 using Allocator = void;

 //Example allocator
 /*
 template<typename T>
 struct UnionAllocator final {
 private:
  union {
   T inst;
   T * constexprInst;
  };

 public:
  constexpr UnionAllocator() noexcept = default;

  //TODO doc
  [[nodiscard]]
  constexpr T * allocate(SizeType const n) const noexcept {
   (void)n;
   return nullptr;
  }


  constexpr void deallocate(T * inst) const noexcept {
   if (isConstexpr()) {
    delete constexprInst;
   }
  }
 };
 */
}
