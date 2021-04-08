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
 #define CX_LAMBDA_BUF_ALIGN 64
 #define CX_LAMBDA_BUF_SIZE 64
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

 //TODO lambda meta-functions
 namespace MetaFunctions {
  //TODO:
  // - qualified lambda -> unqualified lambda
  // - no-alloc lambda -> alloc lambda
  // - alloc lambda (w/ size & align restrictions met) -> no-alloc lambda
  template<typename>
  struct CompatibleLambda;
 }

 //TODO
 template<typename T>
 concept CompatibleLambda = false;

 //Unqualified lambda type
 template<typename R, typename... Args>
 struct Lambda<R (Args...)> {
  template<typename>
  friend struct Lambda;

  using ReturnType = R;
  using ArgumentTypes = Dummy<Args...>;

  static constexpr auto const Alignment = CX_LAMBDA_BUF_ALIGN;
  static constexpr auto const Size = CX_LAMBDA_BUF_SIZE;

 private:
  struct LambdaBase {
   virtual ~LambdaBase() = default;
   virtual R operator()(Args...) {
    throw UninitializedLambdaError{};
   }
  };

  alignas(Alignment) unsigned char buffer[Size];

  void destruct() const {
   auto &ref = const_cast<Lambda&>(*this);
   (*(LambdaBase *)&ref.buffer).~LambdaBase();
  }

  //Ensure `T` will fit within `buffer`
  template<typename T>
  void check() const noexcept {
   static_assert(
    sizeof(T) <= Alignment,
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

  //Copy function pointer/lambda assignment
  template<typename F>
  requires (
   StaticFunction<F, R, Args...>
   || (FunctionOperator<F, R, Args...>
    && (CopyConstructible<F> || (Constructible<F> && CopyAssignable<F>))
   )
  )
  void assign(F const &f) const {
   auto &ref = const_cast<Lambda&>(*this);

   //Clean up current state
   destruct();

   //Wrapper for encapsulated type
   struct LambdaSpecialized : LambdaBase {
    F f;

    LambdaSpecialized(F const &f) :
     f(f)
    {}

    R operator()(Args... args) override {
     return f(args...);
    }
   };

   //Ensure `LambdaSpecialized` will fit within `buffer`
   check<LambdaSpecialized>();

   //Initialize buffer
   if constexpr (CopyConstructible<F>) {
    //Copy construct encapsulated type
    new (&ref.buffer) LambdaSpecialized{f};
   } else if constexpr (Constructible<F> && CopyConstructible<F>) {
    //Default construct encapsulated type and then copy assign to it
    (*new (&ref.buffer) LambdaSpecialized{}).f = (F const&)f;
   }
  }

  //Move lambda assignment
  template<FunctionOperator<R, Args...> F>
  requires (!Const<F>
   && (MoveConstructible<F> || (Constructible<F> && MoveAssignable<F>))
  )
  void assign(F &&f) const {
   auto &ref = const_cast<Lambda&>(*this);

   //Clean up current state
   destruct();

   //Wrapper for encapsulated type
   struct LambdaSpecialized : LambdaBase {
    F f;

    LambdaSpecialized() = default;

    LambdaSpecialized(F &&f) :
     f((F&&)f)
    {}

    R operator()(Args... args) override {
     return f(args...);
    }
   };

   //Ensure 'LambdaSpecialized' will fit within 'buffer'
   check<LambdaSpecialized>();

   //Initialize buffer
   if constexpr (MoveConstructible<F>) {
    //Move construct encapsulated type
    new (&ref.buffer) LambdaSpecialized{(F&&)f};
   } else if constexpr (Constructible<F> && MoveAssignable<F>) {
    //Default construct encapsulated type and move assign to it
    (*new (&ref.buffer) LambdaSpecialized{}).f = (F&&)f;
   }
  }

 public:
  Lambda() {
   new (&buffer) LambdaBase {};
  }

  template<StaticFunction<R, Args...> F>
  Lambda(F * f) : Lambda() {
   assign<F *>((F * const&)f);
  }

  template<FunctionOperator<R, Args...> F>
  Lambda(F const &f) : Lambda() {
   assign<F>((F const&)f);
  }

  template<FunctionOperator<R, Args...> F>
  Lambda(F &&f) : Lambda() {
   assign<F>((F&&)f);
  }

  //TODO
  // - lambda copy constructor
  // - lambda move constructor

  ~Lambda() {
   destruct();
  }

  R operator()(Args... args) const {
   return (*(LambdaBase *)&buffer)(args...);
  }

  //TODO
  // - fptr copy assignment
  // - function copy assignment
  // - function move assignment
  // - lambda copy assignment
  // - lambda move assignment
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
