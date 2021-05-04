#pragma once

#include <cx/common.h>
#include <cx/idioms.h>
#include <cx/templates.h>
#include <cx/error.h>

//Conditional stl dependencies if CX was built with stl support enabled
#ifdef CX_STL_SUPPORT
 #include <memory>
#else
 #pragma message \
  "Building without STL support; 'CX::AllocLambda<...>' will be unavailable."
#endif

#if defined(CX_LAMBDA_BUF_ALIGN) || defined(CX_LAMBDA_BUF_SIZE)
 #pragma message \
  "Using user-specified buffer size and alignment for non-allocating "\
  "polymorphic function wrapper implementation."

 //Check that both 'CX_LAMBDA_BUF_ALIGN' and 'CX_LAMBDA_BUF_SIZE'
 //were specified together
 #ifndef CX_LAMBDA_BUF_ALIGN
  #error \
   "You must specify 'CX_LAMBDA_BUF_ALIGN' when overriding "\
   "'CX_LAMBDA_BUF_SIZE'"
 #endif

 #ifdef CX_LAMBDA_BUF_SIZE
  #error \
   "You must specify 'CX_LAMBDA_BUF_SIZE' when overriding "\
   "'CX_LAMBDA_BUF_ALIGN'"
 #endif

 //Check that the user-specified values are valid
 #if CX_LAMBDA_BUF_ALIGN < CX_LAMBDA_BUF_SIZE
  #error \
   "'CX_LAMBDA_BUF_ALIGN' must be greater than or equal to "\
   "'CX_LAMBDA_BUF_SIZE'"
 #endif
#else
 #define CX_LAMBDA_BUF_ALIGN (sizeof(void *) * 8)
 #define CX_LAMBDA_BUF_SIZE (sizeof(void *) * 8)
#endif

namespace CX {
 //Supporting exceptions
 struct UninitializedLambdaError : CXError {
  UninitializedLambdaError() :
   CXError{"Lambda is uninitialized"}
  {}
 };

 struct IncompatibleLambdaError : CXError {
  using CXError::CXError;
 };

 //Non-allocating lambda
 template<typename>
 struct Lambda;

 //Allocating lambda
 template<typename>
 struct AllocLambda;

 //Lambda meta-functions
 namespace LambdaMetaFunctions {
  namespace Internal {
   template<template<typename> typename T>
   concept IsLambdaTemplate = CX::MatchAnyTemplateType<
    T,
    Lambda,
    AllocLambda
   >;
  }

  template<typename>
  struct IsLambda : FalseType {};

  template<template<typename> typename L, typename R, typename... Args>
  struct IsLambda<L<R (Args...)>> {
   static constexpr auto const Value = Internal::IsLambdaTemplate<L>;
  };

  template<template<typename> typename L, typename R, typename... Args>
  struct IsLambda<L<R (Args...) noexcept>> : IsLambda<L<R (Args...)>> {};

  template<template<typename> typename L, typename R, typename... Args>
  struct IsLambda<L<R (Args..., ...)>> : IsLambda<L<R (Args...)>> {};

  template<template<typename> typename L, typename R, typename... Args>
  struct IsLambda<L<R (Args..., ...) noexcept>> : IsLambda<L<R (Args...)>> {};

  template<typename>
  struct IsNonAllocLambda : FalseType {};

  template<template<typename> typename L, typename Prototype>
  struct IsNonAllocLambda<L<Prototype>> : MetaFunctions::SameTemplateType<L, Lambda> {};

  template<typename>
  struct IsAllocLambda : FalseType {};

  template<template<typename> typename L, typename Prototype>
  struct IsAllocLambda<L<Prototype>> : MetaFunctions::SameTemplateType<L, AllocLambda> {};

  //True conditions:
  // - qualified lambda -> unqualified lambda (both alloc and non-alloc)
  // - no-alloc lambda <-> alloc lambda
  template<typename, typename>
  struct CompatibleLambda : FalseType {};

  //Matching lambda prototype
  template<
   template<typename> typename L1,
   typename Prototype1,
   template<typename> typename L2,
   typename Prototype2
  >
  requires (Internal::IsLambdaTemplate<L1> && Internal::IsLambdaTemplate<L2>)
  struct CompatibleLambda<L1<Prototype1>, L2<Prototype2>> {
   static constexpr auto const Value = CX::SameType<Prototype1, Prototype2>;
  };

  //Noexcept-qualified lambda to unqualified lambda
  template<
   template<typename> typename L1,
   typename Prototype1,
   template<typename> typename L2,
   typename Prototype2
  >
  requires (Internal::IsLambdaTemplate<L1>
   && Internal::IsLambdaTemplate<L2>
   && !CX::NoexceptFunction<Prototype1>
   && CX::NoexceptFunction<Prototype2>
  )
  struct CompatibleLambda<L1<Prototype1>, L2<Prototype2>> : TrueType {};
 }

 //Lambda identity
 template<typename T>
 concept IsLambda = LambdaMetaFunctions
  ::IsLambda<ConstVolatileDecayed<T>>
  ::Value;

 //Non-alloc Lambda identity
 template<typename T>
 concept IsNonAllocLambda =
  LambdaMetaFunctions
   ::IsLambda<ConstVolatileDecayed<T>>
   ::Value
  && LambdaMetaFunctions
   ::IsNonAllocLambda<ConstVolatileDecayed<T>>
   ::Value;

 //AllocLambda identity
 template<typename T>
 concept IsAllocLambda =
  LambdaMetaFunctions
   ::IsLambda<ConstVolatileDecayed<T>>
   ::Value
  && LambdaMetaFunctions
   ::IsAllocLambda<ConstVolatileDecayed<T>>
   ::Value;

 //Compatible lambda types identity (see above)
 template<typename L2, typename L1>
 concept CompatibleLambda = LambdaMetaFunctions
  ::CompatibleLambda<ConstVolatileDecayed<L1>, ConstVolatileDecayed<L2>>
  ::Value;

 //Lambda utilities
 namespace Internal {
  //Utility functions
  [[gnu::always_inline]]
  inline void wrapperBufferCheck(
   SizeType srcSize,
   AlignType srcAlignment,
   SizeType dstSize,
   AlignType dstAlignment
  ) {
   //Check that the receiving buffer is sized properly
   if (dstSize < srcSize) {
    error(IncompatibleLambdaError{
     "Receiving lambda does not have a large enough buffer to contain "
     "the assigned lambda's buffer"
    });
   }

   //Check that the receiving buffer is aligned properly
   if (dstAlignment < srcAlignment) {
    error(IncompatibleLambdaError{
     "Receiving lambda's buffer is not aligned properly to receive "
     "the assigned lambda's buffer"
    });
   }
  }

  //The base of all lambda buffers
  template<typename F>
  struct LambdaBase;

  //Base of unqualified and `noexcept` qualified function types
  template<typename R, typename... Args>
  struct LambdaBase<R (Args...)> {
   template<typename>
   friend struct LambdaBase;

   template<typename>
   friend struct NonAllocLambdaWrapperStub;

   template<typename>
   friend struct AllocLambdaWrapperStub;

   template<typename, typename>
   friend struct NonAllocLambdaWrapper;

   template<typename, typename>
   friend struct AllocLambdaWrapper;

  private:
   LambdaBase() = default;

  public:
   virtual ~LambdaBase() = default;

   [[gnu::always_inline]]
   virtual R op(Args...) {
    return error<R>(UninitializedLambdaError{});
   }

   [[gnu::always_inline]]
   virtual R noexceptOp(Args...) noexcept {
    return error<R>(UninitializedLambdaError{});
   }

   virtual bool allocates() const noexcept = 0;
   virtual SizeType typeSize() const noexcept = 0;
   virtual AlignType typeAlignment() const noexcept = 0;
   virtual SizeType bufferSize() const noexcept = 0;
   virtual AlignType bufferAlignment() const noexcept = 0;
   //(bufPtr, size, alignment, allocating?)
   virtual void copyTo(
    void **,
    SizeType,
    AlignType,
    bool
   ) = 0;

   [[gnu::always_inline]]
   virtual bool present() const noexcept {
    return false;
   }
  };

  //Base of unqualified and `noexcept` qualified c-variadic function types
  template<typename R, typename... Args>
  struct LambdaBase<R (Args..., ...)> {
   template<typename>
   friend struct LambdaBase;

   template<typename>
   friend struct NonAllocLambdaWrapperStub;

   template<typename>
   friend struct AllocLambdaWrapperStub;

   template<typename, typename>
   friend struct NonAllocLambdaWrapper;

   template<typename, typename>
   friend struct AllocLambdaWrapper;

  private:
   LambdaBase() = default;

  public:
   using FuncType = R (Args..., ...);
   using NoexceptFuncType = R (Args..., ...) noexcept;

   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
   #pragma GCC diagnostic ignored "-Wnested-anon-types"
    struct FptrWrapper final {
     void * fptrOrInst;
     struct MemberPtr final {
      union {
       void (MemberPtr::* memberPtr)();
       struct {
        void
         * lo,
         * hi;
       };
      };

      MemberPtr() :
       lo(nullptr),
       hi(nullptr)
      {}

      template<CX::MemberFunction F>
      MemberPtr(F memFptr) :
       memberPtr((decltype(memberPtr))memFptr)
      {}
     } memberPtr;
    #pragma GCC diagnostic pop

    FptrWrapper(CX::StaticFunction auto ptr) :
     fptrOrInst((void *)ptr)
    {}

    template<CX::Struct T>
    FptrWrapper(T const &inst) :
     fptrOrInst((void *)&inst),
     memberPtr{&T::operator()}
    {}

    template<bool Noexcept, typename... Varargs>
    [[gnu::always_inline]]
    R invoke(Args... args, Varargs... varargs) noexcept(Noexcept) {
     if (memberPtr.lo || memberPtr.hi) {
      //Invoke member function
      auto const ptr = (R (FptrWrapper::*)(Args..., ...) noexcept(Noexcept))memberPtr.memberPtr;
      return (((FptrWrapper *)fptrOrInst)->*ptr)(args..., varargs...);
     } else {
      //Invoke static function
      return ((R (*)(Args..., ...) noexcept(Noexcept))fptrOrInst)(args..., varargs...);
     }
    }
   };

   virtual ~LambdaBase() = default;

   virtual FptrWrapper get() {
    return error<FptrWrapper>(UninitializedLambdaError{});
   }

