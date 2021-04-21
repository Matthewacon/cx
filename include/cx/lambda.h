#pragma once

#include <cx/idioms.h>
#include <cx/templates.h>

#ifdef CX_STL_SUPPORT
 #include <memory>
#else
 #pragma message \
  "Building without STL support; 'CX::AllocLambda<...>' will be unavailable."
#endif

//Conditional stl dependencies if CX was built with stl support enabled
#ifdef CX_STL_SUPPORT
 #define CX_STL_SUPPORT_EXPR(expr) expr

 #include <exception>
#else
 #define CX_STL_SUPPORT_EXPR(expr)
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

 static_assert(
  CX_LAMBDA_BUF_ALIGN >= alignof(void *),
  "'CX_LAMBDA_BUF_ALIGN' must be at least as large as `alignof(void *)` to "
  "compensate for internal overhead."
 );

 static_assert(
  CX_LAMBDA_BUF_SIZE >= sizeof(void *),
  "'CX_LAMBDA_BUF_SIZE' must be at least as large as `sizeof(void *)` to "
  "compensate for internal overhead."
 );
#else
 #define CX_LAMBDA_BUF_ALIGN (sizeof(void *) * 8)
 #define CX_LAMBDA_BUF_SIZE (sizeof(void *) * 8)
#endif

namespace CX {
 //Supporting exceptions
 struct UninitializedLambdaError CX_STL_SUPPORT_EXPR(: std::exception) {
  char const * what() const noexcept CX_STL_SUPPORT_EXPR(override) {
   return "Lambda is unitialized";
  }
 };

 struct IncompatibleLambdaError CX_STL_SUPPORT_EXPR(: std::exception) {
  char const * const message;

  IncompatibleLambdaError(char const * message) :
   message(message)
  {}

  char const * what() const noexcept CX_STL_SUPPORT_EXPR(override) {
   return message;
  }
 };

 //Non-allocating lambda
 template<typename>
 struct Lambda;

