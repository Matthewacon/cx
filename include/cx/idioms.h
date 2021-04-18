#pragma once

#include <cx/common.h>

//Optional STL dependencies for construction constraints
//Adds placement new operator
#ifdef CX_STL_SUPPORT
 #include <new>
#else
 [[nodiscard]]
 void * operator new(decltype(sizeof(0)), void *) noexcept;
#endif

namespace CX {
 //Utility macro for defining conditional requirements
 #define CX_CONDITIONAL_CONSTRAINT(condition, constraint, defaultCase) \
 ((condition && constraint) || (defaultCase && !condition))

 //Template meta-functions, support for concept definitions
 namespace MetaFunctions {
  //True for matching types
  template<typename, typename>
  struct SameType : FalseType {};

  template<typename T>
  struct SameType<T, T> : TrueType {};

  //True for parameter packs that contain the first parameter
  template<typename... Types>
  struct MatchAnyType;

  template<typename... Types>
  requires (sizeof...(Types) <= 1)
  struct MatchAnyType<Types...> : FalseType {};

  template<typename T, typename... Args>
  struct MatchAnyType<T, Args...> {
   static constexpr auto const Value = (SameType<T, Args>::Value || ...);
  };

  //True for matching unspecialized templates
  template<template<typename...> typename, template<typename...> typename>
  struct SameTemplateType : FalseType {};

  template<template<typename...> typename T>
  struct SameTemplateType<T, T> : TrueType {};

  //True for parameter packs that contain the first parameter
  template<template<typename...> typename...>
  struct MatchAnyTemplateType;

  template<template<typename...> typename... Types>
  requires (sizeof...(Types) <= 1)
  struct MatchAnyTemplateType<Types...> : FalseType {};

  template<template<typename...> typename T, template<typename...> typename... Args>
  struct MatchAnyTemplateType<T, Args...> {
   static constexpr auto const Value = (SameTemplateType<T, Args>::Value || ...);
  };

  //True for matching values
  template<auto, auto>
  struct SameValue : FalseType {};

  template<auto V>
  struct SameValue<V, V> : TrueType {};

  //True for value packs that contain the first value
  template<auto...>
  struct MatchAnyValue;

  template<auto... Values>
  requires (sizeof...(Values) <= 1)
  struct MatchAnyValue<Values...> : FalseType {};

  template<auto V, auto... Values>
  struct MatchAnyValue<V, Values...> {
   static constexpr auto const Value = (SameValue<V, Values>::Value || ...);
  };

  //True for type packs that do not repeat parameters
  template<typename...>
  struct UniqueTypes;

  template<typename... Types>
  requires (sizeof...(Types) <= 1)
  struct UniqueTypes<Types...> : TrueType {
  private:
   template<typename...>
   friend struct UniqueTypes;

   static constexpr auto const FoundMatch = false;
  };

  template<typename T, typename... Types>
  requires (sizeof...(Types) > 0)
  struct UniqueTypes<T, Types...> {
  private:
   template<typename...>
   friend struct UniqueTypes;

   static constexpr auto const FoundMatch = MatchAnyType<T, Types...>::Value
    || UniqueTypes<Types...>::FoundMatch;

  public:
   static constexpr auto const Value = !FoundMatch;
  };

  //True for template-type packs that do not repeat parameters
  template<template<typename...> typename...>
  struct UniqueTemplateTypes;

  template<template<typename...> typename... Types>
  requires (sizeof...(Types) <= 1)
  struct UniqueTemplateTypes<Types...> : TrueType {
  private:
   template<template<typename...> typename...>
   friend struct UniqueTemplateTypes;

   static constexpr auto const FoundMatch = false;
  };

  template<template<typename...> typename T, template<typename...> typename... Types>
  requires (sizeof...(Types) > 0)
  struct UniqueTemplateTypes<T, Types...> {
  private:
   template<template<typename...> typename...>
   friend struct UniqueTemplateTypes;

   static constexpr auto const FoundMatch = MatchAnyTemplateType<T, Types...>::Value
    || UniqueTemplateTypes<Types...>::Value;

  public:
   static constexpr auto const Value = !FoundMatch;
  };

  //True for value packs that do not repeat parameters
  template<auto... Values>
  struct UniqueValues;

  template<auto... Values>
  requires (sizeof...(Values) <= 1)
  struct UniqueValues<Values...> : TrueType {
  private:
   template<auto...>
   friend struct UniqueValues;

   static constexpr auto const FoundMatch = false;
  };

  template<auto V, auto... Values>
  requires (sizeof...(Values) > 0)
  struct UniqueValues<V, Values...> {
  private:
   template<auto...>
   friend struct UniqueValues;

   static constexpr auto const FoundMatch = MatchAnyValue<V, Values...>::Value
    || UniqueValues<Values...>::FoundMatch;

  public:
   static constexpr auto const Value = !FoundMatch;
  };

  //Strips all type qualifiers
  template<typename T>
  struct Unqualified {
   using Type = T;
   using DecayedType = T;
  };

  template<typename T>
  struct Unqualified<T *> : Unqualified<T> {
   using DecayedType = T;
  };

  template<typename T>
  struct Unqualified<T&> : Unqualified<T> {
   using DecayedType = T;
  };

  template<typename T>
  struct Unqualified<T&&> : Unqualified<T> {
   using DecayedType = T&;
  };

  template<typename T>
  struct Unqualified<T[]> : Unqualified<T> {
   using DecayedType = T;
  };

  template<typename T>
  struct Unqualified<T const[]> : Unqualified<T> {
   using DecayedType = T const;
  };

  template<typename T, auto N>
  struct Unqualified<T[N]> : Unqualified<T> {
   using DecayedType = T[];
  };