   template<bool Noexcept, typename... Varargs>
   [[gnu::always_inline]]
   R op(Args... args, Varargs... varargs) noexcept(Noexcept) {
    return get().template invoke<Noexcept, Varargs...>(args..., varargs...);
   }

   virtual bool allocates() const noexcept = 0;
   virtual SizeType typeSize() const noexcept = 0;
   virtual AlignType typeAlignment() const noexcept = 0;
   virtual SizeType bufferSize() const noexcept = 0;
   virtual AlignType bufferAlignment() const noexcept = 0;
   //(bufPtr, size, alignment, allocating?)
   virtual void copyTo(
    void **,
    SizeType,
    AlignType,
    bool
   ) = 0;

   [[gnu::always_inline]]
   virtual bool present() const noexcept {
    return false;
   }
  };

  //Implementation of `LambdaBase<...>` for uninitialized
  //non-allocating lambdas
  template<typename Prototype>
  struct NonAllocLambdaWrapperStub;

  //Implementation of `LambdaBase<...>` for uninitialized
  //allocating lambdas
  template<typename Prototype>
  struct AllocLambdaWrapperStub;

  //Implementation of `LambdaBase<...>` for non-allocating lambdas
  template<typename, typename>
  struct NonAllocLambdaWrapper;

  //Implementation of `LambdaBase<...>` for allocating lambdas
  template<typename, typename>
  struct AllocLambdaWrapper;

  //Check `F` against a restriction
  template<typename F, template<typename> typename Restriction>
  concept SupportedPrototype = requires (Restriction<F> r) {
   r;
  };

  //LambdaOperationBase implementation; provides implementations for all
  //underlying lambda mechanisms, for all specializations.
  template<
   //The static function prototype of the invoking lambda specialization
   typename SpecializationPrototype,
   //Prototype restrictions for the invoking lambda specialization
   template<typename> typename Restriction
  >
  struct LambdaOperationBase {
  private:
   template<bool Allocating>
   using EmptyWrapperType = CX::SelectType<
    Allocating,
    AllocLambdaWrapperStub<SpecializationPrototype>,
    NonAllocLambdaWrapperStub<SpecializationPrototype>
   >;

   template<
    bool Allocating,
    typename F,
    typename Prototype
   >
   using WrapperType = CX::SelectType<
    Allocating,
    AllocLambdaWrapper<F, Prototype>,
    NonAllocLambdaWrapper<F, Prototype>
   >;

   template<typename>
   friend struct NonAllocLambdaWrapperStub;

   template<typename>
   friend struct AllocLambdaWrapperStub;

   template<typename, typename>
   friend struct NonAllocLambdaWrapper;

   template<typename, typename>
   friend struct AllocLambdaWrapper;

   //Checks an `AllocLambda<...>` buffer against size and alignment
   //requirements; re-allocates buffer if the buffer does not meet the
   //requirements
   #ifdef CX_STL_SUPPORT
    template<typename Wrapper>
    [[gnu::always_inline]]
    static void checkOrReallocate(
     void ** buffer,
     SizeType &size,
     AlignType &alignment
    ) {
     constexpr auto const
      wrapperSize = sizeof(Wrapper),
      wrapperAlignment = alignof(Wrapper);

     if (size == 0 && alignment == 0) {
      //Note: `size == 0 && alignment == 0` is a special case
      //for the initial `AllocLambda` buffer allocation. Will
      //try to use `CX_LAMBDA_BUF_SIZE` and `CX_LAMBDA_BUF_ALIGN`
      //for the allocation, to minimize the number of
      //re-allocations.

      //Determine new buffer properties
      size = std::max(CX_LAMBDA_BUF_SIZE, wrapperSize);
      alignment = std::max(CX_LAMBDA_BUF_ALIGN, wrapperAlignment);

      //Allocate new buffer
      *buffer = operator new(
       size,
       (std::align_val_t)alignment
      );
     } else if (size < wrapperSize || alignment < wrapperAlignment) {
      //If the buffer does not meet the `wrapper` size and
      //alignment requirements, allocate new buffer with
      //requiements met
      *buffer = operator new(
       wrapperSize,
       (std::align_val_t)wrapperAlignment
      );

      //Set new buffer properties
      size = wrapperSize;
      alignment = wrapperAlignment;
     }
    }
   #endif //CX_STL_SUPPORT

   //Copy-constructs or default constructs and copy-assigns
   //`Wrapper` with `Function`, inside the provided buffer,
   //`buf`
   template<typename Wrapper, typename Function>
   requires (CopyConstructible<Function>
    || (Constructible<Function> && CopyAssignable<Function>)
   )
   [[gnu::always_inline]]
   static void copyInitWrapper(
    void * buf,
    SizeType bufSize,
    AlignType bufAlignment,
    Function const &func
   ) {
    //Initialize buffer
    if constexpr (CopyConstructible<Function>) {
     //If `Function` is copy constructible, invoke wrapper
     //function copy constructor
     new (buf) Wrapper{(Function const&)func, bufSize, bufAlignment};
    } else if constexpr(Constructible<Function> && CopyAssignable<Function>) {
     //If `Function` is default constructible and copy assignable,
     //default construct wrapper and copy assign to function
     (*new (buf) Wrapper{bufSize, bufAlignment}).f = (Function const&)func;
    }
   }

   //Move-constructs or default constructs and move-assigns
   //`Wrapper` with `Function`, inside the provided buffer,
   //`buf`
   template<typename Wrapper, typename Function>
   requires (MoveConstructible<Function>
    || (Constructible<Function> && MoveAssignable<Function>)
   )
   [[gnu::always_inline]]
   static void moveInitWrapper(
    void * buf,
    SizeType bufSize,
    AlignType bufAlignment,
    Function &&func
   ) {
    //Initialize buffer
    if constexpr(MoveConstructible<Function>) {
     //If `Function` is move constructible, invoke wrapper
     //function move constructor
     new (buf) Wrapper{(Function&&)func, bufSize, bufAlignment};
    } else if constexpr (Constructible<Function> && MoveAssignable<Function>) {
     //If `Function` is default constructible and move assignable,
     //default construct wrapper and move assign to function
     (*new (buf) Wrapper{bufSize, bufAlignment}).f = (Function&&)func;
    }
   }

   //Common function for copying lambda wrappers to new buffers
   template<
    typename AllocWrapper,
    typename NonAllocWrapper,
    typename... F
   >
   requires (sizeof...(F) < 2)
   [[gnu::always_inline]]
   static void copyWrapperCommon(
    void ** dstBufPtr,
    SizeType dstBufSize,
    AlignType dstBufAlignment,
    bool dstAllocating,
    F... f
   ) {
    using FUnqualified = ConstDecayed<
     ReferenceDecayed<
      TypeAtIndex<0, F...>
     >
    >;

    //Initialize dst buffer with given wrapper type,
    //buffer properties and (if specified) `f`
    auto const init = [&]<typename Wrapper>() {
     if constexpr (MatchAnyType<
      Wrapper,
      AllocLambdaWrapper<FUnqualified, SpecializationPrototype>,
      NonAllocLambdaWrapper<FUnqualified, SpecializationPrototype>
     >) {
      //Initialize alloc / non-alloc lambda wrapper
      copyInitWrapper<Wrapper>(
       *dstBufPtr,
       dstBufSize,
       dstBufAlignment,
       (F)f...
      );
     } else {
      //Initialize alloc / non-alloc stub wrapper
      new (*dstBufPtr) Wrapper {
       dstBufSize,
       dstBufAlignment
      };
     }
    };

    if (!dstAllocating) {
     //Copy current buffer to `NonAllocLambdaWrapper*<...>` buffer
     wrapperBufferCheck(
      sizeof(NonAllocWrapper),
      alignof(NonAllocWrapper),
      dstBufSize,
      dstBufAlignment
     );

     init.template operator()<NonAllocWrapper>();
    } else {
     //Copy current buffer to `AllocLambdaWrapper*<...>` buffer
     #ifdef CX_STL_SUPPORT
      checkOrReallocate<AllocWrapper>(
       dstBufPtr,
       dstBufSize,
       dstBufAlignment
      );

      init.template operator()<AllocWrapper>();
     #endif //CX_STL_SUPPORT
    }
   }

   //Common function for `copyAssignFunction` and `moveAssignFunction`
   template<
    typename F,
    bool Copy,
    IsLambda L
   >
   requires (SupportedPrototype<
    ConstDecayed<ReferenceDecayed<F>>,
    Restriction
   >)
   [[gnu::always_inline]]
   static void assignFunctionCommon(L &l, F f) {
    using WrapperSpecialization = WrapperType<
     IsAllocLambda<L>,
     ConstDecayed<ReferenceDecayed<F>>,
     SpecializationPrototype
    >;

    void ** lBufPtr;
    SizeType lSize;
    AlignType lAlignment;

    //Perform assignment through `op`
    auto const assign = [&]() {
     //Fetch buffer size and alignment
     {
      auto &lBase = **(LambdaBase<SpecializationPrototype> **)lBufPtr;
      lSize = lBase.bufferSize();
      lAlignment = lBase.bufferAlignment();
     }

     //Check buffer before initializing
     if constexpr (IsNonAllocLambda<L>) {
      //Check non-allocated buffer
      wrapperBufferCheck(
       sizeof(WrapperSpecialization),
       alignof(WrapperSpecialization),
       lSize,
       lAlignment
      );
     } else {
      //Check allcoated buffer
      #ifdef CX_STL_SUPPORT
       checkOrReallocate<WrapperSpecialization>(
        lBufPtr,
        lSize,
        lAlignment
       );
      #endif //CX_STL_SUPPORT
     }

     //Clean up `l`'s buffer
     destroy(*lBufPtr);

     //Copy or move initialize `WrapperType`
     if constexpr (Copy) {
      copyInitWrapper<WrapperSpecialization>(
       *lBufPtr,
       lSize,
       lAlignment,
       (F)f
      );
     } else {
      moveInitWrapper<WrapperSpecialization>(
       *lBufPtr,
       lSize,
       lAlignment,
       (F)f
      );
     }
    };

    //Deal with `AllocLambda<...>`
    #ifdef CX_STL_SUPPORT
     if constexpr (IsAllocLambda<L>) {
      allocLambdaBufferOp(l.buffer, [&](void ** ptr) {
       lBufPtr = ptr;
       assign();
      });
      return;
     }
    #endif //CX_STL_SUPPORT

    //Deal with `Lambda<...>`
    if constexpr (IsNonAllocLambda<L>) {
     void * ptr = &l.buf();
     lBufPtr = &ptr;
     assign();
     return;
    }
   }