 //TODO Allocating lambda
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
   decltype(sizeof(0)) srcSize,
   decltype(alignof(int)) srcAlignment,
   decltype(srcSize) dstSize,
   decltype(srcAlignment) dstAlignment
  ) {
   //Check that the receiving buffer is sized properly
   //Note: `dstSize == 0` is a special case for allocated lambdas, which
   //do not have size restrictions.
   if (dstSize) {
    if (dstSize < srcSize) {
     throw IncompatibleLambdaError{
      "Receiving lambda does not have a large enough buffer to contain "
      "the assigned lambda's buffer"
     };
    }

    //Check that the receiving buffer is aligned properly
    if (dstAlignment < srcAlignment) {
     throw IncompatibleLambdaError{
      "Receiving lambda's buffer is not aligned properly to receive "
      "the assigned lambda's buffer"
     };
    }
   }
  }

  //Ensure `T` will fit within a buffer of size `Size` with
  //alignment `Alignment`
  template<typename T, auto Size, auto Alignment>
  [[gnu::always_inline]]
  void lambdaAssignBufferCheck() {
   //Note: Add `alognof(void *)` to account for internal
   //requirements
   static_assert(
    MaxValue<alignof(T), alignof(void *)> <= Alignment,
    "The required alignment of the given functor is larger than the "
    "current 'CX_LAMBDA_BUF_ALIGN' value. Consider increasing it to "
    "the next power of 2."
   );
   //Note: Add `sizeof(void *)` to account for vtable ptr
   static_assert(
    (sizeof(T) + sizeof(void *)) <= Size,
    "The size of the given functor is larger than the current "
    "'CX_LAMBDA_BUF_SIZE' value. Consider increasing it to the next "
    "power of 2."
   );
  }

  //The base of all lambda buffers
  template<typename F>
  struct LambdaBase;

  //Ignore warnings for throwing in noexcept functions;
  //trying to invoke an uninitialized lambda will:
  // - throw for unqualified lambdas
  // - terminate for noexcept lambdas
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wexceptions"

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
    throw UninitializedLambdaError{};
   }

   [[gnu::always_inline]]
   virtual R noexceptOp(Args...) noexcept {
    throw UninitializedLambdaError{};
   }

   virtual bool allocates() const noexcept = 0;
   virtual decltype(sizeof(0)) typeSize() const noexcept = 0;
   virtual decltype(alignof(int)) typeAlignment() const noexcept = 0;
   virtual decltype(sizeof(0)) bufferSize() const noexcept = 0;
   virtual decltype(alignof(int)) bufferAlignment() const noexcept = 0;
   virtual void copyTo(
    void *,
    decltype(sizeof(0)),
    decltype(alignof(int))
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
    throw UninitializedLambdaError{};
   }

   template<bool Noexcept, typename... Varargs>
   [[gnu::always_inline]]
   R op(Args... args, Varargs... varargs) noexcept(Noexcept) {
    return get().template invoke<Noexcept, Varargs...>(args..., varargs...);
   }

   virtual bool allocates() const noexcept = 0;
   virtual decltype(sizeof(0)) typeSize() const noexcept = 0;
   virtual decltype(alignof(int)) typeAlignment() const noexcept = 0;
   virtual decltype(sizeof(0)) bufferSize() const noexcept = 0;
   virtual decltype(alignof(int)) bufferAlignment() const noexcept = 0;
   virtual void copyTo(
    void *,
    decltype(sizeof(0)),
    decltype(alignof(int))
   ) = 0;

   [[gnu::always_inline]]
   virtual bool present() const noexcept {
    return false;
   }
  };

  //Stub implementation of `LambdaBase<...>` for all
  //lambda prototypes; default instance for uninitialized
  //non-allocating lambdas
  template<typename Prototype>
  struct NonAllocLambdaWrapperStub : LambdaBase<Prototype> {
  private:
   decltype(sizeof(0)) bufSize;
   decltype(alignof(int)) bufAlign;

  public:
   NonAllocLambdaWrapperStub(decltype(bufSize) bufSize, decltype(bufAlign) bufAlign) :
    LambdaBase<Prototype>{},
    bufSize(bufSize),
    bufAlign(bufAlign)
   {}

   bool allocates() const noexcept override {
    return false;
   }

   decltype(sizeof(0)) typeSize() const noexcept override {
    return sizeof(NonAllocLambdaWrapperStub);
   }

   decltype(alignof(int)) typeAlignment() const noexcept override {
    return alignof(NonAllocLambdaWrapperStub);
   }

   decltype(sizeof(0)) bufferSize() const noexcept override {
    return bufSize;
   }

   decltype(alignof(int)) bufferAlignment() const noexcept override {
    return bufAlign;
   }

   void copyTo(
    void * buf,
    decltype(sizeof(0)) bufSize,
    decltype(alignof(int)) bufAlignment
   ) override {
    //Ensure receiving buffer is compatible
    wrapperBufferCheck(
     typeSize(),
     typeAlignment(),
     bufSize,
     bufAlignment
    );

    //Initialize buffer
    new (buf) NonAllocLambdaWrapperStub{bufferSize(), bufferAlignment()};
   }
  };

  //Stub implementation of `LambdaBase<...>` for all
  //lambda prototypes; default instance for uninitialized
  //allocating lambdas
  template<typename Prototype>
  struct AllocLambdaWrapperStub : LambdaBase<Prototype> {
   //TODO
  };

  //TODO rename to: copyInitWrapper
  template<typename Wrapper, typename Function>
  requires (CopyConstructible<Function>
   || (Constructible<Function> && CopyAssignable<Function>)
  )
  [[gnu::always_inline]]
  void copyLambdaBuffer(
   void * buf,
   decltype(sizeof(0)) bufSize,
   decltype(alignof(int)) bufAlignment,
   Function const &func
  ) {
   //Ensure receiving buffer is compatible
   wrapperBufferCheck(
    sizeof(Wrapper),
    alignof(Wrapper),
    bufSize,
    bufAlignment
   );

   //Initialize buffer
   if constexpr (CopyConstructible<Function>) {
    //If `Function` is copy constructible, invoke wrapper
    //function copy constructor
    new (buf) Wrapper{(Function const&)func};
   } else if constexpr(Constructible<Function> && CopyAssignable<Function>) {
    //If `Function` is default constructible and copy assignable,
    //default construct wrapper and copy assign to function
    (*new (buf) Wrapper{}).f = (Function const&)func;
   }
  }

  //TODO rename to: moveInitWrapper
  template<typename Wrapper, typename Function>
  requires (MoveConstructible<Function>
   || (Constructible<Function> && MoveAssignable<Function>)
  )
  [[gnu::always_inline]]
  void moveLambdaBuffer(
   void * buf,
   decltype(sizeof(0)) bufSize,
   decltype(alignof(int)) bufAlignment,
   Function &&func
  ) {
   //Ensure receiving buffer is compatible
   wrapperBufferCheck(
    sizeof(Wrapper),
    alignof(Wrapper),
    bufSize,
    bufAlignment
   );

   //Initialize buffer
   if constexpr(MoveConstructible<Function>) {
    //If `Function` is move constructible, invoke wrapper
    //function move constructor
    new (buf) Wrapper{(Function&&)func};
   } else if constexpr (Constructible<Function> && MoveAssignable<Function>) {
    //If `Function` is default constructible and move assignable,
    //default construct wrapper and move assign to function
    (*new (buf) Wrapper{}).f = (Function&&)func;
   }
  }

  //Implementation of LambdaBase<...>
  template<typename, typename>
  struct NonAllocLambdaWrapper;

  //Lambda wrapper for `F` (non-c-variadic)
  template<typename F, typename R, typename... Args>
  struct NonAllocLambdaWrapper<F, R (Args...)> : LambdaBase<R (Args...)> {
   F f;

   //Default constructor
   NonAllocLambdaWrapper() = default;

   //Function copy constructor
   NonAllocLambdaWrapper(F const &f) :
    f((F const&)f)
   {}

   //Function move constructor
   NonAllocLambdaWrapper(F &&f) :
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

   decltype(sizeof(0)) typeSize() const noexcept override {
    return sizeof(NonAllocLambdaWrapper);
   }

   decltype(alignof(int)) typeAlignment() const noexcept override {
    return alignof(NonAllocLambdaWrapper);
   }

   decltype(sizeof(0)) bufferSize() const noexcept override {
    return CX_LAMBDA_BUF_SIZE;
   }

   decltype(alignof(int)) bufferAlignment() const noexcept override {
    return CX_LAMBDA_BUF_ALIGN;
   }

   void copyTo(
    void * buf,
    decltype(sizeof(0)) bufSize,
    decltype(alignof(int)) bufAlignment
   ) override {
    //Copy encapsulated function to new buffer
    copyLambdaBuffer<NonAllocLambdaWrapper>(
     buf,
     bufSize,
     bufAlignment,
     (F const&)f
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
   F f;

   //Default constructor
   NonAllocLambdaWrapper() = default;

   //Function copy constructor
   NonAllocLambdaWrapper(F const &f) :
    f((F const&)f)
   {}

   //Function move constructor
   NonAllocLambdaWrapper(F &&f) :
    f((F&&)f)
   {}

   [[gnu::always_inline]]
   typename LambdaBase<R (Args..., ...)>::FptrWrapper get() override {
    return {(F const&)f};
   }

   bool allocates() const noexcept override {
    return false;
   }

   decltype(sizeof(0)) typeSize() const noexcept override {
    return sizeof(NonAllocLambdaWrapper);
   }

   decltype(alignof(int)) typeAlignment() const noexcept override {
    return alignof(NonAllocLambdaWrapper);
   }

   decltype(sizeof(0)) bufferSize() const noexcept override {
    return CX_LAMBDA_BUF_SIZE;
   }

   decltype(alignof(int)) bufferAlignment() const noexcept override {
    return CX_LAMBDA_BUF_ALIGN;
   }

   void copyTo(
    void * buf,
    decltype(sizeof(0)) bufSize,
    decltype(alignof(int)) bufAlignment
   ) override {
    //Copy encapsulated function to new buffer
    copyLambdaBuffer<NonAllocLambdaWrapper>(
     buf,
     bufSize,
     bufAlignment,
     (F const&)f
    );
   }

   [[gnu::always_inline]]
   bool present() const noexcept override {
    return true;
   }
  };

  #pragma GCC diagnostic pop

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
   //TODO rename to: `initEmptyLambda`
   //Initializes buffer with empty `LambdaBase` obj
   [[gnu::always_inline]]
   static void init(
    void * buffer,
    decltype(sizeof(0)) size,
    decltype(alignof(int)) alignment
   ) {
    using UninitializedLambdaType = NonAllocLambdaWrapperStub<SpecializationPrototype>;

    //Ensure receiving buffer is compatible
    wrapperBufferCheck(
     sizeof(UninitializedLambdaType),
     alignof(UninitializedLambdaType),
     size,
     alignment
    );

    new (buffer) UninitializedLambdaType {
     size,
     alignment
    };
   }

   //Destructs current `LambdaBase` obj in buffer
   [[gnu::always_inline]]
   static void destroy(void * buffer) {
    using LambdaBase = LambdaBase<SpecializationPrototype>;
    (*(LambdaBase *)buffer).~LambdaBase();
   }

   //FunctionOperator / function pointer copy assignment
   template<SupportedPrototype<Restriction> F>
   requires (CopyConstructible<F>
    || (Constructible<F> && CopyAssignable<F>)
   )
   [[gnu::always_inline]]
   static void copyAssignFunction(void * buffer, F const &f) {
    auto &base = *(LambdaBase<SpecializationPrototype> *)buffer;
    auto const
     size = base.bufferSize(),
     alignment = base.bufferAlignment();

    //Clean up current state
    //Note: `base` reference points to an invalid object beyond this point
    destroy(buffer);

    //Initialize buffer
    copyLambdaBuffer<NonAllocLambdaWrapper<F, SpecializationPrototype>>(
     buffer,
     size,
     alignment,
     (F const&)f
    );
   }

   //FunctionOperator move assignment
   template<SupportedPrototype<Restriction> F>
   requires (Struct<F>
    && (MoveConstructible<F> || (Constructible<F> && MoveAssignable<F>))
   )
   [[gnu::always_inline]]
   static void moveAssignFunction(void * buffer, ConstDecayed<F> &&f) {
    auto &base = *(LambdaBase<SpecializationPrototype> *)buffer;
    auto const
     size = base.size(),
     alignment = base.alignment();

    //Clean up current state
    //Note: `base` reference points to an invalid object beyond this point
    destroy(buffer);

    //Initialize buffer
    moveLambdaBuffer<NonAllocLambdaWrapper<
     ConstDecayed<F>,
     SpecializationPrototype>
    >(buffer, (ConstDecayed<F>&&)f);
   }

   //Lambda copy and move assignment
   [[gnu::always_inline]]
   static void copyAssignLambda(void * buf1, void * buf2) {
    auto &l1 = *(LambdaBase<SpecializationPrototype> *)buf1;
    auto const
     buf1Size = l1.bufferSize(),
     buf1Alignment = l1.bufferAlignment();

    auto &l2 = *(LambdaBase<SpecializationPrototype> *)buf2;

    //Clean up current state
    //Note: `l1` reference points to an invalid object beyond this point
    destroy(buf1);

    //Copy `l2` to `l1`
    l2.copyTo(buf1, buf1Size, buf1Alignment);
   }

   //Lambda move assignment
   [[gnu::always_inline]]
   static void moveAssignLambda(void * buf1, void * buf2) {
    auto &l1 = *(LambdaBase<SpecializationPrototype> *)buf1;
    auto const
     buf1Size = l1.bufferSize(),
     buf1Alignment = l1.bufferAlignment();

    //Clean up current state
    //Note: `l1` reference points to an invalid object beyond this point
    destroy(buf1);

    auto &l2 = *(LambdaBase<SpecializationPrototype> *)buf2;
    auto const
     buf2Size = l2.bufferSize(),
     buf2Alignment = l2.bufferAlignment();

    //Copy `l2` to `l1`
    l2.copyTo(buf1, buf1Size, buf1Alignment);

    //Clean up `l2` and reinitialize it as an empty lambda
    //Note: `l2` points to an invalid object beyond this point
    destroy(buf2);
    init(buf2, buf2Size, buf2Alignment);
   }
  };
 }

 //Unqualified lambda specialization
 template<typename R, typename... Args>
 struct Lambda<R (Args...)> {
  template<typename>
  friend struct Lambda;

  template<typename>
  friend struct AllocLambda;

  template<typename, template<typename> typename>
  friend struct Internal::LambdaOperationBase;

  using ReturnType = R;
  template<template<typename...> typename Receiver = Dummy>
  using ArgumentTypes = Receiver<Args...>;
  using FunctonType = R (Args...);

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
   OperationBase::init(&buf(), Size, Alignment);
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
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda(ConstDecayed<F> &&f) : Lambda() {
   operator=<F>((ConstDecayed<F>&&)f);
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
  Lambda(ConstDecayed<L> &&l) : Lambda() {
   operator=<L>((ConstDecayed<L>&&)l);
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
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::copyAssignFunction(&buf(), (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<typename F>
  requires (!IsLambda<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda& operator=(ConstDecayed<F> &&f) {
   //Ensure `F` will fit within `buffer`
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::moveAssignFunction(&buf(), (ConstDecayed<F>&&)f);

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
   OperationBase::copyAssignLambda(&buf(), &l.buf());
   return *this;
  }

  //CompatibleLambda move assignment
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda& operator=(ConstDecayed<L> &&l) {
   OperationBase::moveAssignLambda(&buf(), &l.buf());
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

  template<typename, template<typename> typename>
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
   OperationBase::init(&buf(), Size, Alignment);
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
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda(ConstDecayed<F> &&f) : Lambda() {
   operator=<F>((ConstDecayed<F>&&)f);
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
  Lambda(ConstDecayed<L> &&l) : Lambda() {
   operator=<L>((ConstDecayed<L>&&)l);
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
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::copyAssignFunction(&buf(), (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<typename F>
  requires (!IsLambda<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda& operator=(ConstDecayed<F> &&f) {
   //Ensure `F` will fit within `buffer`
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::moveAssignFunction(&buf(), (ConstDecayed<F>&&)f);

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
   OperationBase::copyAssignLambda(&buf(), l.buf());
   return *this;
  }

  //CompatibleLambda move assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(ConstDecayed<L> &&l) {
   OperationBase::moveAssignLambda(&buf(), l.buf());
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

  template<typename, template<typename> typename>
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
   OperationBase::init(&buf(), Size, Alignment);
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
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda(ConstDecayed<F> &&f) : Lambda() {
   operator=<F>((ConstDecayed<F> &&)f);
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
  Lambda(ConstDecayed<L> &&l) : Lambda() {
   operator=<L>((ConstDecayed<L>&&)l);
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
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::copyAssignFunction(&buf(), (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<typename F>
  requires (!IsLambda<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda& operator=(ConstDecayed<F> &&f) {
   //Ensure `F` will fit within `buffer`
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::moveAssignFunction(&buf(), (ConstDecayed<F>&&)f);

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
   OperationBase::copyAssignLambda(&buf(), &l.buf());
   return *this;
  }

  //CompatibleLambda move assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(ConstDecayed<L> &&l) {
   OperationBase::moveAssignLambda(&buf(), &l.buf());
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

  template<typename, template<typename> typename>
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
   OperationBase::init(&buf(), Size, Alignment);
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
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda(ConstDecayed<F> &&f) : Lambda() {
   operator=<F>((ConstDecayed<F>&&)f);
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
  Lambda(ConstDecayed<L> &&l) {
   operator=<L>((ConstDecayed<L>&&)l);
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
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::copyAssignFunction(&buf(), (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<typename F>
  requires (!IsLambda<F>
   && Struct<F>
   && SupportedPrototype<F>
  )
  Lambda& operator=(ConstDecayed<F> &&f) {
   //Ensure `F` will fit within `buffer`
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::moveAssignFunction(&buf(), (ConstDecayed<F>&&)f);

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
   OperationBase::copyAssignLambda(&buf(), &l.buf());
   return *this;
  }

  //CompatibleLambda move assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(ConstDecayed<L> &&l) {
   OperationBase::moveAssignLambda(&buf(), &l.buf());
   return *this;
  }
 };

 //Deduction guides for Lambda
 Lambda() -> Lambda<void ()>;

 template<StaticFunction F>
 Lambda(F const&) -> Lambda<FunctionPrototype<F>>;

 template<typename F>
 requires (FunctionOperator<F, decltype(&F::operator())>)
 Lambda(F const&) -> Lambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename F>
 requires (StaticFunction<F>)
 Lambda(F&&) -> Lambda<FunctionPrototype<F>>;

 template<typename F>
 requires (FunctionOperator<F, decltype(&F::operator())>)
 Lambda(F&&) -> Lambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename L>
 requires (IsLambda<L>)
 Lambda(L const&) -> Lambda<typename L::FunctionType>;

 template<typename L>
 requires (IsLambda<L>)
 Lambda(L&&) -> Lambda<typename L::FunctionType>;

 //AllocLambda implementation
 #ifdef CX_STL_SUPPORT
  //TODO Unqualified lambda specialization
  template<typename R, typename... Args>
  struct AllocLambda<R (Args...)> {};

  //TODO Noexcept qualified lambda specialization
  template<typename R, typename... Args>
  struct AllocLambda<R (Args...) noexcept > {};

  //TODO Unqualified c-variadic lambda specialization
  template<typename R, typename... Args>
  struct AllocLambda<R (Args..., ...)> {};

  //TODO Noexcept qualified c-variadic lambda specialization
  template<typename R, typename... Args>
  struct AllocLambda<R (Args..., ...) noexcept> {};
 #endif //CX_STL_SUPPORT

 //Deduction guides for AllocLambda
 AllocLambda() -> AllocLambda<void ()>;

 template<typename F>
 requires (StaticFunction<F>)
 AllocLambda(F const&) -> AllocLambda<FunctionPrototype<F>>;

 template<typename F>
 requires (FunctionOperator<F, decltype(&F::operator())>)
 AllocLambda(F const&) -> AllocLambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename F>
 requires (StaticFunction<F>)
 AllocLambda(F&&) -> AllocLambda<FunctionPrototype<F>>;

 template<typename F>
 requires (FunctionOperator<F, decltype(&F::operator())>)
 AllocLambda(F&&) -> AllocLambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename L>
 requires (IsLambda<L>)
 AllocLambda(L const&) -> AllocLambda<typename L::FunctionType>;

 template<typename L>
 requires (IsLambda<L>)
 AllocLambda(L&&) -> AllocLambda<typename L::FunctionType>;

 //TODO conversion from c-variadic lambda to function pointer
 //Note: Use `CX::VaList` as the prototype requirement instead
}

//Clean up lambda macros
#undef CX_LAMBDA_BUF_ALIGN
#undef CX_LAMBDA_BUF_SIZE