  template<typename T, auto N>
  struct Unqualified<T const[N]> : Unqualified<T> {
   using DecayedType = T const[];
  };

  template<typename T>
  struct Unqualified<T const> : Unqualified<T> {
   using DecayedType = T;
  };

  template<typename T>
  struct Unqualified<T volatile> : Unqualified<T> {
   using DecayedType = T;
  };

  //Const qualifier identity
  template<typename T>
  struct Const : FalseType {
   using Type = T;
   using ConstDecayed = T;

   //Does not propagate const qualifier to `T2`
   template<typename T2>
   using Propagate = T2;
  };

  template<typename T>
  struct Const<T const> : TrueType {
   using Type = T const;
   using ConstDecayed = T;

   //Propagates const qualifier to `T2`
   template<typename T2>
   using Propagate = T2 const;
  };

  template<typename T>
  struct Const<T const&> : Const<T const> {
   using Type = T const&;
   using ConstDecayed = T&;
  };

  template<typename T>
  struct Const<T const&&> : Const<T const> {
   using Type = T const&&;
   using ConstDecayed = T&&;
  };

  //Volatile qualifier identity
  template<typename T>
  struct Volatile : FalseType {
   using Type = T;
   using VolatileDecayed = T;

   //Does not propagate volatile qualifier to `T2`
   template<typename T2>
   using Propagate = T2;
  };

  template<typename T>
  struct Volatile<T volatile> : TrueType {
   using Type = T volatile;
   using VolatileDecayed = T;

   //Propagates volatile qualifier to `T2`
   template<typename T2>
   using Propagate = T2 volatile;
  };

  //Decays both const and volatile qualifiers
  template<typename T>
  using ConstVolatileDecayed = typename Const<
   typename Volatile<T>::VolatileDecayed
  >::ConstDecayed;

  //Propagates both const and volatile qualifiers
  template<typename T1, typename T2>
  using ConstVolatilePropagated = typename Const<T1>::template Propagate<
   typename Volatile<T1>::template Propagate<T2>
  >;

  //Array identity
  template<typename T>
  struct Array : FalseType {
   using ElementType = T;
   static constexpr auto const Size = -1;
   static constexpr auto const Sized = false;
  };

  template<typename T>
  struct Array<T[]> : TrueType {
   using ElementType = T;
   static constexpr auto const Size = -1;
   static constexpr auto const Sized = false;
  };

  template<auto N, typename T>
  struct Array<T[N]> : TrueType {
   using ElementType = T;
   static constexpr auto const Size = N;
   static constexpr auto const Sized = true;
  };

  //Note: This idiom will not work for zero-length-arrays
  //in clang due to a bugs in the zero-length-array extension
  //implementation
  //See: https://bugs.llvm.org/show_bug.cgi?id=49808
  template<typename T>
  struct Array<T[0]> : TrueType {
   using ElementType = T;
   static constexpr auto const Size = 0;
   static constexpr auto const Sized = true;
  };

  //Pointer idiom
  template<typename>
  struct Pointer : FalseType {};

  template<typename T>
  struct Pointer<T *> : TrueType {};

  template<>
  struct Pointer<decltype(nullptr)> : TrueType {};

  //lvalue reference identity
  template<typename T>
  struct LValueReference : FalseType {
   using ElementType = T;
  };

  template<typename T>
  struct LValueReference<T&> : TrueType {
   using ElementType = T;
  };

  //rvalue reference identity
  template<typename T>
  struct RValueReference : FalseType {
   using ElementType = T;
  };

  template<typename T>
  struct RValueReference<T&&> : TrueType {
   using ElementType = T;
  };

  //Signed type conversions; decay/promotion
  template<
   typename T,
   typename CVDecayed = ConstVolatileDecayed<T>,
   bool = __is_enum(CVDecayed)
  >
  struct Sign {
   using SignedType = T;
   using UnsignedType = T;
  };

  #define DEFINE_SIGN_SPECIALIZATION(signedType, unsignedType) \
  template<typename T>\
  struct Sign<T, signedType, false> {\
   using SignedType = ConstVolatilePropagated<T, signedType>;\
   using UnsignedType = ConstVolatilePropagated<T, unsignedType>;\
  };\
  \
  template<typename T>\
  struct Sign<T, unsignedType, false> {\
   using SignedType = ConstVolatilePropagated<T, signedType>;\
   using UnsignedType = ConstVolatilePropagated<T, unsignedType>;\
  }

  //Signed integer specializations
  DEFINE_SIGN_SPECIALIZATION(signed char,      unsigned char);
  DEFINE_SIGN_SPECIALIZATION(signed short,     unsigned short);
  DEFINE_SIGN_SPECIALIZATION(signed int,       unsigned int);
  DEFINE_SIGN_SPECIALIZATION(signed long,      unsigned long);
  DEFINE_SIGN_SPECIALIZATION(signed long long, unsigned long long);

  #undef DEFINE_SIGN_SPECIALIZATION

  //Char specialization
  template<typename T>
  struct Sign<T, char, false> {
   using SignedType = ConstVolatilePropagated<T, signed char>;
   using UnsignedType = ConstVolatilePropagated<T, unsigned char>;
   static constexpr auto const Signed = char(-1) < char(0);
   static constexpr auto const Unsigned = !Signed;
  };

  //Utility for `Sign<...>`
  template<typename Target, typename...>
  struct SmallestMatchingTypeSize;

  template<typename Target, typename T, typename... Types>
  struct SmallestMatchingTypeSize<Target, T, Types...> {
   template<bool Match = sizeof(Target) == sizeof(T), auto = 0>
   struct Producer;
  };