   //Common function for `copyAssignLambda` and `moveAssignLambda`
   template<
    typename L2,
    bool Copy,
    typename L1
   >
   requires (CompatibleLambda<
    ConstDecayed<ReferenceDecayed<L2>>,
    L1
   >)
   [[gnu::always_inline]]
   static void assignLambdaCommon(L1 &l1, L2 l2) {
    constexpr auto const
     l1Alloc = IsAllocLambda<L1>,
     l2Alloc = IsAllocLambda<
      ConstDecayed<ReferenceDecayed<L2>>
     >;

    //Detmine whether or not `l2`'s ref-counted buffer can be copied
    //to `l1`'s ref-counted buffer
    constexpr bool fastCopy
     #ifdef CX_STL_SUPPORT
      = l1Alloc && l2Alloc;
     #else
      = false;
     #endif //CX_STL_SUPPORT

    void
     ** l1BufPtr,
     ** l2BufPtr;
    SizeType
     l1Size,
     l2Size;
    AlignType
     l1Alignment,
     l2Alignment;

    auto const assign = [&] {
     //Fetch `l1` size and alignment
     {
      auto &l1Base = **(LambdaBase<SpecializationPrototype> **)l1BufPtr;
      l1Size = l1Base.bufferSize();
      l1Alignment = l1Base.bufferAlignment();
     }

     //Clean up `l1` state
     destroy(*l1BufPtr);

     //Fetch `l2` size and alignment and copy `l2` buffer to
     //`l1` buffer
     {
      auto &l2Base = **(LambdaBase<SpecializationPrototype> **)l2BufPtr;

      //Fetch `l2` size and alignment
      l2Size = l2Base.bufferSize();
      l2Alignment = l2Base.bufferAlignment();

      //Copy `l2` buffer to `l1` buffer
      if constexpr (!fastCopy) {
       //Slow copy; copy-initialize wrapper
       l2Base.copyTo(
        l1BufPtr,
        l1Size,
        l1Alignment,
        l1Alloc
       );
      } else {
       //Fast copy; copy-assign underlying ref-counted buffer
       #ifdef CX_STL_SUPPORT
        l1.buffer = l2.buffer;
       #endif //CX_STL_SUPPORT
      }
     }

     //If move requested, destroy `l2` buffer and re-initialize
     //as empty lambda
     if constexpr (!Copy) {
      if constexpr (!l2Alloc) {
       //Clean up `l2` state
       destroy(*l2BufPtr);
      } else {
       #ifdef CX_STL_SUPPORT
        //Clean up `l2` state
        l2.buffer.reset();
       #endif //CX_STL_SUPPORT
      }

      //Re-initialize `l2` buffer as empty lambda
      //Note: `Allocating` template parameter does not matter
      //here
      LambdaOperationBase<SpecializationPrototype, Restriction>
       ::initEmptyLambda(l2);
     }
    };

    //Prepare `l2` buffer pointer
    auto const l2Prep = [&] {
     if constexpr (!l2Alloc) {
      void * l2Buf = &l2.buf();
      l2BufPtr = &l2Buf;
      assign();
      return;
     } else {
      #ifdef CX_STL_SUPPORT
       [&] {
        void * l2Ptr = l2.buffer.get();
        l2BufPtr = &l2Ptr;
        assign();
       }();
      #endif //CX_STL_SUPPORT
      return;
     }
    };

    //Prepare `l1` buffer pointer
    if constexpr (!l1Alloc) {
     void * l1Buf = &l1.buf();
     l1BufPtr = &l1Buf;
     l2Prep();
     return;
    } else {
     #ifdef CX_STL_SUPPORT
      if constexpr (!l2Alloc) {
       //`l1` buffer may be re-allocated; use `allocLambdaBufferOp` context
       allocLambdaBufferOp(l1.buffer, [&](void ** l1Ptr) {
        l1BufPtr = l1Ptr;
        l2Prep();
       });
      } else {
       //Fast-copy will be used, `l1` buffer will not be re-allocated
       [&] {
        void * l1Ptr = l1.buffer.get();
        l1BufPtr = &l1Ptr;
        l2Prep();
       }();
      }
     #endif //CX_STL_SUPPORT
     return;
    }
   }

  public:
   //Utility function for non-alloc lambda; `Lambda<...>`
   //Ensure `T` will fit within a buffer of size `Size` with
   //alignment `Alignment`
   template<typename T, auto Size, auto Alignment>
   [[gnu::always_inline]]
   static void lambdaAssignBufferCheck() {
    //Note: Add `alignof(void *)` to account for internal
    //requirements
    static_assert(
     alignof(NonAllocLambdaWrapper<T, SpecializationPrototype>) <= Alignment,
     "The required alignment of the given functor is larger than the "
     "current 'CX_LAMBDA_BUF_ALIGN' value. Consider increasing it to "
     "the next power of 2."
    );
    //Note: Add `sizeof(void *)` to account for vtable ptr
    static_assert(
     alignof(NonAllocLambdaWrapper<T, SpecializationPrototype>) <= Size,
     "The size of the given functor is larger than the current "
     "'CX_LAMBDA_BUF_SIZE' value. Consider increasing it to the next "
     "power of 2."
    );
   }

   //Utilities for `AllocLambda`
   #ifdef CX_STL_SUPPORT
    //Aligned deleter for `AllocLambda<...>`'s underlying `shared_ptr`
    //buffer
    static void sharedBufferDeleter(unsigned char * ptr) {
     if (ptr) {
      auto &base = *(LambdaBase<SpecializationPrototype> *)ptr;
      auto const alignment = base.bufferAlignment();

      //Destruct buffer
      destroy(ptr);

      //Delete aligned buffer
      operator delete(ptr, (std::align_val_t)alignment);
     }
    };

    //Buffer manipulation pattern utility
    static void allocLambdaBufferOp(std::shared_ptr<unsigned char> &buf, auto op) {
     unsigned char * bufPtr = buf.get();
     op((void **)&bufPtr);

     //If `bufPtr` was re-allocated, reset `buf`
     if (bufPtr != buf.get()) {
      buf.reset(bufPtr, sharedBufferDeleter);
     }
    }
   #endif //CX_STL_SUPPORT

   //Initializes `l`'s buffer with empty `LambdaBase` obj
   template<IsLambda L>
   [[gnu::always_inline]]
   static void initEmptyLambda(L &l) {
    using EmptyWrapperType = EmptyWrapperType<IsAllocLambda<L>>;
    SizeType size;
    AlignType alignment;

    auto const init = [&](void ** lBufPtr) {
     if constexpr (IsAllocLambda<L>) {
      //Set buffer properties
      size = 0;
      alignment = 0;

      //Allocate aligned buffer for `AllocLambda`
      #ifdef CX_STL_SUPPORT
       //Allocate aligned buffer
       checkOrReallocate<EmptyWrapperType>(
        lBufPtr,
        size,
        alignment
       );
      #endif //CX_STL_SUPPORT
     } else {
      //Set buffer properties
      size = CX_LAMBDA_BUF_SIZE;
      alignment = CX_LAMBDA_BUF_ALIGN;

      //Check buffer constraints for non-alloc `Lambda`
      //Ensure receiving buffer is compatible
      wrapperBufferCheck(
       sizeof(EmptyWrapperType),
       alignof(EmptyWrapperType),
       CX_LAMBDA_BUF_SIZE,
       CX_LAMBDA_BUF_ALIGN
      );
     }

     //Initialize buffer
     new (*lBufPtr) EmptyWrapperType {
      size,
      alignment
     };
    };

    if constexpr (IsNonAllocLambda<L>) {
     void * buf = &l.buf();
     init(&buf);
    } else {
     #ifdef CX_STL_SUPPORT
      allocLambdaBufferOp(l.buffer, [&](void ** lBufPtr) {
       init(lBufPtr);
      });
     #endif //CX_STL_SUPPORT
    }
   }

   //Destructs current `LambdaBase` obj in buffer
   [[gnu::always_inline]]
   static void destroy(void * buffer) {
    using LambdaBase = LambdaBase<SpecializationPrototype>;
    (*(LambdaBase *)buffer).~LambdaBase();
   }

   //Resets a lambda buffer to an empty `LambdaBase` object
   template<IsLambda L>
   [[gnu::always_inline]]
   static void reset(L &l) {
    //Destroy lambda buffer
    if constexpr (IsNonAllocLambda<L>) {
     destroy(&l.buf());
    } else {
     #ifdef CX_STL_SUPPORT
      destroy(l.buffer.get());
     #endif
    }
    //Re-initialize lambda buffer
    initEmptyLambda(l);
   }

   //FunctionOperator move assignment
   template<SupportedPrototype<Restriction> F>
   requires (CopyConstructible<F>
    || (Constructible<F> && CopyAssignable<F>)
   )
   [[gnu::always_inline]]
   static void copyAssignFunction(IsLambda auto &l, F const &f) {
    assignFunctionCommon<F const&, true>(l, f);
   }

   //FunctionOperator / function pointer copy assignment
   template<SupportedPrototype<Restriction> F>
   requires (MoveConstructible<F>
    || (Constructible<F> && MoveAssignable<F>)
   )
   [[gnu::always_inline]]
   static void moveAssignFunction(IsLambda auto &l, F &&f) {
    assignFunctionCommon<F&&, false>(l, (F&&)f);
   }

   //Lambda copy assignment
   template<typename L1, typename L2>
   requires (CompatibleLambda<L2, L1>)
   [[gnu::always_inline]]
   static void copyAssignLambda(L1 &l1, L2 const &l2) {
    assignLambdaCommon<L2 const&, true>(l1, (L2 const&)l2);
   }

   //Lambda move assignment
   template<typename L1, typename L2>
   requires (!Const<L2> && CompatibleLambda<L2, L1>)
   [[gnu::always_inline]]
   static void moveAssignLambda(L1 &l1, L2 &l2) {
    assignLambdaCommon<L2&, false>(l1, (L2&)l2);
   }
  };

  //Stub implementation of `LambdaBase<...>` for all
  //lambda prototypes; default instance for uninitialized
  //non-allocating lambdas
  template<typename Prototype>
  struct NonAllocLambdaWrapperStub : LambdaBase<Prototype> {
  private:
   using LambdaBase = LambdaBase<Prototype>;

