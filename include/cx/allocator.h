//Dependencies for supporting the libc allocator implementation
//Note: Included before any CX headers since STL headers depend on exceptions.
#ifdef CX_LIBC_SUPPORT
 #include <cstdlib>
#endif

#include <cx/common.h>

//Dependencies for supporting compile-time allocators
#ifndef CX_STL_SUPPORT
 //Define `std::allocator<T>` and `std::construct_at`
 //Note: Neither definitions are standard compliant, they are simply defined as
 //to satisfy the dependencies of the allocators defined in this header.
 //`std::allocator` and `std::construct_at` have relaxed restrictions from
 //[expr.const] seemingly permitting the following:
 // - allocations of untyped, arbitrarily-sized memory regions (must be
 //   implicitly or explicitly cast within the scope of the function to be
 //   considered valid?)
 // - placement-new invocations on, untyped or correctly-typed,
 //   pointers so long as they point to sufficiently sized and aligned memory

 namespace std {
  template<typename T>
  struct allocator final {
   constexpr allocator() noexcept = default;
   constexpr ~allocator() noexcept = default;

   constexpr T* allocate(SizeType const n) noexcept {
    constexpr std::nothrow_t no_throw;
    return static_cast<T *>(::operator new(n * sizeof(T), no_throw));
   }

   constexpr void deallocate(T * p, SizeType) noexcept {
    constexpr std::nothrow_t no_throw;
    ::operator delete(p, no_throw);
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
 //Stateless allocator identity concept
 template<template<typename...> typename MaybeAllocator>
 concept IsStatelessAllocator =
  //MaybeAllocator<Dummy<>>::Stateless &&
  requires (
   MaybeAllocator<Dummy<>>& a,
   Dummy<>& (* allocate)(SizeType) noexcept,
   void (* deallocate)(Dummy<> const&, SizeType) noexcept
  ) {
   a;
   allocate = &MaybeAllocator<Dummy<>>::allocate;
   deallocate = &MaybeAllocator<Dummy<>>::deallocate;
  };

 //Stateful allocator identity concept
 template<template<typename...> typename MaybeAllocator>
 concept IsStatefulAllocator =
  //!MaybeAllocator<Dummy<>>::Stateless &&
  requires (
   MaybeAllocator<Dummy<>>& a,
   Dummy<>& (MaybeAllocator<Dummy<>>::* allocate)(SizeType) noexcept,
   void (MaybeAllocator<Dummy<>>::* deallocate)(Dummy<> const&, SizeType) noexcept
  ) {
   a;
   allocate = &MaybeAllocator<Dummy<>>::allocate;
   deallocate = &MaybeAllocator<Dummy<>>::deallocate;
  };

 //Allocator identity concept
 template<template<typename...> typename MaybeAllocator>
 concept IsAllocator = IsStatefulAllocator<MaybeAllocator>
  || IsStatelessAllocator<MaybeAllocator>;

 //Error for `ConstexprAllocator`
 struct NotInConstantEvaluatedContextError final {
  constexpr auto& describe() const noexcept {
   return
    "ConstexprAllocator cannot be used outside of constant-evaluted contexts!";
  }
 };

 //Constant-evaluated allocator implementation
 //Note: Can only be used in constant-evaluated contexts
 template<typename T>
 struct ConstexprAllocator final : Never {
 private:
  //Abort if allocator use is attempted at runtime
  static constexpr void assertInConstantEvaluatedContext() noexcept {
   if (!isConstexpr()) {
    exit(NotInConstantEvaluatedContextError{});
   }
  }

 public:
  constexpr ConstexprAllocator() noexcept {
   assertInConstantEvaluatedContext();
  }

  constexpr ~ConstexprAllocator() noexcept {
   assertInConstantEvaluatedContext();
  }

  //TODO Convert to `CX::Result`
  [[nodiscard]]
  static constexpr T& allocate(SizeType const n = 1) noexcept {
   assertInConstantEvaluatedContext();
   auto val = std::allocator<T>{}
    .allocate(n);
   if (!val) {
    //TODO return error
    exit();
   }
   return *val;
  }

  //TODO Convert to `CX::Result`
  static constexpr void deallocate(T const& t, SizeType const n = 1) noexcept {
   assertInConstantEvaluatedContext();
   std::allocator<T>{}
    .deallocate(&const_cast<T&>(t), n);
  }
 };
 static_assert(IsStatelessAllocator<ConstexprAllocator>);

 //STL-backed allocator implementation
 #ifdef CX_STL_SUPPORT
  template<typename T>
  struct StlAllocator final : Never {
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
  static_assert(IsStatelessAllocator<StlAllocator>);
 #endif

 //LIBC-backed allocator implementation
 #ifdef CX_LIBC_SUPPORT
  //TODO Windows UCRT _aligned_malloc / _aligned_free support
  template<typename T>
  struct LibcAllocator final : Never {
   constexpr LibcAllocator() noexcept = default;
   constexpr ~LibcAllocator() noexcept = default;

   //TODO Convert to CX::Result
   [[nodiscard]]
   static constexpr T& allocate(SizeType const n = 1) noexcept {
    if (isConstexpr()) {
     //Use ConstexprAllocator for constant-evaluated allocations
     return ConstexprAllocator<T>::allocate(n);
    } else {
     //Use libc memory management logic at runtime
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
     //Use ConstexprAllocator for constant-evaluated deallocations
     ConstexprAllocator<T>::deallocate(t, n);
    } else {
     //Use libc memory management logic at runtime
     free(&t);
    }
   }
  };
  static_assert(IsStatelessAllocator<LibcAllocator>);
 #endif

 //Error for `NoneAllocator`
 struct NoneAllocatorError final {
  constexpr auto& describe() const noexcept {
   return
    "NoneAllocator cannot allocate memory. Either enable STL/LIBC support to "
    "inherit one of their allocator backends, or define your own using "
    "CX_ALLOC_USER_IMPL.";
  }
 };

 //Default allocator when no supporting libraries (stl / libc) are available to
 //provide an allocator backend
 template<typename T>
 struct NoneAllocator final : Never {
  constexpr NoneAllocator() noexcept = default;
  constexpr ~NoneAllocator() noexcept = default;

  //TODO Convert to CX::Result
  [[nodiscard]]
  static constexpr T& allocate(SizeType) noexcept {
   //TODO Return error instead
   exit(NoneAllocatorError{});
  }
 };

 //Alias template for CX default allocator
 #if CX_ALLOC_IMPL == 0
  //Use user-defined allocator implementation as default
  template<typename T>
  using Allocator = CX_ALLOC_USER_IMPL<T>;
 #elif CX_ALLOC_IMPL == 1
  //Use stl-backed allocator implementation as default
  template<typename T>
  using Allocator = StlAllocator<T>;
 #elif CX_ALLOC_IMPL == 2
  //Use libc-backed allocator implementation as default
  template<typename T>
  using Allocator = LibcAllocator<T>;
 #elif CX_ALLOC_IMPL == 3
  //No default allocator implementation, use `NoneAllocator`
  template<typename>
  using Allocator = NoneAllocator<T>;
 #endif
 static_assert(IsAllocator<Allocator>);

 //Constant indicating if current CX instance has a default allocator
 constexpr bool const HasDefaultAllocator = !SameType<
  Allocator<Dummy<>>,
  void
 >;

 //TODO Move to another header
 //Special allocator: stores instance internally and returns pointer to
 //internal instance on `::allocate` invocations.
 template<typename T>
 struct SinglePlacementAllocator final : Never {
 private:
  union {
   T t;
  };

 public:
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
  constexpr void deallocate(T const&, SizeType) noexcept {}
 };
 static_assert(IsStatefulAllocator<SinglePlacementAllocator>);
}