  template<typename Target, typename T, typename... Types>
  template<auto _>
  struct SmallestMatchingTypeSize<Target, T, Types...>::Producer<false, _> {
   using Type = typename SmallestMatchingTypeSize<Target, Types...>
    ::template Producer<>
    ::Type;
  };

  template<typename Target, typename T, typename... Types>
  template<auto _>
  struct SmallestMatchingTypeSize<Target, T, Types...>::Producer<true, _> {
   using Type = T;
  };

  template<typename Target>
  struct SmallestMatchingTypeSize<Target> {
   template<bool>
   struct Producer {
    using Type = void;
   };
  };

  //Utility for `Sign<...>`
  template<typename Target>
  using EquivalentIntegerType = typename SmallestMatchingTypeSize<
   Target,
   char,
   short,
   int,
   long,
   long long
  >
   ::template Producer<>
   ::Type;

  #define DEFINE_SPECIAL_CASE_SIGN_SPECIALIZATION(specialType) \
  template<typename T>\
  struct Sign<T, specialType, false> {\
  private:\
   using SignEquivalent = Sign<EquivalentIntegerType<specialType>>;\
  \
  public:\
   using SignedType = ConstVolatilePropagated<\
    T,\
    typename SignEquivalent::SignedType\
   >;\
   using UnsignedType = ConstVolatilePropagated<\
    T,\
    typename SignEquivalent::UnsignedType\
   >;\
  }

  //Extension char specializations
  DEFINE_SPECIAL_CASE_SIGN_SPECIALIZATION(wchar_t);
  DEFINE_SPECIAL_CASE_SIGN_SPECIALIZATION(char8_t);
  DEFINE_SPECIAL_CASE_SIGN_SPECIALIZATION(char16_t);
  DEFINE_SPECIAL_CASE_SIGN_SPECIALIZATION(char32_t);

  #undef DEFINE_SPECIAL_CASE_SIGN_SPECIALIZATION

  //Enum specialization
  template<typename T>
  struct Sign<T, ConstVolatileDecayed<T>, true> {
  private:
   using SignEquivalent = Sign<EquivalentIntegerType<T>>;

  public:
   using SignedType = ConstVolatilePropagated<
    T,
    typename SignEquivalent::SignedType
   >;
   using UnsignedType = ConstVolatilePropagated<
    T,
    typename SignEquivalent::UnsignedType
   >;
  };

  //Function identity meta-functions
  //Member function idiom
  template<typename>
  struct MemberFunction : FalseType {
   using Type = ImpossibleType<>;
   using ReturnType = ImpossibleType<>;
   template<template<typename...> typename Receiver = ImpossibleType>
   using ArgumentTypes = Receiver<>;
   using FunctionPrototype = ImpossibleType<>;
   using MemberFunctionPrototype = ImpossibleType<>;
   static constexpr auto const Const = false;
   static constexpr auto const Volatile = false;
   static constexpr auto const Noexcept = false;
   static constexpr auto const Variadic = false;
   static constexpr auto const LValueRef = false;
   static constexpr auto const RValueRef = false;
  };

  //Base unqualified member function case
  template<typename T, typename R, typename... Args>
  struct MemberFunction<R (T::*)(Args...)> : MemberFunction<void> {
   using Type = T;
   using ReturnType = R;
   template<template<typename...> typename Receiver = Dummy>
   using ArgumentTypes = Receiver<Args...>;
   using FunctionPrototype = R (Args...);
   using MemberFunctionPrototype = R (T::*)(Args...);
   static constexpr auto const Value = true;
  };

  #define DEFINE_MEMBER_FUNCTION_IDIOM_CASE(field, prototype, staticPrototype, base) \
  template<typename T, typename R, typename... Args>\
  struct MemberFunction<prototype> : base {\
   using FunctionPrototype = staticPrototype;\
   using MemberFunctionPrototype = prototype;\
   static constexpr auto const field = true;\
  }