   //Note: Use `CX::Dummy<...>` as the restriction since it
   //will pass all parameters; at this point, the encapsulated
   //type, `F`, has already been checked against the expected
   //restriction, at compile-time.
   template<typename>
   using Restriction = Dummy<>;

   using OperationBase = LambdaOperationBase<Prototype, Restriction>;

   SizeType bufSize;
   AlignType bufAlignment;

  public:
   NonAllocLambdaWrapperStub(
    SizeType size,
    AlignType alignment
   ) :
    LambdaBase{},
    bufSize(size),
    bufAlignment(alignment)
   {}

   bool allocates() const noexcept override {
    return false;
   }

   SizeType typeSize() const noexcept override {
    return sizeof(NonAllocLambdaWrapperStub);
   }

   AlignType typeAlignment() const noexcept override {
    return alignof(NonAllocLambdaWrapperStub);
   }

   SizeType bufferSize() const noexcept override {
    return bufSize;
   }

   AlignType bufferAlignment() const noexcept override {
    return bufAlignment;
   }

   void copyTo(
    void ** dstBufPtr,
    SizeType dstBufSize,
    AlignType dstBufAlignment,
    bool allocating
   ) override {
    OperationBase::template copyWrapperCommon<
     AllocLambdaWrapperStub<Prototype>,
     NonAllocLambdaWrapperStub<Prototype>
    >(
     dstBufPtr,
     dstBufSize,
     dstBufAlignment,
     allocating
    );
   }
  };

  //Checks to ensure that the non-allocated lambda buffers are
  //sufficiently sized and aligned for internal overhead
  static_assert(
   CX_LAMBDA_BUF_ALIGN >= alignof(NonAllocLambdaWrapperStub<void ()>),
   "'CX_LAMBDA_BUF_ALIGN' must be at least as large as "
   "`alignof(NonAllocLambdaWrapperStub<void ()>)` to compensate for "
   "internal overhead."
  );
  static_assert(
   CX_LAMBDA_BUF_SIZE >= sizeof(NonAllocLambdaWrapperStub<void ()>),
   "'CX_LAMBDA_BUF_SIZE' must be at least as large as "
   "`sizeof(NonAllocLambdaWrapperStub<void ()>)` to compensate for "
   "internal overhead."
  );

  //`AllocLambda` stub wrapper implementation
  #ifdef CX_STL_SUPPORT
   //Stub implementation of `LambdaBase<...>` for all
   //lambda prototypes; default instance for uninitialized
   //allocating lambdas
   template<typename Prototype>
   struct AllocLambdaWrapperStub : NonAllocLambdaWrapperStub<Prototype> {
   private:
    using LambdaBase = LambdaBase<Prototype>;

    //Note: Use `CX::Dummy<...>` as the restriction since it
    //will pass all parameters; at this point, the encapsulated
    //type, `F`, has already been checked against the expected
    //restriction, at compile-time.
    template<typename>
    using Restriction = Dummy<>;

    using OperationBase = LambdaOperationBase<Prototype, Restriction>;

   public:
    using NonAllocLambdaWrapperStub<Prototype>::NonAllocLambdaWrapperStub;

    bool allocates() const noexcept override {
     return true;
    }

    SizeType typeSize() const noexcept override {
     return sizeof(AllocLambdaWrapperStub);
    }

    AlignType typeAlignment() const noexcept override {
     return alignof(AllocLambdaWrapperStub);
    }

    void copyTo(
     void ** dstBufPtr,
     SizeType dstBufSize,
     AlignType dstBufAlignment,
     bool allocating
    ) override {
     OperationBase::template copyWrapperCommon<
      AllocLambdaWrapperStub<Prototype>,
      NonAllocLambdaWrapperStub<Prototype>
     >(
      dstBufPtr,
      dstBufSize,
      dstBufAlignment,
      allocating
     );
    }
   };
  #endif //CX_STL_SUPPORT

  //Lambda wrapper for `F` (non-c-variadic)
  template<typename F, typename R, typename... Args>
  struct NonAllocLambdaWrapper<F, R (Args...)> : LambdaBase<R (Args...)> {
  private:
   using Prototype = R (Args...);
   using LambdaBase = LambdaBase<Prototype>;

   //Note: Use `CX::Dummy<...>` as the restriction since it
   //will pass all parameters; at this point, the encapsulated
   //type, `F`, has already been checked against the expected
   //restriction, at compile-time.
   template<typename>
   using Restriction = Dummy<>;

   using OperationBase = LambdaOperationBase<Prototype, Restriction>;

  public:
   F f;

   //Default constructor
   NonAllocLambdaWrapper(
    SizeType,
    AlignType
   ) :
    LambdaBase{}
   {};

   //Function copy constructor
   //Note: size and alignment arguments are unused for the
   //non-allocating lambda specialization
   NonAllocLambdaWrapper(
    F const &f,
    SizeType,
    AlignType
   ) :
    LambdaBase{},
    f((F const&)f)
   {}

   //Function move constructor
   NonAllocLambdaWrapper(
    F &&f,
    SizeType,
    AlignType
   ) :
    LambdaBase{},
    f((F&&)f)
   {}

   [[gnu::always_inline]]
   R op(Args... args) override {
    return f(args...);
   }

   [[gnu::always_inline]]
   R noexceptOp(Args... args) noexcept override {
    return f(args...);
   }

   bool allocates() const noexcept override {
    return false;
   }

   SizeType typeSize() const noexcept override {
    return sizeof(NonAllocLambdaWrapper);
   }

   AlignType typeAlignment() const noexcept override {
    return alignof(NonAllocLambdaWrapper);
   }

   SizeType bufferSize() const noexcept override {
    return CX_LAMBDA_BUF_SIZE;
   }

   AlignType bufferAlignment() const noexcept override {
    return CX_LAMBDA_BUF_ALIGN;
   }

   void copyTo(
    void ** dstBufPtr,
    SizeType dstBufSize,
    AlignType dstBufAlignment,
    bool allocating
   ) override {
    OperationBase::template copyWrapperCommon<
     AllocLambdaWrapper<F, Prototype>,
     NonAllocLambdaWrapper<F, Prototype>
    >(
     dstBufPtr,
     dstBufSize,
     dstBufAlignment,
     allocating,
     (F&)f
    );
   }

   [[gnu::always_inline]]
   bool present() const noexcept override {
    return true;
   }
  };

  //Lambda wrapper for `F` (c-variadic)
  template<typename F, typename R, typename... Args>
  struct NonAllocLambdaWrapper<F, R (Args..., ...)> : LambdaBase<R (Args..., ...)> {
  private:
   using Prototype = R (Args..., ...);
   using LambdaBase = LambdaBase<Prototype>;

   //Note: Use `CX::Dummy<...>` as the restriction since it
   //will pass all parameters; at this point, the encapsulated
   //type, `F`, has already been checked against the expected
   //restriction, at compile-time.
   template<typename>
   using Restriction = Dummy<>;

   using OperationBase = LambdaOperationBase<Prototype, Restriction>;

  public:
   F f;

   //Default constructor
   NonAllocLambdaWrapper(
    SizeType,
    AlignType
   ) :
    LambdaBase{}
   {};

   //Function copy constructor
   NonAllocLambdaWrapper(
    F const &f,
    SizeType,
    AlignType
   ) :
    LambdaBase{},
    f((F const&)f)
   {}

   //Function move constructor
   NonAllocLambdaWrapper(
    F &&f,
    SizeType,
    AlignType
   ) :
    LambdaBase{},
    f((F&&)f)
   {}

   [[gnu::always_inline]]
   typename LambdaBase::FptrWrapper get() override {
    return {(F const&)f};
   }

   bool allocates() const noexcept override {
    return false;
   }

   SizeType typeSize() const noexcept override {
    return sizeof(NonAllocLambdaWrapper);
   }

   AlignType typeAlignment() const noexcept override {
    return alignof(NonAllocLambdaWrapper);
   }

   SizeType bufferSize() const noexcept override {
    return CX_LAMBDA_BUF_SIZE;
   }

   AlignType bufferAlignment() const noexcept override {
    return CX_LAMBDA_BUF_ALIGN;
   }

   void copyTo(
    void ** dstBufPtr,
    SizeType dstBufSize,
    AlignType dstBufAlignment,
    bool allocating
   ) override {
    OperationBase::template copyWrapperCommon<
     AllocLambdaWrapper<F, Prototype>,
     NonAllocLambdaWrapper<F, Prototype>
    >(
     dstBufPtr,
     dstBufSize,
     dstBufAlignment,
     allocating,
     (F&)f
    );
   }

   [[gnu::always_inline]]
   bool present() const noexcept override {
    return true;
   }
  };

  //`AllocLambda` wrapper implementations
  #ifdef CX_STL_SUPPORT
   //AllocLambda wrapper for `F` (non-c-variadic)
   template<typename F, typename R, typename... Args>
   struct AllocLambdaWrapper<F, R (Args...)> : LambdaBase<R (Args...)> {
   private:
    using Prototype = R (Args...);
    using LambdaBase = LambdaBase<Prototype>;

    //Note: Use `CX::Dummy<...>` as the restriction since it
    //will pass all parameters; at this point, the encapsulated
    //type, `F`, has already been checked against the expected
    //restriction, at compile-time.
    template<typename>
    using Restriction = Dummy<>;

    using OperationBase = LambdaOperationBase<Prototype, Restriction>;

    F f;
    SizeType bufSize;
    AlignType bufAlignment;

   public:
    //Default constructor
    AllocLambdaWrapper(
     SizeType size,
     AlignType alignment
    ) :
     LambdaBase{},
     bufSize(size),
     bufAlignment(alignment)
    {};

    //Function copy constructor
    AllocLambdaWrapper(
     F const &f,
     SizeType size,
     AlignType alignment
    ) :
     LambdaBase{},
     f((F const&)f),
     bufSize(size),
     bufAlignment(alignment)
    {}

    //Function move constructor
    AllocLambdaWrapper(
     F &&f,
     SizeType size,
     AlignType alignment
    ) :
     LambdaBase{},
     f((F&&)f),
     bufSize(size),
     bufAlignment(alignment)
    {}

    [[gnu::always_inline]]
    R op(Args... args) override {
     return f(args...);
    }

    [[gnu::always_inline]]
    R noexceptOp(Args... args) noexcept override {
     return f(args...);
    }

    bool allocates() const noexcept override {
     return true;
    }

    SizeType typeSize() const noexcept override {
     return sizeof(AllocLambdaWrapper);
    }

