//Dependencies for supporting the libc allocator implementation
#ifdef CX_LIBC_SUPPORT
 #include <cstdlib>
#endif

//Dependencies for supporting compile-time allocators
#ifdef CX_STL_SUPPORT
 #include <memory>
#else
 #include <cx/common.h>

 //Define `std::allocator<T>` and `std::construct_at`
 //Note: Neither definitions are standard compliant, they are simply defined as
 //to satisfy the dependencies of the allocators defined in this header.
 //`std::allocator` and `std::construct_at` have relaxed restrictions from
 //[expr.const] seemingly permitting the following:
 // - allocations of untyped, arbitrarily-sized memory regions
 // - placement-new invocations on untyped memory regions allocated by
 //   `std::allocator<T>::allocate`
 // - placement-new invocations on arbitrary, untyped or correctly-typed,
 //   pointers so long as they point to sufficiently sized and aligned memory

 namespace std {
  template<typename T>
  struct allocator final {
   constexpr allocator() noexcept = default;
   constexpr ~allocator() noexcept = default;

   static constexpr T* allocate(SizeType const n) noexcept {
    constexpr std::nothrow_t no_throw;
    return static_cast<T *>(::operator new(n * sizeof(T), no_throw));
   }

   static constexpr void deallocate(T * p, SizeType) noexcept {
    constexpr std::nothrow_t no_throw;
    ::operator delete(p, no_throw);
   }

   template<typename T, typename... Args>
   constexpr T* construct_at(T * p, Args... args) noexcept {
    auto vp = const_cast<void *>(static_cast<const volatile void *>(p));
    return ::new (vp) T{(Args)args...};
   }
  };
 }
#endif

#include <cx/idioms.h>
#include <cx/error.h>
#include <cx/exit.h>

//Flags to configure default allocator behaviour
#if defined(CX_ALLOC_USER_IMPL)
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
#endif //defined(CX_ALLOC_USER_IMPL)

namespace CX {
 //Supporting types for allocator concepts
 namespace Internal {
  struct AllocatorTestType final : Never {};
 }

 //TODO Allocator identity concept
 template<template<typename...> typename MaybeAllocator>
 concept IsAllocator = true;

 //TODO Stateless allocator identity concept
 template<template<typename...> typename MaybeStatelessAllocator>
 concept IsStatelessAllocator = true;

 //TODO Stateful allocator identity concept
 template<template<typename...> typename MaybeStatefulAllocator>
 concept IsStatefulAllocator = true;

 //Define default allocator backends,
 namespace Internal {
  //STL-backed allocator implementation
  #ifdef CX_STL_SUPPORT
   template<typename T>
   struct StlAllocator final : Never {
    static constexpr bool const Stateless = true;

    constexpr StlAllocator() noexcept = default;
    constexpr ~StlAllocator() noexcept = default;

    //TODO Convert to `CX::Result`
    [[nodiscard]]
    static constexpr T& allocate(SizeType const n = 1) noexcept {
     auto val = std
      ::allocator<T>
      ::allocate(n);
     if (!val) {
      //TODO return error
      exit();
     }
     return *val;
    }

    //TODO Convert to return `Option<Error>` or `Result<void, Error>` and add
    //[[nodiscard]]
    static constexpr void deallocate(T const& t, SizeType const n = 1)
     noexcept
    {
     std
      ::allocator<T>
      ::deallocate(t, n);
    }
   };
   static_assert(IsAllocator<StlAllocator>
    && IsStatelessAllocator<StlAllocator>
   );
  #endif

  //LIBC-backed allocator implementation
  #ifdef CX_LIBC_SUPPORT
   //TODO Windows UCRT _aligned_malloc / _aligned_free support
   template<typename T>
   struct LibcAllocator final : Never {
    static constexpr bool const Stateless = true;

    constexpr LibcAllocator() noexcept = default;
    constexpr ~LibcAllocator() noexcept = default;

    //TODO Convert to CX::Result
    [[nodiscard]]
    static constexpr T& allocate(SizeType const n = 1) noexcept {
     if (isConstexpr()) {
      //Use constant-expression compatible logic for compile-time allocation
      auto ptr = std
       ::allocator<T>
       ::allocate(n);
      if (!ptr) {
       //TODO Appropriate error
       exit();
      }
      return *ptr;
     } else {
      //Use libc memory management logic
      T * ptr;
      auto const err = posix_memalign(&ptr, alignof(T), n * sizeof(T));
      if (err) {
       //TODO Appropriate errors for EINVAL and ENOMEM
       exit();
      }
      return *ptr;
     }
    }

    //TODO Convert to CX::Result
    static constexpr void deallocate(T const& t, SizeType const n) noexcept {
     if (isConstexpr()) {
      //Use constant-expression compatible logic at compile-time
      std
       ::allocator<T>
       ::deallocate(&t, n);
     } else {
      //Use libc memory management logic at runtime
      free(&t);
     }
    }
   };
   static_assert(IsAllocator<LibcAllocator>
    && IsStatelessAllocator<LibcAllocator>
   );
  #endif

  //Select default allocator
  #if CX_ALLOC_IMPL == 0
   //TODO Use user-defined allocator implementation as default
   // - Check against `IsAllocator`
   template<typename T>
   using DefaultAllocator = CX_ALLOC_USER_IMPL<T>;
  #elif CX_ALLOC_IMPL == 1
   //Use stl-backed allocator implementation as default
   template<typename T>
   using DefaultAllocator = StlAllocator<T>;
  #elif CX_ALLOC_IMPL == 2
   //Use libc-backed allocator implementation as default
   template<typename T>
   using DefaultAllocator = LibcAllocator<T>;
  #elif CX_ALLOC_IMPL == 3
   //TODO No default allocator impl, error on use
  #endif
 }

 //Alias template for CX default typed-allocator
 template<typename T>
 using Allocator = Internal::DefaultAllocator<T>;

 //Special allocator: stores instance internally and returns pointer to
 //internal instance on `::allocate` invocations.
 template<typename T>
 struct SinglePlacementAllocator final : Never {
 private:
  union {
   T t;
  };

 public:
  static constexpr bool const Stateless = false;

  constexpr SinglePlacementAllocator() noexcept {}
  constexpr ~SinglePlacementAllocator() noexcept {}

  //TODO Convert to CX::Result
  constexpr T& allocate(SizeType const n = 0) noexcept {
   if (n != 1) {
    //TODO Appropriate error
    exit();
   }
   return t;
  }

  //TODO Convert to CX::Result
  //Nop
  constexpr void deallocate(T const&) noexcept {}
 };
 static_assert(IsAllocator<SinglePlacementAllocator>
  && IsStatefulAllocator<SinglePlacementAllocator>
 );

 //Constant-evaluated placement new
 template<typename T, typename... Args>
 constexpr T& newInPlace(T& t, Args... args) noexcept {
  return *std::construct_at(&t, (Args)args...);
 }
}