  //All base qualifiers
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const,     R (T::*)(Args...) const,    R (Args...),          MemberFunction<R (T::*)(Args...)>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile,  R (T::*)(Args...) volatile, R (Args...),          MemberFunction<R (T::*)(Args...)>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Noexcept,  R (T::*)(Args...) noexcept, R (Args...) noexcept, MemberFunction<R (T::*)(Args...)>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Variadic,  R (T::*)(Args..., ...),     R (Args..., ...),     MemberFunction<R (T::*)(Args...)>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(LValueRef, R (T::*)(Args...) &,        R (Args...),          MemberFunction<R (T::*)(Args...)>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(RValueRef, R (T::*)(Args...) &&,       R (Args...),          MemberFunction<R (T::*)(Args...)>);

  //Const product expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const volatile, R (Args...),          MemberFunction<R (T::*)(Args...) volatile>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const noexcept, R (Args...) noexcept, MemberFunction<R (T::*)(Args...) noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const,     R (Args..., ...),     MemberFunction<R (T::*)(Args..., ...)>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const &,        R (Args...),          MemberFunction<R (T::*)(Args...) &>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const &&,       R (Args...),          MemberFunction<R (T::*)(Args...) &&>);

  //Volatile product expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args...) volatile noexcept, R (Args...) noexcept, MemberFunction<R (T::*)(Args...) noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args..., ...) volatile,     R (Args..., ...),     MemberFunction<R (T::*)(Args..., ...)>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args...) volatile &,        R (Args...),          MemberFunction<R (T::*)(Args...) &>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args...) volatile &&,       R (Args...),          MemberFunction<R (T::*)(Args...) &&>);

  //Noexcept product expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Noexcept, R (T::*)(Args..., ...) noexcept, R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...)>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Noexcept, R (T::*)(Args...) & noexcept,    R (Args...) noexcept,      MemberFunction<R (T::*)(Args...) &>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Noexcept, R (T::*)(Args...) && noexcept,   R (Args...) noexcept,      MemberFunction<R (T::*)(Args...) &&>);

  //Variadic product expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Variadic, R (T::*)(Args..., ...) &,  R (Args..., ...), MemberFunction<R (T::*)(Args...) &>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Variadic, R (T::*)(Args..., ...) &&, R (Args..., ...), MemberFunction<R (T::*)(Args...) &&>);

  //Note: l/r-value reference qualifiers cannot both be applied
  //to the same member function, so there is no need to expand
  //them.

  //Const-volatile expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const volatile noexcept, R (Args...) noexcept, MemberFunction<R (T::*)(Args...) volatile noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const volatile,     R (Args..., ...),     MemberFunction<R (T::*)(Args..., ...) volatile>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const volatile &,        R (Args...),          MemberFunction<R (T::*)(Args...) volatile &>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const volatile &&,       R (Args...),          MemberFunction<R (T::*)(Args...) volatile &&>);

  //Const-noexcept expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const noexcept, R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...) noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const & noexcept,    R (Args...) noexcept,      MemberFunction<R (T::*)(Args...) & noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const && noexcept,   R (Args...) noexcept,      MemberFunction<R (T::*)(Args...) && noexcept>);

  //Const-variadic expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const &,  R (Args..., ...), MemberFunction<R (T::*)(Args..., ...) &>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const &&, R (Args..., ...), MemberFunction<R (T::*)(Args..., ...) &&>);

  //Volatile-noexcept expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args..., ...) volatile noexcept, R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...) noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args...) volatile & noexcept,    R (Args...) noexcept,      MemberFunction<R (T::*)(Args...) & noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args...) volatile && noexcept,   R (Args...) noexcept,      MemberFunction<R (T::*)(Args...) && noexcept>);

  //Volatile-variadic expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args..., ...) volatile &,  R (Args..., ...), MemberFunction<R (T::*)(Args..., ...) &>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args..., ...) volatile &&, R (Args..., ...), MemberFunction<R (T::*)(Args..., ...) &&>);

  //Noexcept-variadic expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Noexcept, R (T::*)(Args..., ...) & noexcept,  R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...) &>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Noexcept, R (T::*)(Args..., ...) && noexcept, R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...) &&>);

  //Const-volatile-noexcept expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const volatile noexcept, R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...) volatile noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const volatile & noexcept,    R (Args...) noexcept, MemberFunction<R (T::*)(Args...) volatile & noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args...) const volatile && noexcept,   R (Args...) noexcept, MemberFunction<R (T::*)(Args...) volatile && noexcept>);

  //Const-volatile-variadic expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const volatile &,  R (Args..., ...), MemberFunction<R (T::*)(Args..., ...) volatile &>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const volatile &&, R (Args..., ...), MemberFunction<R (T::*)(Args..., ...) volatile &&>);

  //Const-noexcept-variadic expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const & noexcept,  R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...) & noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Const, R (T::*)(Args..., ...) const && noexcept, R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...) && noexcept>);

  //Volatile-noexcept-variadic expansion
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args..., ...) volatile & noexcept,  R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...) & noexcept>);
  DEFINE_MEMBER_FUNCTION_IDIOM_CASE(Volatile, R (T::*)(Args..., ...) volatile && noexcept, R (Args..., ...) noexcept, MemberFunction<R (T::*)(Args..., ...) && noexcept>);

  #undef DEFINE_MEMBER_FUNCTION_IDIOM_CASE

  //Static function idiom
  template<typename>
  struct StaticFunction : FalseType {
   using ReturnType = ImpossibleType<>;
   template<template<typename...> typename Receiver = ImpossibleType>
   using ArgumentTypes = Receiver<>;
   using FunctionPrototype = ImpossibleType<>;
   static constexpr auto const Noexcept = false;
   static constexpr auto const Variadic = false;
  };

  //Base unqualified static function case
  template<typename R, typename... Args>
  struct StaticFunction<R (Args...)> : StaticFunction<void> {
   using ReturnType = R;
   template<template<typename...> typename Receiver = Dummy>
   using ArgumentTypes = Receiver<Args...>;
   using FunctionPrototype = R (Args...);
   template<typename T = Dummy<>>
   using MemberFunctionPrototype = R (T::*)(Args...);
   static constexpr auto const Value = true;
  };

  #define DEFINE_STATIC_FUNCTION_IDIOM_CASE(field, prototype, memberPrototype, base) \
  template<typename R, typename... Args>\
  struct StaticFunction<prototype> : base {\
   using FunctionPrototype = prototype;\
   template<typename T = Dummy<>>\
   using MemberFunctionPrototype = memberPrototype;\
   static constexpr auto const field = true;\
  }

  //All qualifiers and qualifier combinations
  DEFINE_STATIC_FUNCTION_IDIOM_CASE(Noexcept, R (Args...) noexcept,      R (T::*)(Args...) noexcept,      StaticFunction<R (Args...)>);
  DEFINE_STATIC_FUNCTION_IDIOM_CASE(Variadic, R (Args..., ...),          R (T::*)(Args..., ...),          StaticFunction<R (Args...)>);
  DEFINE_STATIC_FUNCTION_IDIOM_CASE(Noexcept, R (Args..., ...) noexcept, R (T::*)(Args..., ...) noexcept, StaticFunction<R (Args..., ...)>);

  #undef DEFINE_STATIC_FUNCTION_IDIOM_CASE

  //Function prototype meta-function
  template<typename F>
  struct FunctionPrototype {
   using FunctionType = ImpossibleType<>;
  };

  template<typename F>
  requires (StaticFunction<F>::Value)
  struct FunctionPrototype<F> {
   using FunctionType = typename StaticFunction<F>::FunctionPrototype;
  };

  template<typename F>
  requires (MemberFunction<F>::Value)
  struct FunctionPrototype<F> {
   using FunctionType = typename MemberFunction<F>::FunctionPrototype;
  };

  //Virtual member function idiom
  template<auto F, typename = void>
  struct VirtualFunction {
   static constexpr auto const Value = MemberFunction<decltype(F)>::Value;
  };

  template<auto F>
  struct VirtualFunction<F, VoidA<F == F>> : FalseType {};

  //Member field idiom
  template<typename F, typename = typename Unqualified<F>::Type>
  struct MemberField : FalseType {
   using Type = ImpossibleType<>;
   using FieldType = ImpossibleType<>;
  };

  template<typename F, typename T, typename C>
  requires (!MemberFunction<F>::Value)
  struct MemberField<F, T C::*> : TrueType {
   using Type = C;
   using FieldType = T;
  };
 }

 //Concept/alias/value definitions
 //Yields result of type expresion: `T1 == T2`
 template<typename T1, typename T2>
 concept SameType = MetaFunctions
  ::SameType<T1, T2>
  ::Value;

 //True for type expression: `((Types[0] == Types) || ...)`
 template<typename... Types>
 concept MatchAnyType = MetaFunctions
  ::MatchAnyType<Types...>
  ::Value;

 //Yields result of type expression: `T1 == T2`
 template<template<typename...> typename T1, template<typename...> typename T2>
 concept SameTemplateType = MetaFunctions
  ::SameTemplateType<T1, T2>
  ::Value;

 //True for type expression: `((Types[0] == Types) || ...)`
 template<template<typename...> typename... Types>
 concept MatchAnyTemplateType = MetaFunctions
  ::MatchAnyTemplateType<Types...>
  ::Value;

 //Yields result of: `V1 == V2`
 template<auto V1, auto V2>
 concept SameValue = MetaFunctions
  ::SameValue<V1, V2>
  ::Value;

 //True for result of expression: `((Values[0] == Values) || ...)`
 template<auto... Values>
 concept MatchAnyValue = MetaFunctions
  ::MatchAnyValue<Values...>
  ::Value;

 //True for type expression: `!((Types[0] == Types) || ...)`
 template<typename... Types>
 concept UniqueTypes = MetaFunctions
  ::UniqueTypes<Types...>
  ::Value;

 //True for type expression: `!((Types[0] == Types) || ...)`
 template<template<typename...> typename... Types>
 concept UniqueTemplateTypes = MetaFunctions
  ::UniqueTemplateTypes<Types...>
  ::Value;

 //True for expression: `!((Values[0] == Values) || ...)`
 template<auto... Values>
 concept UniqueValues = MetaFunctions
  ::UniqueValues<Values...>
  ::Value;

 //True if `T1` is implicitly, or explicitly, convertible to `T2`
 template<typename T1, typename T2>
 concept ConvertibleTo = requires (T1 t1) {
  { (T2)t1 } -> SameType<T2>;
 };

 //True for types that contain a constructor definition with
 //the signature: `T::T(Args...)`
 template<typename T, typename... Args>
 concept Constructible = __is_constructible(T, Args...);

 //True for types with a non-deleted destructor
 //Note: False for array types
 template<typename T>
 concept Destructible = requires (T t) {
  { t.~T() } -> SameType<void>;
 };

 //True for types that contain a constructor definition with
 //the signature: `T::T(T const&)`
 template<typename T>
 concept CopyConstructible = __is_constructible(T, T const&)
  && requires (T * t1, T t2) {
   { new (t1) T {(T const&)t2} } -> SameType<T *>;
  };

 //True for types that contain a constructor definition with
 //the signature: `T::T(T&&)`
 template<typename T>
 concept MoveConstructible = __is_constructible(T, T&&)
  && requires (T * t1, T t2) {
   { new (t1) T {(T&&)t2} } -> SameType<T *>;
  };

 //True for types that contain an assignment operator definition
 //with the signature: `T& T::operator=(T const&)`
 template<typename T>
 concept CopyAssignable =
  requires (T t1, T t2) {
   { t1.operator=((T const&)t2) } -> SameType<T&>;
  }
  || requires (T t1, T t2) {
   { t1 = (T&)t2 } -> SameType<T&>;
  };

 //True for types that contain an assignment operator definition
 //with the signature: `T& operator=(T&&)`
 template<typename T>
 concept MoveAssignable =
  requires (T t1, T t2) {
   { t1.operator=((T&&)t2) } -> SameType<T&>;
  }
  || requires (T t1, T t2) {
   { t1 = (T&&)t2 } -> SameType<T&>;
  };

 //Yields the type `T`, stripped of all cv, pointer, reference
 //and array qualifiers
 template<typename T>
 using Unqualified = typename MetaFunctions
  ::Unqualified<T>
  ::Type;

 //Yields the type `T`, stripped of the rightmost cv, pointer,
 //reference or array qualifier
 template<typename T>
 using Decayed = typename MetaFunctions
  ::Unqualified<T>
  ::DecayedType;

 //Const identity
 template<typename T>
 concept Const = MetaFunctions
  ::Const<T>
  ::Value;

 //Yields the type `T` stripped of a const qualifier, if present
 template<typename T>
 using ConstDecayed = typename MetaFunctions
  ::Const<T>
  ::ConstDecayed;

 //Volatile identity
 template<typename T>
 concept Volatile = MetaFunctions
  ::Volatile<T>
  ::Value;

 //Yields the type `T` stripped of a volatile qualifier, if present
 template<typename T>
 using VolatileDecayed = typename MetaFunctions
  ::Volatile<T>
  ::VolatileDecayed;

 //Yields the type `T` stripped of cv qualifiers, if present
 template<typename T>
 using ConstVolatileDecayed = typename MetaFunctions::ConstVolatileDecayed<T>;

 //Yields the type `T2` with the same cv qualifiers as `T1`, if present
 template<typename T1, typename T2>
 using ConstVolatilePropagated = typename MetaFunctions::ConstVolatilePropagated<T1, T2>;

 //Array identity
 //Note: Supports both sized and unsized arrays
 template<typename T>
 concept Array = MetaFunctions
  ::Array<T>
  ::Value;

 //Sized array identity
 template<typename T>
 concept SizedArray = MetaFunctions
   ::Array<T>
   ::Sized;

 //Unsized array identity
 template<typename T>
 concept UnsizedArray = Array<T>
  && !MetaFunctions
   ::Array<T>
   ::Sized;

 //Yields element type of an array type
 //ie. `T[...] -> T`
 template<typename T>
 using ArrayElementType = typename MetaFunctions
  ::Array<T>
  ::ElementType;

 //Yields the size of an array type
 //Note: For unsized arrays, yields `-1`
 template<typename T>
 requires (Array<T>)
 constexpr auto const ArraySize = MetaFunctions
  ::Array<T>
  ::Size;

 //Pointer identity
 template<typename T>
 concept Pointer = MetaFunctions
  ::Pointer<T>
  ::Value;

 //Member pointer identity
 template<typename T>
 concept MemberPointer = MetaFunctions
  ::MemberField<T>
  ::Value
  || MetaFunctions
   ::MemberFunction<T>
   ::Value;

 //l-value reference identity
 template<typename T>
 concept LValueReference = MetaFunctions
  ::LValueReference<T>
  ::Value;

 //Yields the element type of a l-value refernece;
 //ie. `T const& -> T const`
 template<typename T>
 using LValueReferenceElementType = typename MetaFunctions
  ::LValueReference<T>
  ::ElementType;

 //r-value reference identity
 template<typename T>
 concept RValueReference = MetaFunctions
  ::RValueReference<T>
  ::Value;

 //Yields the element type of a r-value reference;
 //ie. `T&& -> T`
 template<typename T>
 using RValueReferenceElementType = typename MetaFunctions
  ::RValueReference<T>
  ::ElementType;

 //Produce unsigned type equivalent
 template<typename T>
 using SignDecayed = typename MetaFunctions
  ::Sign<T>
  ::UnsignedType;

 //Produce signed type equivalent
 template<typename T>
 using SignPromoted = typename MetaFunctions
  ::Sign<T>
  ::SignedType;

 //Enum identity
 template<typename T>
 concept Enum = __is_enum(T);

 //Union identity
 template<typename T>
 concept Union = __is_union(T);

 //Struct identity
 template<typename T>
 concept Struct = __is_class(T);

 //Integral identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_integral
 template<typename T>
 concept Integral = MatchAnyType<
  SignDecayed<ConstVolatileDecayed<T>>,
  bool,
  unsigned char,
  wchar_t,
  char8_t,
  char16_t,
  char32_t,
  unsigned short,
  unsigned int,
  unsigned long,
  unsigned long long
 >;

 //Floating precision identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_floating_point
 template<typename T>
 concept Floating = MatchAnyType<
  ConstVolatileDecayed<T>,
  float,
  double,
  long double
 >;

 //Arithmetic identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_arithmetic
 template<typename T>
 concept Arithmetic = Integral<T>
  || Floating<T>;

 //Signed identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_signed
 template<typename T>
 concept Signed = Arithmetic<T> && T(-1) < T(0);

 //Unsigned identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_unsigned
 template<typename T>
 concept Unsigned = (Arithmetic<T> && T(0) < T(-1))
  || SameType<Unqualified<T>, bool>;

 //Scalar identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_scalar
 template<typename T>
 concept Scalar = Arithmetic<T>
  || Enum<T>
  || Pointer<T>
  || MemberPointer<T>;

 //Trivially copyable identity
 //Defined by: https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable
 template<typename T>
 concept TriviallyCopyable = __is_trivially_copyable(T);

 //Trivial identity
 //Defined by: https://en.cppreference.com/w/cpp/named_req/TrivialType
 template<typename T>
 concept Trivial = Scalar<T>
  || TriviallyCopyable<T>;

 //Function identity concepts
 //Member function identity
 //Note: Has optional return type and argument type matching
 template<typename F, typename R = ImpossibleType<>, typename... Args>
 concept MemberFunction = MetaFunctions::MemberFunction<Unqualified<F>>::Value
  //Match member function against return type, if specified
  && CX_CONDITIONAL_CONSTRAINT(
   (!SameType<R, ImpossibleType<>>),
   (SameType<R, typename MetaFunctions::MemberFunction<Unqualified<F>>::ReturnType>),
   true
  )
  //Match member function against argument types
  //Note: Will only match against argument types if the return type
  // has been specified and there is at least one argument specified
  && CX_CONDITIONAL_CONSTRAINT(
   (!SameType<R, ImpossibleType<>> && sizeof...(Args) > 0),
   (SameType<Dummy<Args...>, typename MetaFunctions::MemberFunction<Unqualified<F>>::template ArgumentTypes<>>),
   true
  );

 //Static function identity
 //Note: Has optional return type and argument type matching
 template<typename F, typename R = ImpossibleType<>, typename... Args>
 concept StaticFunction = MetaFunctions::StaticFunction<Unqualified<F>>::Value
  //Match static function against return type, if specified
  && CX_CONDITIONAL_CONSTRAINT(
   (!SameType<R, ImpossibleType<>>),
   (SameType<R, typename MetaFunctions::StaticFunction<Unqualified<F>>::ReturnType>),
   true
  )
  //Match static function against argument types
  //Note: Same conditional logic as the argument matching case for
  // the `MemberFunction` concept
  && CX_CONDITIONAL_CONSTRAINT(
   (!SameType<R, ImpossibleType<>> && sizeof...(Args) > 0),
   (SameType<Dummy<Args...>, typename MetaFunctions::StaticFunction<Unqualified<F>>::template ArgumentTypes<>>),
   true
  );

 //Yields the prototype of a given function type;
 //For member functions, the member pointer is stripped from the type
 template<typename F>
 using FunctionPrototype = typename MetaFunctions
  ::FunctionPrototype<Unqualified<F>>
  ::FunctionType;

 //Yields the prototype of a ginen member function type;
 //For static functios, yields `ImpossibleType<>`
 template<typename F>
 using MemberFunctionPrototype = typename MetaFunctions
  ::MemberFunction<Unqualified<F>>
  ::MemberFunctionPrototype;

 namespace MetaFunctions {
  //Matching function prototype idiom
  template<typename F, typename Prototype>
  requires (CX::StaticFunction<Prototype> || CX::MemberFunction<Prototype>)
  struct FunctionWithPrototype : FalseType {};

  //`<static, static>` specialization
  template<typename F, typename Prototype>
  requires (CX::StaticFunction<Prototype> && CX::StaticFunction<F>)
  struct FunctionWithPrototype<F, Prototype> : SameType<
   typename StaticFunction<typename Unqualified<F>::Type>::FunctionPrototype,
   Prototype
  > {};

  //`<member, static>` specialization
  template<typename F, typename Prototype>
  requires (CX::StaticFunction<Prototype> && CX::MemberFunction<F>)
  struct FunctionWithPrototype<F, Prototype> : SameType<
   typename MemberFunction<typename Unqualified<F>::Type>::FunctionPrototype,
   Prototype
  > {};

  //`<member, member>` specialization
  template<typename F, typename Prototype>
  requires (CX::MemberFunction<Prototype> && CX::MemberFunction<F>)
  struct FunctionWithPrototype<F, Prototype> : SameType<
   typename Unqualified<F>::Type,
   Prototype
  > {};

  //Function operator specialization; `<struct, static>`
  //Note: Not all member function qualifiers are applicable
  //to static functions so the deduction capabilities of this
  //partial specialization are limited.
  template<typename F, typename Prototype>
  requires (CX::StaticFunction<Prototype>
   && CX::Struct<F>
   && requires (typename StaticFunction<Prototype>::template MemberFunctionPrototype<F> func) {
    func = &CX::ConstDecayed<F>::operator();
   }
  )
  struct FunctionWithPrototype<F, Prototype> : TrueType {};

  //Function operator specialization; `<struct, member>`
  template<typename F, typename Prototype>
  requires (CX::MemberFunction<Prototype>
   && CX::Struct<F>
   && requires (Prototype func) {
    func = &CX::ConstDecayed<F>::operator();
   }
  )
  struct FunctionWithPrototype<F, Prototype> : TrueType {};
 }

 //Identity concept for matching function types, as well as
 //function operators
 template<typename F, typename Prototype>
 concept FunctionWithPrototype = MetaFunctions
  ::FunctionWithPrototype<F, Prototype>
  ::Value;

 //Virtual function identity
 template<auto F>
 concept VirtualFunction = MetaFunctions
  ::VirtualFunction<F>
  ::Value;

 namespace MetaFunctions {
  //Variadic function idiom (member and static)
  template<typename Prototype>
  struct VariadicFunction : FalseType {};

  template<typename Prototype>
  requires (CX::StaticFunction<Prototype>)
  struct VariadicFunction<Prototype> {
   static constexpr auto const Value = StaticFunction<Prototype>::Variadic;
  };

  template<typename Prototype>
  requires (CX::MemberFunction<Prototype>)
  struct VariadicFunction<Prototype> {
   static constexpr auto const Value = MemberFunction<Prototype>::Variadic;
  };

  //Noexcept function idiom (member and static)
  template<typename Prototype>
  struct NoexceptFunction : FalseType {};

  template<typename Prototype>
  requires (CX::StaticFunction<Prototype>)
  struct NoexceptFunction<Prototype> {
   static constexpr auto const Value = StaticFunction<Prototype>::Noexcept;
  };

  template<typename Prototype>
  requires (CX::MemberFunction<Prototype>)
  struct NoexceptFunction<Prototype> {
   static constexpr auto const Value = MemberFunction<Prototype>::Noexcept;
  };
 }

 //Variadic function prototype identity concept
 template<typename Prototype>
 concept VariadicFunction = MetaFunctions
  ::VariadicFunction<Unqualified<Prototype>>
  ::Value;

 //Noexcept function prototype identity concept
 template<typename Prototype>
 concept NoexceptFunction = MetaFunctions
  ::NoexceptFunction<Unqualified<Prototype>>
  ::Value;

 //Member field identity
 //Note: Has optional field type and class type matching
 template<typename F, typename T = ImpossibleType<>, typename C = ImpossibleType<>>
 concept MemberField = MetaFunctions::MemberField<F>::Value
  //Match member field against field type
  && CX_CONDITIONAL_CONSTRAINT(
   (!SameType<T, ImpossibleType<>>),
   (SameType<T, typename MetaFunctions::MemberField<F>::FieldType>),
   true
  )
  //Match member field against class type
  && CX_CONDITIONAL_CONSTRAINT(
   (!SameType<C, ImpossibleType<>>),
   (SameType<C, typename MetaFunctions::MemberField<F>::Type>),
   true
  );

 #define CX_DEFINE_FUNCTION_IDENTITY_CONCEPT(name, funcName) \
 namespace MetaFunctions {\
  /*Matching member function with prototype idiom*/\
  template<typename S, typename Prototype>\
  requires (\
   CX::Struct<S>\
   && (CX::StaticFunction<Prototype> || CX::MemberFunction<Prototype>))\
  struct name : FalseType {};\
  \
  /*`<struct, static>` specialization*/\
  template<typename S, typename Prototype>\
  requires (CX::StaticFunction<Prototype>\
   && CX::Struct<S>\
   && requires (typename StaticFunction<Prototype>::template MemberFunctionPrototype<S> func) {\
    func = &CX::ConstDecayed<S>::funcName;\
   }\
  )\
  struct name<S, Prototype> : TrueType {};\
  \
  /*`<struct, member>` specialization*/\
  template<typename S, typename Prototype>\
  requires (CX::MemberFunction<Prototype>\
   && CX::Struct<S>\
   && requires (Prototype func) {\
    func = &CX::ConstDecayed<S>::funcName;\
   }\
  )\
  struct name<S, Prototype> : TrueType {};\
 }\
 \
 /*Concept definition*/\
 template<typename S, typename Prototype>\
 concept name = MetaFunctions\
  ::name<S, Prototype>\
  ::Value;

 #define CX_DEFINE_OPERATOR_CONCEPT(name, op) \
 CX_DEFINE_FUNCTION_IDENTITY_CONCEPT(name##Operator, operator op)

 //Binary operators
 CX_DEFINE_OPERATOR_CONCEPT(Addition, +);
 CX_DEFINE_OPERATOR_CONCEPT(Subtraction, -);
 CX_DEFINE_OPERATOR_CONCEPT(Division, /);
 CX_DEFINE_OPERATOR_CONCEPT(Multiplication, *);
 CX_DEFINE_OPERATOR_CONCEPT(Modulus, %);
 CX_DEFINE_OPERATOR_CONCEPT(BinaryAnd, &&);
 CX_DEFINE_OPERATOR_CONCEPT(BinaryOr, ||);
 CX_DEFINE_OPERATOR_CONCEPT(LeftShift, <<);
 CX_DEFINE_OPERATOR_CONCEPT(RightShift, >>);
 #define MORE_THAN >
 CX_DEFINE_OPERATOR_CONCEPT(GreaterThan, MORE_THAN);
 #undef MORE_THAN
 #define LESS_THAN <
 CX_DEFINE_OPERATOR_CONCEPT(LessThan, LESS_THAN);
 #undef LESS_THAN
 CX_DEFINE_OPERATOR_CONCEPT(Equality, ==);
 CX_DEFINE_OPERATOR_CONCEPT(Inequality, !=);
 CX_DEFINE_OPERATOR_CONCEPT(GreaterOrEqualThan, >=);
 CX_DEFINE_OPERATOR_CONCEPT(LessOrEqualThan, <=);
 CX_DEFINE_OPERATOR_CONCEPT(Assignment, =);
 CX_DEFINE_OPERATOR_CONCEPT(AdditionAssignment, +=);
 CX_DEFINE_OPERATOR_CONCEPT(SubtractionAssignment, -=);
 CX_DEFINE_OPERATOR_CONCEPT(MultiplicationAssignment, *=);
 CX_DEFINE_OPERATOR_CONCEPT(DivisionAssignment, /=);
 CX_DEFINE_OPERATOR_CONCEPT(ModulusAssignment, %=);
 CX_DEFINE_OPERATOR_CONCEPT(OrAssignment, |=);
 CX_DEFINE_OPERATOR_CONCEPT(AndAssignment, &=);
 CX_DEFINE_OPERATOR_CONCEPT(XORAssignment, ^=);
 CX_DEFINE_OPERATOR_CONCEPT(LeftShiftAssignment, <<=);
 CX_DEFINE_OPERATOR_CONCEPT(RightShiftAssignment, >>=);
 #define COMMA ,
 CX_DEFINE_FUNCTION_IDENTITY_CONCEPT(CommaOperator, operator COMMA);
 #undef COMMA

 //Special operators
 CX_DEFINE_OPERATOR_CONCEPT(ThreeWayComparison, <=>);
 CX_DEFINE_OPERATOR_CONCEPT(MemberAccess, ->);
 CX_DEFINE_OPERATOR_CONCEPT(MemberPointerAccess, ->*);
 CX_DEFINE_OPERATOR_CONCEPT(Subscript, []);
 CX_DEFINE_OPERATOR_CONCEPT(Function, ());

 //Unary operators
 template<typename F, typename Prototype>
 concept DereferenceOperator = MultiplicationOperator<F, Prototype>;
 CX_DEFINE_OPERATOR_CONCEPT(Compliment, ~);
 CX_DEFINE_OPERATOR_CONCEPT(Increment, ++);
 CX_DEFINE_OPERATOR_CONCEPT(Decrement, --);
 CX_DEFINE_OPERATOR_CONCEPT(Not, !);
 CX_DEFINE_OPERATOR_CONCEPT(Address, &);

 //Clean up internal macro
 #undef CX_DEFINE_OPERATOR_CONCEPT

 //Conversion operator
 //Notes:
 // 1. There is no way to explicitly specify template arguments for the conversion operator
 // 2. The conversion function does not accept arguments so there is no need to check for overloads
 template<typename T, typename C>
 concept ConversionOperator =
  /*Member non-overloaded operator*/
  requires { expect(&T::operator C); }
  /*Member overloaded operator (C as implicit template parameter + return type)*/
  || requires (T t) {
   { t.operator C() } -> SameType<C>;
  };
}