    AlignType typeAlignment() const noexcept override {
     return alignof(AllocLambdaWrapper);
    }

    SizeType bufferSize() const noexcept override {
     return bufSize;
    }

    AlignType bufferAlignment() const noexcept override {
     return bufAlignment;
    }

    void copyTo(
     void ** dstBufPtr,
     SizeType dstBufSize,
     AlignType dstBufAlignment,
     bool allocating
    ) override {
     OperationBase::template copyWrapperCommon<
      AllocLambdaWrapper<F, Prototype>,
      NonAllocLambdaWrapper<F, Prototype>
     >(
      dstBufPtr,
      dstBufSize,
      dstBufAlignment,
      allocating,
      (F&)f
     );
    }

    [[gnu::always_inline]]
    bool present() const noexcept override {
     return true;
    }
   };

   //AllocLambda wrapper for `F` (c-variadic)
   template<typename F, typename R, typename... Args>
   struct AllocLambdaWrapper<F, R (Args..., ...)> : LambdaBase<R (Args..., ...)> {
   private:
    using Prototype = R (Args..., ...);
    using LambdaBase = LambdaBase<Prototype>;

    //Note: Use `CX::Dummy<...>` as the restriction since it
    //will pass all parameters; at this point, the encapsulated
    //type, `F`, has already been checked against the expected
    //restriction, at compile-time.
    template<typename>
    using Restriction = Dummy<>;

    using OperationBase = LambdaOperationBase<Prototype, Restriction>;

    F f;
    SizeType bufSize;
    AlignType bufAlignment;

   public:
    //Default constructor
    AllocLambdaWrapper(
     SizeType size,
     AlignType alignment
    ) :
     LambdaBase{},
     bufSize(size),
     bufAlignment(alignment)
    {};

    //Function copy constructor
    AllocLambdaWrapper(
     F const &f,
     SizeType size,
     AlignType alignment
    ) :
     LambdaBase{},
     f((F const&)f),
     bufSize(size),
     bufAlignment(alignment)
    {}

    //Function move constructor
    AllocLambdaWrapper(
     F &&f,
     SizeType size,
     AlignType alignment
    ) :
     LambdaBase{},
     f((F&&)f),
     bufSize(size),
     bufAlignment(alignment)
    {}

    [[gnu::always_inline]]
    typename LambdaBase::FptrWrapper get() override {
     return {(F const&)f};
    }

    bool allocates() const noexcept override {
     return true;
    }

    SizeType typeSize() const noexcept override {
     return sizeof(AllocLambdaWrapper);
    }

    AlignType typeAlignment() const noexcept override {
     return alignof(AllocLambdaWrapper);
    }

    SizeType bufferSize() const noexcept override {
     return bufSize;
    }

    AlignType bufferAlignment() const noexcept override {
     return bufAlignment;
    }

    void copyTo(
     void ** dstBufPtr,
     SizeType dstBufSize,
     AlignType dstBufAlignment,
     bool allocating
    ) override {
     OperationBase::template copyWrapperCommon<
      AllocLambdaWrapper<F, Prototype>,
      NonAllocLambdaWrapper<F, Prototype>
     >(
      dstBufPtr,
      dstBufSize,
      dstBufAlignment,
      allocating,
      (F&)f
     );
    }

    [[gnu::always_inline]]
    bool present() const noexcept override {
     return true;
    }
   };
  #endif //CX_STL_SUPPORT
 }

 //Unqualified lambda specialization
 template<typename R, typename... Args>
 struct Lambda<R (Args...)> {
  template<typename>
  friend struct Lambda;

  template<typename>
  friend struct AllocLambda;

  template<
   typename,
   template<typename> typename
  >
  friend struct Internal::LambdaOperationBase;

  using ReturnType = R;
  template<template<typename...> typename Receiver = Dummy>
  using ArgumentTypes = Receiver<Args...>;
  using FunctionType = R (Args...);

  static constexpr auto const Alignment = CX_LAMBDA_BUF_ALIGN;
  static constexpr auto const Size = CX_LAMBDA_BUF_SIZE;

 private:
  template<typename F>
  requires (FunctionWithPrototype<F, R (Args...)>
   || FunctionWithPrototype<F, R (Args...) noexcept>
   || FunctionWithPrototype<F, R (F::*)(Args...)>
   || FunctionWithPrototype<F, R (F::*)(Args...) const>
   || FunctionWithPrototype<F, R (F::*)(Args...) noexcept>
   || FunctionWithPrototype<F, R (F::*)(Args...) const noexcept>
  )
  struct PrototypeRestriction {};

  template<typename F>
  static constexpr bool const SupportedPrototype = Internal::SupportedPrototype<
   F,
   PrototypeRestriction
  >;

  using OperationBase = Internal::LambdaOperationBase<
   R (Args...),
   PrototypeRestriction
  >;
  using LambdaBase = Internal::LambdaBase<R (Args...)>;

  alignas(Alignment) unsigned char buffer[Size];

  //Return mutable reference to `buffer`
  auto& buf() const noexcept {
   return const_cast<Lambda&>(*this).buffer;
  }

 public:
  //Default constructor
  Lambda() {
   //Default initialize buffer for empty lambda
   OperationBase::initEmptyLambda(const_cast<Lambda&>(*this));
  }

  //FunctionOperator / function pointer copy constructor
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda(F const &f) : Lambda() {
   operator=<F>((F const&)f);
  }

