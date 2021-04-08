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
 struct UninitializedLambdaError CX_STL_SUPPORT_EXPR(: std::exception) {
  char const * what() const noexcept CX_STL_SUPPORT_EXPR(override) {
   return "Lambda is unitialized";
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
  ::IsLambda<T>
  ::Value;

 //Compatible lambda types identity (see above)
 template<typename L2, typename L1>
 concept CompatibleLambda = LambdaMetaFunctions
  ::CompatibleLambda<L1, L2>
  ::Value;

 //Lambda utilities
 namespace Internal {
  //The base of all lambda buffers
  template<typename>
  struct LambdaBase;

  //Base of unqualified and `noexcept` qualified function types
  template<typename R, typename... Args>
  struct LambdaBase<R (Args...)> {
   virtual ~LambdaBase() = default;

   virtual R op(Args...) {
    throw UninitializedLambdaError{};
   }

   virtual R noexceptOp(Args...) noexcept {
    throw UninitializedLambdaError{};
   }
  };

  //Base of unqualified and `noexcept` qualified c-variadic function types
  template<typename R, typename... Args>
  struct LambdaBase<R (Args..., ...)> {
   using FuncType = R (Args..., ...);
   using NoexceptFuncType = R (Args..., ...) noexcept;

   virtual ~LambdaBase() = default;

   virtual FuncType * get() {
    throw UninitializedLambdaError{};
   }

   virtual NoexceptFuncType * getNoexcept() {
    throw UninitializedLambdaError{};
   }

   template<typename... Varargs>
   R op(Args... args, Varargs... varargs) {
    return get()(args..., varargs...);
   }

   template<typename... Varargs>
   R noexceptOp(Args... args, Varargs... varargs) {
    return getNoexcept()(args..., varargs...);
   }
  };

  //Base of all lambda internal operations
  template<typename>
  struct LambdaOperationBase;

  //LambdaOperationBase implementation for non-c-variadic functions
  template<typename R, typename... Args>
  struct LambdaOperationBase<R (Args...)> {
   using LambdaBase = LambdaBase<R (Args...)>;

   //Initializes buffer with empty `LambdaBase` obj
   template<auto Size>
   [[gnu::always_inline]]
   static void init(unsigned char (&buffer)[Size]) {
    new (&buffer) LambdaBase{};
   }

   //Destructs current `LambdaBase` obj in buffer
   template<auto Size>
   [[gnu::always_inline]]
   static void destroy(unsigned char (&buffer)[Size]) {
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

    //Wrapper for encapsulated type
    struct LambdaSpecialized : LambdaBase {
     F f;

     LambdaSpecialized() = default;

     LambdaSpecialized(F const &f) :
      f((F const&)f)
     {}

     R op(Args... args) override {
      return f(args...);
     }

     R noexceptOp(Args... args) noexcept override {
      return f(args...);
     }
    };

    //Initialize buffer
    if constexpr (CopyConstructible<F>) {
     //Copy construct encapsulated type
     new (&buffer) LambdaSpecialized{(F const&)f};
    } else if constexpr (Constructible<F> && CopyAssignable<F>) {
     //Default construct encapsulated type and copy assign to it
     (*new (&buffer) LambdaSpecialized{}).f = (F const&)f;
    }
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

    //Wrapper for encapsulated type
    struct LambdaSpecialized : LambdaBase {
     F f;

     LambdaSpecialized() = default;

     LambdaSpecialized(F &&f) :
      f((F&&)f)
     {}

     R op(Args... args) override {
      return f(args...);
     }

     R noexceptOp(Args... args) noexcept override {
      return f(args...);
     }
    };

    //Initialize buffer
    if constexpr (MoveConstructible<F>) {
     new (&buffer) LambdaSpecialized{(F&&)f};
    } else if constexpr (Constructible<F> && MoveAssignable<F>) {
     (*new (&buffer) LambdaSpecialized{}).f = (F&&)f;
    }
   }

   //TODO lambda copy and move assignment
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

  static constexpr auto const Alignment = CX_LAMBDA_BUF_ALIGN;
  static constexpr auto const Size = CX_LAMBDA_BUF_SIZE;

 private:
  using OperationBase = Internal::LambdaOperationBase<R (Args...)>;
  using LambdaBase = typename OperationBase::LambdaBase;

  alignas(Alignment) unsigned char buffer[Size];

  //Return mutable reference to `buffer`
  auto& buf() const noexcept {
   auto &ref = const_cast<Lambda&>(*this);
   return ref.buffer;
  }

  //Ensure `T` will fit within `buffer`
  template<typename T>
  static void check() noexcept {
   //Note: Add `sizeof(void *)` to account for vtable ptr
   static_assert(
    (sizeof(T) + sizeof(void *)) <= Alignment,
    "The required alignment of the given functor is larger than the "
    "current 'CX_LAMBDA_BUF_ALIGN' value. Consider increasing it to "
    "the next power of 2."
   );
   static_assert(
    alignof(T) <= Size,
    "The size of the given functor is larger than the current "
    "'CX_LAMBDA_BUF_SIZE' value. Consider increasing it to the next "
    "power of 2."
   );
  }

 public:
  //Default constructor
  Lambda() {
   //Default initialize buffer for empty lambda
   OperationBase::init(buf());
  }

  //Function pointer constructor
  template<StaticFunction<R, Args...> F>
  Lambda(F * f) : Lambda() {
   operator=<F>(f);
  }

  //FunctionOperator copy constructor
  template<FunctionOperator<R, Args...> F>
  Lambda(F const &f) : Lambda() {
   operator=<F>((F const&)f);
  }

  //FunctionOperator move constructor
  template<FunctionOperator<R, Args...> F>
  Lambda(F &&f) : Lambda() {
   operator=<F>((F&&)f);
  }

  //Lambda copy constructor
  template<CompatibleLambda<Lambda> L>
  Lambda(L const &l) : Lambda() {
   operator=<L>((L const&)l);
  }

  //Lambda move constructor
  template<CompatibleLambda<Lambda> L>
  Lambda(L &&l) : Lambda() {
   operator=<L>((L&&)l);
  }

  ~Lambda() {
   //Destruct current buffer (guaranteed to be initialized)
   OperationBase::destroy(buf());
  }

  //Lambda function operator
  R operator()(Args... args) const {
   return (*(typename OperationBase::LambdaBase *)&buffer).op(args...);
  }

  //Function pointer assignment operator
  template<StaticFunction<R, Args...> F>
  Lambda& operator=(F * f) {
   //Ensure `F` will fit within `buffer`
   check<F *>();

   //Initalize buffer
   OperationBase::copyAssignFunction(buf(), (F * const&)f);

   return *this;
  }

  //FunctionOperator copy assignment
  template<FunctionOperator<R, Args...> F>
  Lambda& operator=(F const &f) {
   //Ensure `F` will fit within `buffer`
   check<F>();

   //Initialize buffer
   OperationBase::copyAssignFunction(buf(), (F const&)f);

   return *this;
  }

  //FunctionOperator move assignment
  template<FunctionOperator<R, Args...> F>
  Lambda& operator=(F &&f) {
   //Ensure `F` will fit within `buffer`
   check<F>();

   //Initialize buffer
   OperationBase::moveAssignFunction(buf(), (F&&)f);

   return *this;
  }

  //TODO Lambda copy assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(L const &l) {
   throw l;
   return *this;
  }

  //TODO Lambda move assignment
  template<CompatibleLambda<Lambda> L>
  Lambda& operator=(L &&l) {
   throw l;
   return *this;
  }
 };

 template<typename R, typename... Args>
 struct Lambda<R (Args...) noexcept> {
  template<typename>
  friend struct Lambda;

  using ReturnType = R;
  using ArgumentTypes = Dummy<Args...>;

  //TODO
 };

 //TODO C-Variadic lambda support
 template<typename R, typename... Args>
 struct Lambda<R (Args..., ...)> {};

 template<typename R, typename... Args>
 struct Lambda<R (Args..., ...) noexcept> {};

 //Deduction guides for lambda
 template<typename F>
 requires (FunctionOperator<F> || StaticFunction<F>)
 Lambda(F) -> Lambda<F>;
}
