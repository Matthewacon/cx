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
   typename R1,
   typename... Args1,
   template<typename> typename L2,
   typename R2,
   typename... Args2
  >
  requires (Internal::IsLambdaTemplate<L1> && Internal::IsLambdaTemplate<L2>)
  struct CompatibleLambda<L1<R1 (Args1...)>, L2<R2 (Args2...)>> {
   static constexpr auto const Value = CX::SameType<R1 (Args1...), R2 (Args2...)>;
  };

  //Matching lambda prototype w/ c-varargs
  template<
   template<typename> typename L1,
   typename R1,
   typename... Args1,
   template<typename> typename L2,
   typename R2,
   typename... Args2
  >
  requires (Internal::IsLambdaTemplate<L1> && Internal::IsLambdaTemplate<L2>)
  struct CompatibleLambda<L1<R1 (Args1..., ...)>, L2<R2 (Args2..., ...)>> {
   static constexpr auto const Value = CX::SameType<R1 (Args1..., ...), R2 (Args2..., ...)>;
  };

  //Qualified lambda to unqualified lambda
  template<
   template<typename> typename L1,
   typename R1,
   typename... Args1,
   template<typename> typename L2,
   typename R2,
   typename... Args2
  >
  requires (Internal::IsLambdaTemplate<L1> && Internal::IsLambdaTemplate<L2>)
  struct CompatibleLambda<L1<R1 (Args1...) noexcept>, L2<R2 (Args2...)>> {
   static constexpr auto const Value = CX::SameType<R1 (Args1...), R2 (Args2...)>;
  };

  //Qualified c-variadic lambda to unualified c-variadic lambda
  template<
   template<typename> typename L1,
   typename R1,
   typename... Args1,
   template<typename> typename L2,
   typename R2,
   typename... Args2
  >
  requires (Internal::IsLambdaTemplate<L1> && Internal::IsLambdaTemplate<L2>)
  struct CompatibleLambda<L1<R1 (Args1..., ...) noexcept>, L2<R2 (Args2..., ...)>> {
   static constexpr auto const Value = CX::SameType<R1 (Args1..., ...), R2 (Args2..., ...)>;
  };
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
  //The base of all lambda buffers
  template<typename F, typename = F>
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
   template<typename, typename>
   friend struct LambdaBase;

   template<typename, typename>
   friend struct LambdaSpecialized;

  private:
   LambdaBase() = default;

  public:
   virtual ~LambdaBase() = default;

   virtual R op(Args...) {
    throw UninitializedLambdaError{};
   }

   virtual R noexceptOp(Args...) noexcept {
    throw UninitializedLambdaError{};
   }

   virtual decltype(sizeof(0)) size() const noexcept {
    throw UninitializedLambdaError{};
   }

   virtual decltype(alignof(int)) alignment() const noexcept {
    throw UninitializedLambdaError{};
   }

   virtual void copy(void *) {
    throw UninitializedLambdaError{};
   }
  };

  //Uninitialized `LambdaBase` for unqualified and `noexcept` qualified
  //function types
  template<typename R, typename... Args>
  struct LambdaBase<R (Args...), ImpossibleType<>> : LambdaBase<R (Args...)> {
   template<typename, typename>
   friend struct LambdaBase;

  private:
   decltype(sizeof(0)) bufSize;
   decltype(alignof(int)) bufAlign;

  public:
   LambdaBase(decltype(bufSize) bufSize, decltype(bufAlign) bufAlign) :
    LambdaBase<R (Args...)>{},
    bufSize(bufSize),
    bufAlign(bufAlign)
   {}

   virtual decltype(bufSize) size() const noexcept override {
    return bufSize;
   }

   virtual decltype(bufAlign) alignment() const noexcept override {
    return bufAlign;
   }
  };

  //Base of unqualified and `noexcept` qualified c-variadic function types
  template<typename R, typename... Args>
  struct LambdaBase<R (Args..., ...)> {
   template<typename, typename>
   friend struct LambdaBase;

   template<typename, typename>
   friend struct LambdaSpecialized;

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

     template<typename T>
     MemberPtr(R (T::* ptr)(Args..., ...)) :
      memberPtr((decltype(memberPtr))ptr)
     {}
    } memberPtr;

    #pragma GCC diagnostic pop

    FptrWrapper(R (* const &ptr)(Args..., ...)) :
     fptrOrInst(ptr)
    {}

    template<FunctionOperator T>
    FptrWrapper(T const &inst) :
     fptrOrInst((void *)&inst),
     memberPtr{&T::operator()}
    {}

    template<typename... Varargs, bool Noexcept>
    R invoke(Args... args, Varargs... varargs) {
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

   virtual FptrWrapper get() noexcept {
    throw UninitializedLambdaError{};
   }

   template<typename... Varargs>
   R op(Args... args, Varargs... varargs) {
    return get().template invoke<Varargs..., false>(args..., varargs...);
   }

   template<typename... Varargs>
   R noexceptOp(Args... args, Varargs... varargs) noexcept {
    return get().template invoke<Varargs..., true>(args..., varargs...);
   }

   virtual decltype(sizeof(0)) size() const noexcept {
    throw UninitializedLambdaError{};
   }

   virtual decltype(alignof(int)) alignment() const noexcept {
    throw UninitializedLambdaError{};
   }

   virtual void copy(void *) {
    throw UninitializedLambdaError{};
   }
  };

  //Uninitialized `LambdaBase` for unqualified and `noexcept` qualified
  //c-variadic function types
  template<typename R, typename... Args>
  struct LambdaBase<R (Args..., ...), ImpossibleType<>> : LambdaBase<R (Args..., ...)> {
   template<typename, typename>
   friend struct LambdaBase;

  private:
   decltype(sizeof(0)) bufSize;
   decltype(alignof(int)) bufAlign;

  public:
   LambdaBase(decltype(bufSize) bufSize, decltype(bufAlign) bufAlign) :
    LambdaBase<R (Args..., ...)>{},
    bufSize(bufSize),
    bufAlign(bufAlign)
   {}

   decltype(bufSize) size() const noexcept override {
    return bufSize;
   }

   decltype(bufAlign) alignment() const noexcept override {
    return bufAlign;
   }
  };

  //TODO rename to: copyInitWrapper
  template<typename Wrapper, typename Function>
  requires (CopyConstructible<Function>
   || (Constructible<Function> && CopyAssignable<Function>)
  )
  [[gnu::always_inline]]
  void copyLambdaBuffer(void * buf, Function const &func) {
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
  void moveLambdaBuffer(void * buf, Function &&func) {
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

  [[gnu::always_inline]]
  void wrapperBufferCheck(auto const &w1, auto const &w2) {
   //Check that the receiving buffer is sized properly
   if (w2.size() < w1.size()) {
    throw IncompatibleLambdaError{
     "Receiving lambda does not have a large enough buffer to contain "
     "the assigned lambda's buffer"
    };
   }

   //Check that the receiving buffer is aligned properly
   if (w2.alignment() < w1.alignment()) {
    throw IncompatibleLambdaError{
     "Receiving lambda's buffer is not aligned properly to receive "
     "the assigned lambda's buffer"
    };
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


  //Implementation of LambdaBase<...>
  template<typename, typename>
  struct LambdaSpecialized;

  //Lambda wrapper for `F` (non-c-variadic)
  template<typename F, typename R, typename... Args>
  struct LambdaSpecialized<F, R (Args...)> : LambdaBase<R (Args...)> {
   F f;

   //Default constructor
   LambdaSpecialized() = default;

   //Function copy constructor
   LambdaSpecialized(F const &f) :
    f((F const&)f)
   {}

   //Function move constructor
   LambdaSpecialized(F &&f) :
    f((F&&)f)
   {}

   R op(Args... args) override {
    return f(args...);
   }

   R noexceptOp(Args... args) noexcept override {
    return f(args...);
   }

   decltype(sizeof(0)) size() const noexcept override {
    return sizeof(LambdaSpecialized);
   }

   decltype(alignof(int)) alignment() const noexcept override {
    return alignof(LambdaSpecialized);
   }

   void copy(void * buf) override {
    //Ensure the receiving buffer is compatible
    auto &other = *(LambdaBase<R (Args...)> *)buf;
    wrapperBufferCheck(*this, other);

    //Copy encapsulated function to new buffer
    copyLambdaBuffer<LambdaSpecialized>(buf, (F const&)f);
   }
  };

  //Lambda wrapper for `F` (c-variadic)
  template<typename F, typename R, typename... Args>
  struct LambdaSpecialized<F, R (Args..., ...)> : LambdaBase<R (Args..., ...)> {
   F f;

   //Default constructor
   LambdaSpecialized() = default;

   //Function copy constructor
   LambdaSpecialized(F const &f) :
    f((F const&)f)
   {}

   //Function move constructor
   LambdaSpecialized(F &&f) :
    f((F&&)f)
   {}

   typename LambdaBase<R (Args..., ...)>::FptrWrapper get() noexcept override {
    return {(F const&)f};
   }

   decltype(sizeof(0)) size() const noexcept override {
    return sizeof(LambdaSpecialized);
   }

   decltype(alignof(int)) alignment() const noexcept override {
    return alignof(LambdaSpecialized);
   }

   void copy(void * buf) override {
    //Ensure that the receiving buffer is compatible
    auto &other = *(LambdaBase<R (Args..., ...)> *)buf;
    wrapperBufferCheck(*this, other);

    //Copy encapsulated function to new buffer
    copyLambdaBuffer<LambdaSpecialized>(buf, (F const&)f);
   }
  };

  #pragma GCC diagnostic pop

  //Base of all lambda internal operations
  template<typename>
  struct LambdaOperationBase;

  //LambdaOperationBase implementation for non-c-variadic functions
  template<typename R, typename... Args>
  struct LambdaOperationBase<R (Args...)> {
   //TODO rename to: `initEmptyLambda`
   //Initializes buffer with empty `LambdaBase` obj
   template<auto Size>
   [[gnu::always_inline]]
   static void init(unsigned char (&buffer)[Size], decltype(alignof(int)) alignment) {
    new (&buffer) LambdaBase<R (Args...), ImpossibleType<>>{
     Size,
     alignment
    };
   }

   //Destructs current `LambdaBase` obj in buffer
   template<auto Size>
   [[gnu::always_inline]]
   static void destroy(unsigned char (&buffer)[Size]) {
    using LambdaBase = LambdaBase<R (Args...)>;
    (*(LambdaBase *)&buffer).~LambdaBase();
   }

   //FunctionOperator/StaticFunction copy assignment
   template<auto Size, typename F>
   requires (
    StaticFunction<F, R, Args...>
    || (FunctionOperator<F, R, Args...>
     && (CopyConstructible<F> || (Constructible<F> && CopyAssignable<F>))
    )
   )
   [[gnu::always_inline]]
   static void copyAssignFunction(unsigned char (&buffer)[Size], F const &f) {
    //Clean up current state
    destroy(buffer);

    //Initialize buffer
    copyLambdaBuffer<LambdaSpecialized<F, R (Args...)>>(buffer, (F const&)f);
   }

   //FunctionOperator move assignment
   template<auto Size, FunctionOperator<R, Args...> F>
   requires (!Const<F>
    && (MoveConstructible<F> || (Constructible<F> && MoveAssignable<F>))
   )
   [[gnu::always_inline]]
   static void moveAssignFunction(unsigned char (&buffer)[Size], F &&f) {
    //Clean up current state
    destroy(buffer);

    //Initialize buffer
    moveLambdaBuffer<LambdaSpecialized<F, R (Args...)>>(buffer, (F&&)f);
   }

   //Lambda copy and move assignment
   template<auto Size1, auto Size2>
   static void copyAssignLambda(unsigned char (&l1)[Size1], unsigned char (&l2)[Size2]) {
    //Clean up current state
    destroy(l1);

    //Copy `l2` to `l1`
    (*(LambdaBase<R (Args...)> *)&l2).copy(l1);
   }

   //Lambda move assignment
   template<auto Size1, auto Size2>
   static void moveAssignLambda(unsigned char (&l1)[Size1], unsigned char (&l2)[Size2]) {
    //Clean up current state
    destroy(l1);

    //Copy `l2` to `l1`
    (*(LambdaBase<R (Args...)> *)&l2).copy(l1);

    //Clean up `l2` and reinitialize it as an empty lambda
    destroy(l2);
    init(l2);
   }
  };

  //TODO LambdaOperationBase implementation for c-variadic functions
  // - Lambda <-> Lambda
  // -
 }

 //Unqualified lambda type
 template<typename R, typename... Args>
 struct Lambda<R (Args...)> {
  template<typename>
  friend struct Lambda;

  template<typename>
  friend struct AllocLambda;

  template<typename>
  friend struct Internal::LambdaOperationBase;

  using ReturnType = R;
  using ArgumentTypes = Dummy<Args...>;
  using FunctonType = R (Args...);

  static constexpr auto const Alignment = CX_LAMBDA_BUF_ALIGN;
  static constexpr auto const Size = CX_LAMBDA_BUF_SIZE;

 private:
  using OperationBase = Internal::LambdaOperationBase<R (Args...)>;
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
   OperationBase::init(buf(), Alignment);
  }

  //Function pointer constructor
  template<StaticFunction<R, Args...> F>
  Lambda(F * f) : Lambda() {
   operator=<F>(f);
  }

  //FunctionOperator copy constructor
  template<FunctionOperator<R, Args...> F>
  requires (!IsLambda<F>)
  Lambda(F const &f) : Lambda() {
   operator=<F>((F const&)f);
  }

  //FunctionOperator move constructor
  template<FunctionOperator<R, Args...> F>
  requires (!IsLambda<F>)
  Lambda(ConstDecayed<F> &&f) : Lambda() {
   operator=<F>((ConstDecayed<F>&&)f);
  }

  //Lambda copy constructor
  template<CompatibleLambda<Lambda> L>
  Lambda(L const &l) : Lambda() {
   operator=<L>((L const&)l);
  }

  //Lambda move constructor
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda(ConstDecayed<L> &&l) : Lambda() {
   operator=<L>((ConstDecayed<L>&&)l);
  }

  ~Lambda() {
   //Destruct current buffer (guaranteed to be initialized)
   OperationBase::destroy(buf());
  }

  //Lambda function operator
  R operator()(Args... args) const {
   return (*(LambdaBase *)&buffer).op(args...);
  }

  //Function pointer assignment operator
  template<StaticFunction<R, Args...> F>
  Lambda& operator=(F * f) {
   //Ensure `F` will fit within `buffer`
   Internal::lambdaAssignBufferCheck<F *, Size, Alignment>();

   //Initalize buffer
   OperationBase::copyAssignFunction(buf(), (F * const&)f);

   return *this;
  }

  //FunctionOperator copy assignment
  template<FunctionOperator<R, Args...> F>
  requires (!IsLambda<F>)
  Lambda& operator=(F const &f) {
   //Ensure `F` will fit within `buffer`
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::copyAssignFunction(buf(), (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<FunctionOperator<R, Args...> F>
  requires (!IsLambda<F>)
  Lambda& operator=(ConstDecayed<F> &&f) {
   //Ensure `F` will fit within `buffer`
   Internal::lambdaAssignBufferCheck<F, Size, Alignment>();

   //Initialize buffer
   OperationBase::moveAssignFunction(buf(), (ConstDecayed<F>&&)f);

   return *this;
  }

  //Lambda copy assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(L const &l) {
   OperationBase::copyAssignLambda(buf(), l.buf());
   return *this;
  }

  //Lambda move assignment
  template<CompatibleLambda<Lambda> L>
  requires (!Const<L>)
  Lambda& operator=(ConstDecayed<L> &&l) {
   OperationBase::moveAssignLambda(buf(), l.buf());
   return *this;
  }
 };

 template<typename R, typename... Args>
 struct Lambda<R (Args...) noexcept> {
  template<typename>
  friend struct Lambda;

  template<typename>
  friend struct AllocLambda;

  template<typename>
  friend struct Internal::LambdaOperationBase;

  using ReturnType = R;
  using ArgumentTypes = Dummy<Args...>;
  using FunctionType = R (Args...) noexcept;

  static constexpr auto const Alignment = CX_LAMBDA_BUF_ALIGN;
  static constexpr auto const Size = CX_LAMBDA_BUF_SIZE;

 private:
  using OperationBase = Internal::LambdaOperationBase<R (Args...)>;
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
   OperationBase::init(buf(), Alignment);
  }

  //Function pointer constructor
  template<FunctionOperator<R, Args...> F>
  requires (NoexceptFunction<F>)
  Lambda(F * f) : Lambda() {
   operator=<F>(f);
  }

  //FunctionOperator copy constructor
  template<FunctionOperator<R, Args...> F>
  requires (!IsLambda<F> && NoexceptFunction<F>)
  Lambda(F const &f) : Lambda() {
   operator=<F>((F const&)f);
  }

  //FunctionOperator move constructor
  template<FunctionOperator<R, Args...> F>
  requires (!IsLambda<F> && NoexceptFunction<F>)
  Lambda(ConstDecayed<F> &&f) : Lambda() {
   operator=<F>((ConstDecayed<F>&&)f);
  }

  //Lambda copy constructor
  template<CompatibleLambda<Lambda> L>
  Lambda(L const &l) : Lambda() {
   operator=<L>((L const&)l);
  }

  //Lambda move constructor
  template<CompatibleLambda<Lambda> L>
  Lambda(ConstDecayed<L> &&l) : Lambda() {
   operator=<L>((ConstDecayed<L>&&)l);
  }

  ~Lambda() {
   //Destruct current buffer (guaranteed to be initialized)
   OperationBase::destroy(buf());
  }

  //Lambda function operator
  R operator()(Args... args) noexcept {
   return (*(LambdaBase *)&buffer).noexceptOp(args...);
  }

  //TODO all assignment operators
 };

 //TODO C-Variadic lambda support
 template<typename R, typename... Args>
 struct Lambda<R (Args..., ...)> {};

 template<typename R, typename... Args>
 struct Lambda<R (Args..., ...) noexcept> {};

 //Deduction guides for Lambda
 template<typename F>
 requires (StaticFunction<F>)
 Lambda(F const&) -> Lambda<FunctionPrototype<F>>;

 template<typename F>
 requires (FunctionOperator<F>)
 Lambda(F const&) -> Lambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename F>
 requires (StaticFunction<F>)
 Lambda(F&&) -> Lambda<FunctionPrototype<F>>;

 template<typename F>
 requires (FunctionOperator<F>)
 Lambda(F&&) -> Lambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename L>
 requires (IsLambda<L>)
 Lambda(L const&) -> Lambda<typename L::FunctionType>;

 template<typename L>
 requires (IsLambda<L>)
 Lambda(L&&) -> Lambda<typename L::FunctionType>;

 //TODO AllocLambda implementation
 #ifdef CX_STL_SUPPORT
  template<typename R, typename... Args>
  struct AllocLambda<R (Args...)> {};

  template<typename R, typename... Args>
  struct AllocLambda<R (Args...) noexcept > {};

  template<typename R, typename... Args>
  struct AllocLambda<R (Args..., ...)> {};

  template<typename R, typename... Args>
  struct AllocLambda<R (Args..., ...) noexcept> {};

 #endif //CX_STL_SUPPORT

 //Deduction guides for AllocLambda
 template<typename F>
 requires (StaticFunction<F>)
 AllocLambda(F const&) -> AllocLambda<FunctionPrototype<F>>;

 template<typename F>
 requires (FunctionOperator<F>)
 AllocLambda(F const&) -> AllocLambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename F>
 requires (StaticFunction<F>)
 AllocLambda(F&&) -> AllocLambda<FunctionPrototype<F>>;

 template<typename F>
 requires (FunctionOperator<F>)
 AllocLambda(F&&) -> AllocLambda<FunctionPrototype<decltype(&F::operator())>>;

 template<typename L>
 requires (IsLambda<L>)
 AllocLambda(L const&) -> AllocLambda<typename L::FunctionType>;

 template<typename L>
 requires (IsLambda<L>)
 AllocLambda(L&&) -> AllocLambda<typename L::FunctionType>;
}