  //FunctionOperator move constructor
  template<typename F>
  requires (!IsLambda<F>
   && !Const<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda(F &&f) : Lambda() {
   operator=<F>((F&&)f);
  }

  //Lambda copy constructor
  Lambda(Lambda const &l) : Lambda() {
   operator=<Lambda>((Lambda const&)l);
  }

  //Lambda move constructor
  Lambda(Lambda &&l) : Lambda() {
   operator=<Lambda>((Lambda&&)l);
  }

  //CompatibleLambda copy constructor
  template<CompatibleLambda<Lambda> L>
  Lambda(L const &l) : Lambda() {
   operator=<L>((L const&)l);
  }

  //CompatibleLambda move constructor
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda(L &&l) : Lambda() {
   operator=<L>((L&&)l);
  }

  ~Lambda() {
   //Destruct current buffer (guaranteed to be initialized)
   OperationBase::destroy(&buf());
  }

  //Lambda function operator
  R operator()(Args... args) const {
   return (*(LambdaBase *)&buffer).op(args...);
  }

  //Presence conversion operator
  operator bool() const noexcept {
   return (*(LambdaBase *)&buffer).present();
  }

  //Resets current lambda buffer state to empty
  void reset() {
   OperationBase::reset(*this);
  }

  //Implicit lambda conversion operator
  template<CompatibleLambda<Lambda> L>
  explicit operator L() const {
   return L{*this};
  }

  //FunctionOperator / function pointer copy assignment
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda& operator=(F const &f) {
   //Ensure `F` will fit within `buffer`
   OperationBase::template lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::copyAssignFunction(*this, (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<typename F>
  requires (!IsLambda<F>
   && !Const<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda& operator=(F &&f) {
   //Ensure `F` will fit within `buffer`
   OperationBase::template lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::moveAssignFunction(*this, (F&&)f);

   return *this;
  }

  //Lambda copy assignment
  Lambda& operator=(Lambda const &l) {
   return operator=<Lambda>((Lambda const&)l);
  }

  //Lambda move assignment
  Lambda& operator=(Lambda &&l) {
   return operator=<Lambda>((Lambda&&)l);
  }

  //CompatibleLambda copy assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(L const &l) {
   OperationBase::copyAssignLambda(*this, l);
   return *this;
  }

  //CompatibleLambda move assignment
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda& operator=(L &&l) {
   OperationBase::moveAssignLambda(*this, l);
   return *this;
  }
 };

 //Noexcept qualified lambda specialization
 template<typename R, typename... Args>
 struct Lambda<R (Args...) noexcept> {
  template<typename>
  friend struct Lambda;

  template<typename>
  friend struct AllocLambda;

  template<
   typename,
   template<typename> typename
  >
  friend struct Internal::LambdaOperationBase;

  using ReturnType = R;
  template<template<typename...> typename Receiver = Dummy>
  using ArgumentTypes = Receiver<Args...>;
  using FunctionType = R (Args...) noexcept;

  static constexpr auto const Alignment = CX_LAMBDA_BUF_ALIGN;
  static constexpr auto const Size = CX_LAMBDA_BUF_SIZE;

 private:
  template<typename F>
  requires (FunctionWithPrototype<F, R (Args...) noexcept>
   || FunctionWithPrototype<F, R (F::*)(Args...) noexcept>
   || FunctionWithPrototype<F, R (F::*)(Args...) const noexcept>
  )
  struct PrototypeRestriction {};

  template<typename F>
  static constexpr bool const SupportedPrototype = Internal::SupportedPrototype<
   F,
   PrototypeRestriction
  >;

  using OperationBase = Internal::LambdaOperationBase<
   R (Args...),
   PrototypeRestriction
  >;
  using LambdaBase = Internal::LambdaBase<R (Args...)>;

  alignas(Alignment) unsigned char buffer[Size];

  //Return mutable reference to `buffer`
  auto& buf() const noexcept {
   return const_cast<Lambda&>(*this).buffer;
  }

 public:
  //Default constructor
  Lambda() {
   //Default initialize buffer for empty lambda
   OperationBase::initEmptyLambda(const_cast<Lambda&>(*this));
  }

  //FunctionOperator / function pointer copy constructor
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda(F const &f) : Lambda() {
   operator=<F>((F const&)f);
  }

  //FunctionOperator move constructor
  template<typename F>
  requires (!IsLambda<F>
   && !Const<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda(F &&f) : Lambda() {
   operator=<F>((F&&)f);
  }

  //Lambda copy constructor
  Lambda(Lambda const &l) : Lambda() {
   operator=<Lambda>((Lambda const&)l);
  }

  //Lambda move constructor
  Lambda(Lambda &&l) : Lambda() {
   operator=<Lambda>((Lambda&&)l);
  }

  //CompatibleLambda copy constructor
  template<CompatibleLambda<Lambda> L>
  Lambda(L const &l) : Lambda() {
   operator=<L>((L const&)l);
  }

  //CompatibleLambda move constructor
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda(L &&l) : Lambda() {
   operator=<L>((L&&)l);
  }

  ~Lambda() {
   //Destruct current buffer (guaranteed to be initialized)
   OperationBase::destroy(&buf());
  }

  //Lambda function operator
  R operator()(Args... args) noexcept {
   return (*(LambdaBase *)&buffer).noexceptOp(args...);
  }

  //Presence conversion operator
  operator bool() const noexcept {
   return (*(LambdaBase *)&buffer).present();
  }

  //Resets current lambda buffer state to empty
  void reset() {
   OperationBase::reset(*this);
  }

  //Implicit lambda conversion operator
  template<CompatibleLambda<Lambda> L>
  explicit operator L() const {
   return {*this};
  }

  //FunctionOperator / function pointer copy assignment
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda& operator=(F const &f) {
   //Ensure `F` will fit within `buffer`
   OperationBase::template lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::copyAssignFunction(*this, (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<typename F>
  requires (!IsLambda<F>
   && !Const<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda& operator=(F &&f) {
   //Ensure `F` will fit within `buffer`
   OperationBase::template lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::moveAssignFunction(*this, (F&&)f);

   return *this;
  }

  //Lambda copy assignment
  Lambda& operator=(Lambda const &l) {
   return operator=<Lambda>((Lambda const&)l);
  }

  //Lambda move assignment
  Lambda& operator=(Lambda &&l) {
   return operator=<Lambda>((Lambda&&)l);
  }

  //CompatibleLambda copy assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(L const &l) {
   OperationBase::copyAssignLambda(*this, l);
   return *this;
  }

  //CompatibleLambda move assignment
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda& operator=(L &&l) {
   OperationBase::moveAssignLambda(*this, l);
   return *this;
  }
 };

 //Unqualified c-variadic lambda specialization
 template<typename R, typename... Args>
 struct Lambda<R (Args..., ...)> {
  template<typename>
  friend struct Lambda;

  template<typename>
  friend struct AllocLambda;

  template<
   typename,
   template<typename> typename
  >
  friend struct Internal::LambdaOperationBase;

  using ReturnType = R;
  template<template<typename...> typename Receiver = Dummy>
  using ArgumentTypes = Receiver<Args...>;
  using FunctionType = R (Args..., ...);

  static constexpr auto const Alignment = CX_LAMBDA_BUF_ALIGN;
  static constexpr auto const Size = CX_LAMBDA_BUF_SIZE;

 private:
  template<typename F>
  requires (FunctionWithPrototype<F, R (Args..., ...)>
   || FunctionWithPrototype<F, R (Args..., ...) noexcept>
   || FunctionWithPrototype<F, R (F::*)(Args..., ...)>
   || FunctionWithPrototype<F, R (F::*)(Args..., ...) const>
   || FunctionWithPrototype<F, R (F::*)(Args..., ...) noexcept>
   || FunctionWithPrototype<F, R (F::*)(Args..., ...) const noexcept>
  )
  struct PrototypeRestriction {};

  template<typename F>
  static constexpr bool const SupportedPrototype = Internal::SupportedPrototype<
   F,
   PrototypeRestriction
  >;

  using OperationBase = Internal::LambdaOperationBase<
   R (Args..., ...),
   PrototypeRestriction
  >;
  using LambdaBase = Internal::LambdaBase<R (Args..., ...)>;

  alignas(Alignment) unsigned char buffer[Size];

  //Return mutable reference to `buffer`
  auto& buf() const noexcept {
   return const_cast<Lambda&>(*this).buffer;
  }

 public:
  //Default constructor
  Lambda() {
   //Default initialize buffer for empty lambda
   OperationBase::initEmptyLambda(const_cast<Lambda&>(*this));
  }

  //FunctionOperator / function pointer copy constructor
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda(F const &f) : Lambda() {
   operator=<F>((F const&)f);
  }

  //FunctionOperator move constructor
  template<typename F>
  requires (!IsLambda<F>
   && !Const<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda(F &&f) : Lambda() {
   operator=<F>((F &&)f);
  }

  //Lambda copy constructor
  Lambda(Lambda const &l) : Lambda() {
   operator=<Lambda>((Lambda const&)l);
  }

  //Lambda move constructor
  Lambda(Lambda &&l) : Lambda() {
   operator=<Lambda>((Lambda&&)l);
  }

  //CompatibleLambda copy constructor
  template<CompatibleLambda<Lambda> L>
  Lambda(L const &l) : Lambda() {
   operator=<L>((L const&)l);
  }

  //CompatibleLambda move constructor
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda(L &&l) : Lambda() {
   operator=<L>((L&&)l);
  }

  ~Lambda() {
   //Destruct current buffer (guaranteed to be initialized)
   OperationBase::destroy(buf());
  }

  //Lambda function operator
  template<typename... Varargs>
  R operator()(Args... args, Varargs... varargs) {
   return (*(LambdaBase *)&buffer).template op<false>(args..., varargs...);
  }

  //Presence conversion operator
  operator bool() const noexcept {
   return (*(LambdaBase *)&buffer).present();
  }

  //Resets current lambda buffer state to empty
  void reset() {
   OperationBase::reset(*this);
  }

  //Implicit lambda conversion operator
  template<CompatibleLambda<Lambda> L>
  explicit operator L() const {
   L{*this};
  }

  //FunctionOperator / function pointer copy assignment
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda& operator=(F const &f) {
   //Ensure `F` will fit within `buffer`
   OperationBase::template lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::copyAssignFunction(*this, (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<typename F>
  requires (!IsLambda<F>
   && !Const<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda& operator=(F &&f) {
   //Ensure `F` will fit within `buffer`
   OperationBase::template lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::moveAssignFunction(*this, (F&&)f);

   return *this;
  }

  //Lambda copy assignment
  Lambda& operator=(Lambda const &l) {
   return operator=<Lambda>((Lambda const&)l);
  }

  //Lambda move assignment
  Lambda& operator=(Lambda &&l) {
   return operator=<Lambda>((Lambda&&)l);
  }

  //CompatibleLambda copy assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(L const &l) {
   OperationBase::copyAssignLambda(*this, l);
   return *this;
  }

  //CompatibleLambda move assignment
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda& operator=(L &&l) {
   OperationBase::moveAssignLambda(*this, l);
   return *this;
  }
 };

 //Noexcept qualified c-variadic lambda specialization
 template<typename R, typename... Args>
 struct Lambda<R (Args..., ...) noexcept> {
  template<typename>
  friend struct Lambda;

  template<typename>
  friend struct AllocLambda;

  template<
   typename,
   template<typename> typename
  >
  friend struct Internal::LambdaOperationBase;

  using ReturnType = R;
  template<template<typename...> typename Receiver = Dummy>
  using ArgumentTypes = Receiver<Args...>;
  using FunctionType = R (Args..., ...) noexcept;

  static constexpr auto const Alignment = CX_LAMBDA_BUF_ALIGN;
  static constexpr auto const Size = CX_LAMBDA_BUF_SIZE;

 private:
  template<typename F>
  requires (FunctionWithPrototype<F, R (Args..., ...) noexcept>
   || FunctionWithPrototype<F, R (F::*)(Args..., ...) noexcept>
   || FunctionWithPrototype<F, R (F::*)(Args..., ...) const noexcept>
  )
  struct PrototypeRestriction {};

  template<typename F>
  static constexpr bool const SupportedPrototype = Internal::SupportedPrototype<
   F,
   PrototypeRestriction
  >;

  using OperationBase = Internal::LambdaOperationBase<
   R (Args..., ...),
   PrototypeRestriction
  >;
  using LambdaBase = Internal::LambdaBase<R (Args..., ...)>;

  alignas(Alignment) unsigned char buffer[Size];

  //Return mutable reference to `buffer`
  auto& buf() const noexcept {
   return const_cast<Lambda&>(*this).buffer;
  }

 public:
  //Default constructor
  Lambda() {
   //Default initialize buffer for empty lambda
   OperationBase::initEmptyLambda(const_cast<Lambda&>(*this));
  }

  //FunctionOperator / function pointer copy constructor
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda(F const &f) : Lambda() {
   operator=<F>((F const&)f);
  }

  //FunctionOperator move constructor
  template<typename F>
  requires (!IsLambda<F>
   && !Const<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda(F &&f) : Lambda() {
   operator=<F>((F&&)f);
  }

  //Lambda copy constructor
  Lambda(Lambda const &l) : Lambda() {
   operator=<Lambda>((Lambda const&)l);
  }

  //Lambda move constructor
  Lambda(Lambda &&l) : Lambda() {
   operator=<Lambda>((Lambda&&)l);
  }

  //CompatibleLambda copy constructor
  template<CompatibleLambda<Lambda> L>
  Lambda(L const &l) : Lambda() {
   operator=<L>((L const &)l);
  }

  //CompatibleLambda move constructor
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda(L &&l) {
   operator=<L>((L&&)l);
  }

  ~Lambda() {
   //Destruct current buffer (guaranteed to be initialized)
   OperationBase::destroy(buf());
  }

  //Lambda function operator
  template<typename... Varargs>
  R operator()(Args... args, Varargs... varargs) {
   return (*(LambdaBase *)&buffer).template op<true>(args..., varargs...);
  }

  //Presence conversion operator
  operator bool() const noexcept {
   return (*(LambdaBase *)&buffer).present();
  }

  //Resets current lambda buffer state to empty
  void reset() {
   OperationBase::reset(*this);
  }

  //Explicit lambda conversion operator
  template<CompatibleLambda<Lambda> L>
  explicit operator L() const {
   return L{*this};
  }

  //FunctionOperator / function pointer copy assignment
  template<typename F>
  requires (!IsLambda<F> && SupportedPrototype<F>)
  Lambda& operator=(F const &f) {
   //Ensure `F` will fit within `buffer`
   OperationBase::template lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::copyAssignFunction(*this, (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<typename F>
  requires (!IsLambda<F>
   && !Const<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda& operator=(F &&f) {
   //Ensure `F` will fit within `buffer`
   OperationBase::template lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::moveAssignFunction(*this, (F&&)f);

   return *this;
  }

  //Lambda copy assignment
  Lambda& operator=(Lambda const &l) {
   return operator=<Lambda>((Lambda const&)l);
  }

  //Lambda move assignment
  Lambda& operator=(Lambda &&l) {
   return operator=<Lambda>((Lambda&&)l);
  }

  //CompatibleLambda copy assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(L const &l) {
   OperationBase::copyAssignLambda(*this, l);
   return *this;
  }

  //CompatibleLambda move assignment
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda& operator=(L &&l) {
   OperationBase::moveAssignLambda(*this, l);
   return *this;
  }
 };

 //Deduction guides for Lambda
 Lambda() -> Lambda<void ()>;

 template<StaticFunction F>
 Lambda(F const&) -> Lambda<FunctionPrototype<F>>;

 template<typename F>
 requires (!IsLambda<F> && FunctionOperator<F, decltype(&F::operator())>)
 Lambda(F const&) -> Lambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename F>
 requires (StaticFunction<F>)
 Lambda(F&&) -> Lambda<FunctionPrototype<F>>;

 template<typename F>
 requires (!IsLambda<F> && FunctionOperator<F, decltype(&F::operator())>)
 Lambda(F&&) -> Lambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename L>
 requires (IsLambda<L>)
 Lambda(L const&) -> Lambda<typename Unqualified<L>::FunctionType>;

 template<typename L>
 requires (IsLambda<L>)
 Lambda(L&&) -> Lambda<typename Unqualified<L>::FunctionType>;

 //AllocLambda implementation
 #ifdef CX_STL_SUPPORT
  //Unqualified lambda specialization
  template<typename R, typename... Args>
  struct AllocLambda<R (Args...)> {
   template<typename>
   friend struct Lambda;

   template<typename>
   friend struct AllocLambda;

   template<
    typename,
    template<typename> typename
   >
   friend struct Internal::LambdaOperationBase;

   using ReturnType = R;
   template<template<typename...> typename Receiver = Dummy>
   using ArgumentTypes = Receiver<Args...>;
   using FunctionType = R (Args...);

  private:
   //Note: Reuse prototype restrictions from `Lambda<R (Args...)>`
   template<typename F>
   using PrototypeRestriction = typename Lambda<R (Args...)>
    ::template PrototypeRestriction<F>;

   template<typename F>
   static constexpr bool const SupportedPrototype = Internal::SupportedPrototype<
    F,
    PrototypeRestriction
   >;

   using OperationBase = Internal::LambdaOperationBase<
    R (Args...),
    PrototypeRestriction
   >;
   using LambdaBase = Internal::LambdaBase<R (Args...)>;
   static constexpr auto const Deleter = &OperationBase::sharedBufferDeleter;

   std::shared_ptr<unsigned char> mutable buffer;

  public:
   //Default constructor
   AllocLambda() {
    OperationBase::initEmptyLambda(const_cast<AllocLambda&>(*this));
   }

   //FunctionOperator / function pointer copy constructor
   template<typename F>
   requires (!IsLambda<F> && SupportedPrototype<F>)
   AllocLambda(F const &f) : AllocLambda() {
    operator=<F>((F const&)f);
   }

   //FunctionOperator move constructor
   template<typename F>
   requires (!IsLambda<F>
    && !Const<F>
    && Struct<F>
    && SupportedPrototype<F>
   )
   AllocLambda(F &&f) : AllocLambda() {
    operator=<F>((F&&)f);
   }

   //Lambda copy constructor
   AllocLambda(AllocLambda const &l) : AllocLambda() {
    operator=<AllocLambda>((AllocLambda const&)l);
   }

   //Lambda move constructor
   AllocLambda(AllocLambda &&l) : AllocLambda() {
    operator=<AllocLambda>((AllocLambda&&)l);
   }

   //CompatibleLambda copy constructor
   template<CompatibleLambda<AllocLambda> L>
   AllocLambda(L const &l) : AllocLambda() {
    operator=<L>((L const&)l);
   }

   //CompatibleLambda move constructor
   template<CompatibleLambda<AllocLambda> L>
   requires (!Const<L>)
   AllocLambda(L &&l) : AllocLambda() {
    operator=<L>((L&&)l);
   }

   ~AllocLambda() = default;

   //Lambda function operator
   R operator()(Args... args) const {
    return (*(LambdaBase *)buffer.get()).op(args...);
   }

   //Presence conversion operator
   operator bool() const noexcept {
    return (*(LambdaBase *)buffer.get()).present();
   }

   //Resets current lambda buffer state to empty
   void reset() {
    OperationBase::reset(*this);
   }

   //Implicit lambda conversion operator
   template<CompatibleLambda<AllocLambda> L>
   explicit operator L() const {
    return L{*this};
   }

   //FunctionOperator / function pointer copy assignment
   template<typename F>
   requires (!IsLambda<F> && SupportedPrototype<F>)
   AllocLambda& operator=(F const &f) {
    //Initialize buffer
    OperationBase::copyAssignFunction(*this, (F const &)f);

    return *this;
   }

   //FunctionOperator move assignment
   template<typename F>
   requires (!IsLambda<F>
    && !Const<F>
    && Struct<F>
    && SupportedPrototype<F>
   )
   AllocLambda& operator=(F &&f) {
    //Initialize buffer
    OperationBase::moveAssignFunction(*this, (F&&)f);

    return *this;
   }

   //Lambda copy assignment
   AllocLambda& operator=(AllocLambda const &l) {
    return operator=<AllocLambda>((AllocLambda const&)l);
   }

   //Lambda move assignment
   AllocLambda& operator=(AllocLambda &&l) {
    return operator=<AllocLambda>((AllocLambda&&)l);
   }

   //CompatibleLambda copy assignment
   template<CompatibleLambda<AllocLambda> L>
   AllocLambda& operator=(L const &l) {
    OperationBase::copyAssignLambda(*this, l);
    return *this;
   }

   //CompatibleLambda move assignment
   template<CompatibleLambda<AllocLambda> L>
   requires (!Const<L>)
   AllocLambda& operator=(L &&l) {
    OperationBase::moveAssignLambda(*this, l);
    return *this;
   }
  };

  //Noexcept qualified lambda specialization
  template<typename R, typename... Args>
  struct AllocLambda<R (Args...) noexcept> {
   template<typename>
   friend struct Lambda;

   template<typename>
   friend struct AllocLambda;

   template<
    typename,
    template<typename> typename
   >
   friend struct Internal::LambdaOperationBase;

   using ReturnType = R;
   template<template<typename...> typename Receiver = Dummy>
   using ArgumentTypes = Receiver<Args...>;
   using FunctionType = R (Args...);

  private:
   //Note: Reuse prototype restrictions from `Lambda<R (Args...) noexcept>`
   template<typename F>
   using PrototypeRestriction = typename Lambda<R (Args...) noexcept>
    ::template PrototypeRestriction<F>;

   template<typename F>
   static constexpr bool const SupportedPrototype = Internal::SupportedPrototype<
    F,
    PrototypeRestriction
   >;

   using OperationBase = Internal::LambdaOperationBase<
    R (Args...),
    PrototypeRestriction
   >;
   using LambdaBase = Internal::LambdaBase<R (Args...)>;
   static constexpr auto const Deleter = &OperationBase::sharedBufferDeleter;

   std::shared_ptr<unsigned char> mutable buffer;

  public:
   //Default constructor
   AllocLambda() {
    OperationBase::initEmptyLambda(const_cast<AllocLambda&>(*this));
   }

   //FunctionOperator / function pointer copy constructor
   template<typename F>
   requires (!IsLambda<F> && SupportedPrototype<F>)
   AllocLambda(F const &f) : AllocLambda() {
    operator=<F>((F const&)f);
   }

   //FunctionOperator move constructor
   template<typename F>
   requires (!IsLambda<F>
    && !Const<F>
    && Struct<F>
    && SupportedPrototype<F>
   )
   AllocLambda(F &&f) : AllocLambda() {
    operator=<F>((F&&)f);
   }

   //Lambda copy constructor
   AllocLambda(AllocLambda const &l) : AllocLambda() {
    operator=<AllocLambda>((AllocLambda const&)l);
   }

   //Lambda move constructor
   AllocLambda(AllocLambda &&l) : AllocLambda() {
    operator=<AllocLambda>((AllocLambda&&)l);
   }

   //CompatibleLambda copy constructor
   template<CompatibleLambda<AllocLambda> L>
   AllocLambda(L const &l) : AllocLambda() {
    operator=<L>((L const&)l);
   }

   //CompatibleLambda move constructor
   template<CompatibleLambda<AllocLambda> L>
   requires (!Const<L>)
   AllocLambda(L &&l) : AllocLambda() {
    operator=<L>((L&&)l);
   }

   ~AllocLambda() = default;

   //Lambda function operator
   R operator()(Args... args) const noexcept {
    return (*(LambdaBase *)buffer.get()).noexceptOp(args...);
   }

   //Presence conversion operator
   operator bool() const noexcept {
    return (*(LambdaBase *)buffer.get()).present();
   }

   //Resets current lambda buffer state to empty
   void reset() {
    OperationBase::reset(*this);
   }

   //Implicit lambda conversion operator
   template<CompatibleLambda<AllocLambda> L>
   explicit operator L() const {
    return L{*this};
   }

   //FunctionOperator / function pointer copy assignment
   template<typename F>
   requires (!IsLambda<F> && SupportedPrototype<F>)
   AllocLambda& operator=(F const &f) {
    //Initialize buffer
    OperationBase::copyAssignFunction(*this, (F const &)f);

    return *this;
   }

   //FunctionOperator move assignment
   template<typename F>
   requires (!IsLambda<F>
    && !Const<F>
    && Struct<F>
    && SupportedPrototype<F>
   )
   AllocLambda& operator=(F &&f) {
    //Initialize buffer
    OperationBase::moveAssignFunction(*this, (F&&)f);

    return *this;
   }

   //Lambda copy assignment
   AllocLambda& operator=(AllocLambda const &l) {
    return operator=<AllocLambda>((AllocLambda const&)l);
   }

   //Lambda move assignment
   AllocLambda& operator=(AllocLambda &&l) {
    return operator=<AllocLambda>((AllocLambda&&)l);
   }

   //CompatibleLambda copy assignment
   template<CompatibleLambda<AllocLambda> L>
   AllocLambda& operator=(L const &l) {
    OperationBase::copyAssignLambda(*this, l);
    return *this;
   }

   //CompatibleLambda move assignment
   template<CompatibleLambda<AllocLambda> L>
   requires (!Const<L>)
   AllocLambda& operator=(L &&l) {
    OperationBase::moveAssignLambda(*this, l);
    return *this;
   }
  };

  //Unqualified c-variadic lambda specialization
  template<typename R, typename... Args>
  struct AllocLambda<R (Args..., ...)> {
   template<typename>
   friend struct Lambda;

   template<typename>
   friend struct AllocLambda;

   template<
    typename,
    template<typename> typename
   >
   friend struct Internal::LambdaOperationBase;

   using ReturnType = R;
   template<template<typename...> typename Receiver = Dummy>
   using ArgumentTypes = Receiver<Args...>;
   using FunctionType = R (Args..., ...);

  private:
   //Note: Reuse prototype restrictions from `Lambda<R (Args..., ...)>`
   template<typename F>
   using PrototypeRestriction = typename Lambda<R (Args..., ...)>
    ::template PrototypeRestriction<F>;

   template<typename F>
   static constexpr bool const SupportedPrototype = Internal::SupportedPrototype<
    F,
    PrototypeRestriction
   >;

   using OperationBase = Internal::LambdaOperationBase<
    R (Args..., ...),
    PrototypeRestriction
   >;
   using LambdaBase = Internal::LambdaBase<R (Args..., ...)>;
   static constexpr auto const Deleter = &OperationBase::sharedBufferDeleter;

   std::shared_ptr<unsigned char> mutable buffer;

  public:
   //Default constructor
   AllocLambda() {
    OperationBase::initEmptyLambda(const_cast<AllocLambda&>(*this));
   }

   //FunctionOperator / function pointer copy constructor
   template<typename F>
   requires (!IsLambda<F> && SupportedPrototype<F>)
   AllocLambda(F const &f) : AllocLambda() {
    operator=<F>((F const&)f);
   }

   //FunctionOperator move constructor
   template<typename F>
   requires (!IsLambda<F>
    && !Const<F>
    && Struct<F>
    && SupportedPrototype<F>
   )
   AllocLambda(F &&f) : AllocLambda() {
    operator=<F>((F&&)f);
   }

   //Lambda copy constructor
   AllocLambda(AllocLambda const &l) : AllocLambda() {
    operator=<AllocLambda>((AllocLambda const&)l);
   }

   //Lambda move constructor
   AllocLambda(AllocLambda &&l) : AllocLambda() {
    operator=<AllocLambda>((AllocLambda&&)l);
   }

   //CompatibleLambda copy constructor
   template<CompatibleLambda<AllocLambda> L>
   AllocLambda(L const &l) : AllocLambda() {
    operator=<L>((L const&)l);
   }

   //CompatibleLambda move constructor
   template<CompatibleLambda<AllocLambda> L>
   requires (!Const<L>)
   AllocLambda(L &&l) : AllocLambda() {
    operator=<L>((L&&)l);
   }

   ~AllocLambda() = default;

   //Lambda function operator
   template<typename... Varargs>
   R operator()(Args... args, Varargs... varargs) const {
    return (*(LambdaBase *)buffer.get()).template op<false>(args..., varargs...);
   }

   //Presence conversion operator
   operator bool() const noexcept {
    return (*(LambdaBase *)buffer.get()).present();
   }

   //Resets current lambda buffer state to empty
   void reset() {
    OperationBase::reset(*this);
   }

   //Implicit lambda conversion operator
   template<CompatibleLambda<AllocLambda> L>
   explicit operator L() const {
    return L{*this};
   }

   //FunctionOperator / function pointer copy assignment
   template<typename F>
   requires (!IsLambda<F> && SupportedPrototype<F>)
   AllocLambda& operator=(F const &f) {
    //Initialize buffer
    OperationBase::copyAssignFunction(*this, (F const &)f);

    return *this;
   }

   //FunctionOperator move assignment
   template<typename F>
   requires (!IsLambda<F>
    && !Const<F>
    && Struct<F>
    && SupportedPrototype<F>
   )
   AllocLambda& operator=(F &&f) {
    //Initialize buffer
    OperationBase::moveAssignFunction(*this, (F&&)f);

    return *this;
   }

   //Lambda copy assignment
   AllocLambda& operator=(AllocLambda const &l) {
    return operator=<AllocLambda>((AllocLambda const&)l);
   }

   //Lambda move assignment
   AllocLambda& operator=(AllocLambda &&l) {
    return operator=<AllocLambda>((AllocLambda&&)l);
   }

   //CompatibleLambda copy assignment
   template<CompatibleLambda<AllocLambda> L>
   AllocLambda& operator=(L const &l) {
    OperationBase::copyAssignLambda(*this, l);
    return *this;
   }

   //CompatibleLambda move assignment
   template<CompatibleLambda<AllocLambda> L>
   requires (!Const<L>)
   AllocLambda& operator=(L &&l) {
    OperationBase::moveAssignLambda(*this, l);
    return *this;
   }
  };

  //Noexcept qualified c-variadic lambda specialization
  template<typename R, typename... Args>
  struct AllocLambda<R (Args..., ...) noexcept> {
   template<typename>
   friend struct Lambda;

   template<typename>
   friend struct AllocLambda;

   template<
    typename,
    template<typename> typename
   >
   friend struct Internal::LambdaOperationBase;

   using ReturnType = R;
   template<template<typename...> typename Receiver = Dummy>
   using ArgumentTypes = Receiver<Args...>;
   using FunctionType = R (Args..., ...) noexcept;

  private:
   //Note: Reuse prototype restrictions from `Lambda<R (Args..., ...) noexcept>`
   template<typename F>
   using PrototypeRestriction = typename Lambda<R (Args..., ...) noexcept>
    ::template PrototypeRestriction<F>;

   template<typename F>
   static constexpr bool const SupportedPrototype = Internal::SupportedPrototype<
    F,
    PrototypeRestriction
   >;

   using OperationBase = Internal::LambdaOperationBase<
    R (Args..., ...),
    PrototypeRestriction
   >;
   using LambdaBase = Internal::LambdaBase<R (Args..., ...)>;
   static constexpr auto const Deleter = &OperationBase::sharedBufferDeleter;

   std::shared_ptr<unsigned char> mutable buffer;

  public:
   //Default constructor
   AllocLambda() {
    OperationBase::initEmptyLambda(const_cast<AllocLambda&>(*this));
   }

   //FunctionOperator / function pointer copy constructor
   template<typename F>
   requires (!IsLambda<F> && SupportedPrototype<F>)
   AllocLambda(F const &f) : AllocLambda() {
    operator=<F>((F const&)f);
   }

   //FunctionOperator move constructor
   template<typename F>
   requires (!IsLambda<F>
    && !Const<F>
    && Struct<F>
    && SupportedPrototype<F>
   )
   AllocLambda(F &&f) : AllocLambda() {
    operator=<F>((F&&)f);
   }

   //Lambda copy constructor
   AllocLambda(AllocLambda const &l) : AllocLambda() {
    operator=<AllocLambda>((AllocLambda const&)l);
   }

   //Lambda move constructor
   AllocLambda(AllocLambda &&l) : AllocLambda() {
    operator=<AllocLambda>((AllocLambda&&)l);
   }

   //CompatibleLambda copy constructor
   template<CompatibleLambda<AllocLambda> L>
   AllocLambda(L const &l) : AllocLambda() {
    operator=<L>((L const&)l);
   }

   //CompatibleLambda move constructor
   template<CompatibleLambda<AllocLambda> L>
   requires (!Const<L>)
   AllocLambda(L &&l) : AllocLambda() {
    operator=<L>((L&&)l);
   }

   ~AllocLambda() = default;

   //Lambda function operator
   template<typename... Varargs>
   R operator()(Args... args, Varargs... varargs) const noexcept {
    return (*(LambdaBase *)buffer.get()).template op<true>(args..., varargs...);
   }

   //Presence conversion operator
   operator bool() const noexcept {
    return (*(LambdaBase *)buffer.get()).present();
   }

   //Resets current lambda buffer state to empty
   void reset() {
    OperationBase::reset(*this);
   }

   //Implicit lambda conversion operator
   template<CompatibleLambda<AllocLambda> L>
   explicit operator L() const {
    return L{*this};
   }

   //FunctionOperator / function pointer copy assignment
   template<typename F>
   requires (!IsLambda<F> && SupportedPrototype<F>)
   AllocLambda& operator=(F const &f) {
    //Initialize buffer
    OperationBase::copyAssignFunction(*this, (F const &)f);

    return *this;
   }

   //FunctionOperator move assignment
   template<typename F>
   requires (!IsLambda<F>
    && !Const<F>
    && Struct<F>
    && SupportedPrototype<F>
   )
   AllocLambda& operator=(F &&f) {
    //Initialize buffer
    OperationBase::moveAssignFunction(*this, (F&&)f);

    return *this;
   }

   //Lambda copy assignment
   AllocLambda& operator=(AllocLambda const &l) {
    return operator=<AllocLambda>((AllocLambda const&)l);
   }

   //Lambda move assignment
   AllocLambda& operator=(AllocLambda &&l) {
    return operator=<AllocLambda>((AllocLambda&&)l);
   }

   //CompatibleLambda copy assignment
   template<CompatibleLambda<AllocLambda> L>
   AllocLambda& operator=(L const &l) {
    OperationBase::copyAssignLambda(*this, l);
    return *this;
   }

   //CompatibleLambda move assignment
   template<CompatibleLambda<AllocLambda> L>
   requires (!Const<L>)
   AllocLambda& operator=(L &&l) {
    OperationBase::moveAssignLambda(*this, l);
    return *this;
   }
  };
 #endif //CX_STL_SUPPORT

 //Deduction guides for AllocLambda
 AllocLambda() -> AllocLambda<void ()>;

 template<typename F>
 requires (StaticFunction<F>)
 AllocLambda(F const&) -> AllocLambda<FunctionPrototype<F>>;

 template<typename F>
 requires (!IsLambda<F> && FunctionOperator<F, decltype(&F::operator())>)
 AllocLambda(F const&) -> AllocLambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename F>
 requires (StaticFunction<F>)
 AllocLambda(F&&) -> AllocLambda<FunctionPrototype<F>>;

 template<typename F>
 requires (!IsLambda<F> && FunctionOperator<F, decltype(&F::operator())>)
 AllocLambda(F&&) -> AllocLambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename L>
 requires (IsLambda<L>)
 AllocLambda(L const&) -> AllocLambda<typename Unqualified<L>::FunctionType>;

 template<typename L>
 requires (IsLambda<L>)
 AllocLambda(L&&) -> AllocLambda<typename Unqualified<L>::FunctionType>;

 //TODO conversion from c-variadic lambda to function pointer
 //Note: Use `CX::VaList` as the prototype requirement instead
}

//Clean up internal macros
#undef CX_STL_SUPPORT_EXPR
