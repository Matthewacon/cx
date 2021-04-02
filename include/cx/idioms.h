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

  //Const identity
  template<typename T>
  struct Const : FalseType {
   using Type = T;
   using ConstDecayed = T;
  };

  template<typename T>
  struct Const<T const> : TrueType {
   using Type = T const;
   using ConstDecayed = T;
  };

  template<typename T>
  struct Const<T const&> : TrueType {
   using Type = T const&;
   using ConstDecayed = T&;
  };

  template<typename T>
  struct Const<T const&&> : TrueType {
   using Type = T const&&;
   using ConstDecayed = T&&;
  };

  template<typename T>
  struct Volatile : FalseType {
   using Type = T;
   using VolatileDecayed = T;
  };

  template<typename T>
  struct Volatile<T volatile> : TrueType {
   using Type = T volatile;
   using VolatileDecayed = T;
  };

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

  //Sign identity and decay/promotion
  template<typename T>
  struct Sign {
   using SignedType = T;
   using UnsingedType = T;
   static constexpr auto const Signed = false;
   static constexpr auto const Unsigned = false;
  };

  #define DEFINE_SIGN_SPECIALIZATION(signedType, unsignedType) \
  template<>\
  struct Sign<signedType> {\
   using SignedType = signedType;\
   using UnsignedType = unsignedType;\
   static constexpr auto const Signed = true;\
   static constexpr auto const Unsigned = false;\
  };\
  \
  template<>\
  struct Sign<unsignedType> {\
   using SignedType = signedType;\
   using UnsignedType = unsignedType;\
   static constexpr auto const Signed = false;\
   static constexpr auto const Unsigned = true;\
  }

  DEFINE_SIGN_SPECIALIZATION(signed char,      unsigned char);
  DEFINE_SIGN_SPECIALIZATION(signed short,     unsigned short);
  DEFINE_SIGN_SPECIALIZATION(signed int,       unsigned int);
  DEFINE_SIGN_SPECIALIZATION(signed long,      unsigned long);
  DEFINE_SIGN_SPECIALIZATION(signed long long, unsigned long long);

  #undef DEFINE_SIGN_SPECIALIZATION

  //Function identity meta-functions
  //Member function idiom
  template<typename>
  struct MemberFunction : FalseType {
   using Type = ImpossibleType<>;
   using ReturnType = ImpossibleType<>;
   using ArgumentTypes = Dummy<>;
   static constexpr auto const Const = false;
   static constexpr auto const Noexcept = false;
   static constexpr auto const Variadic = false;
  };

  template<typename T, typename R, typename... Args>
  struct MemberFunction<R (T::*)(Args...)> : TrueType {
   using Type = T;
   using ReturnType = R;
   using ArgumentTypes = Dummy<Args...>;
   static constexpr auto const Const = false;
   static constexpr auto const Noexcept = false;
   static constexpr auto const Variadic = false;
  };

  template<typename T, typename R, typename... Args>
  struct MemberFunction<R (T::*)(Args...) const> : MemberFunction<R (T::*)(Args...)> {
   static constexpr auto const Const = true;
  };

  template<typename T, typename R, typename... Args>
  struct MemberFunction<R (T::*)(Args..., ...)> : MemberFunction<R (T::*)(Args...)> {
   static constexpr auto const Variadic = true;
  };

  template<typename T, typename R, typename... Args>
  struct MemberFunction<R (T::*)(Args..., ...) const> : MemberFunction<R (T::*)(Args...) const> {
   static constexpr auto const Variadic = true;
  };

  template<typename T, typename R, typename... Args>
  struct MemberFunction<R (T::*)(Args...) noexcept> : MemberFunction<R (T::*)(Args...)> {
   static constexpr auto const Noexcept = true;
  };

  template<typename T, typename R, typename... Args>
  struct MemberFunction<R (T::*)(Args..., ...) noexcept> : MemberFunction<R (T::*)(Args...) noexcept> {
   static constexpr auto const Variadic = true;
  };

  template<typename T, typename R, typename... Args>
  struct MemberFunction<R (T::*)(Args...) const noexcept> : MemberFunction<R (T::*)(Args...) noexcept> {
   static constexpr auto const Const = true;
  };

  template<typename T, typename R, typename... Args>
  struct MemberFunction<R (T::*)(Args..., ...) const noexcept> : MemberFunction<R (T::*)(Args...) const noexcept> {
   static constexpr auto const Variadic = true;
  };

  //Static function idiom
  template<typename>
  struct StaticFunction : FalseType {
   using ReturnType = ImpossibleType<>;
   using ArgumentTypes = Dummy<>;
   static constexpr auto const Noexcept = false;
   static constexpr auto const Variadic = false;
  };

  template<typename R, typename... Args>
  struct StaticFunction<R (Args...)> : TrueType {
   using ReturnType = R;
   using ArgumentTypes = Dummy<Args...>;
   static constexpr auto const Noexcept = false;
   static constexpr auto const Variadic = false;
  };

  template<typename R, typename... Args>
  struct StaticFunction<R (Args..., ...)> : StaticFunction<R (Args...)> {
   static constexpr auto const Variadic = true;
  };

  template<typename R, typename... Args>
  struct StaticFunction<R (Args...) noexcept> : StaticFunction<R (Args...)> {
   static constexpr auto const Noexcept = true;
  };

  template<typename R, typename... Args>
  struct StaticFunction<R (Args..., ...) noexcept> : StaticFunction<R (Args...) noexcept > {
   static constexpr auto const Variadic = true;
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
 template<typename T1, typename T2>
 concept SameType = MetaFunctions::SameType<T1, T2>::Value;

 template<typename... Types>
 concept MatchAnyType = MetaFunctions::MatchAnyType<Types...>::Value;

 template<template<typename...> typename T1, template<typename...> typename T2>
 concept SameTemplateType = MetaFunctions::SameTemplateType<T1, T2>::Value;

 template<template<typename...> typename... Types>
 concept MatchAnyTemplateType = MetaFunctions::MatchAnyTemplateType<Types...>::Value;

 template<auto V1, auto V2>
 concept SameValue = MetaFunctions::SameValue<V1, V2>::Value;

 template<auto... Values>
 concept MatchAnyValue = MetaFunctions::MatchAnyValue<Values...>::Value;

 template<typename... Args>
 concept UniqueTypes = MetaFunctions::UniqueTypes<Args...>::Value;

 template<template<typename...> typename... Args>
 concept UniqueTemplateTypes = MetaFunctions::UniqueTemplateTypes<Args...>::Value;

 template<auto... Values>
 concept UniqueValues = MetaFunctions::UniqueValues<Values...>::Value;

 template<typename T1, typename T2>
 concept ConvertibleTo = requires (T1 t1) {
  { (T2)t1 } -> SameType<T2>;
 };

 template<typename T, typename... Args>
 concept Constructible = __is_constructible(T, Args...);

 template<typename T>
 concept Destructible = requires (T t) {
  { t.~T() } -> SameType<void>;
 };

 template<typename T>
 concept CopyConstructible = __is_constructible(T, T const&)
  && requires (T * t1, T t2) {
   { new (t1) T {(T const&)t2} } -> SameType<T *>;
  };

 template<typename T>
 concept MoveConstructible = __is_constructible(T, T&&)
  && requires (T * t1, T t2) {
   { new (t1) T {(T&&)t2} } -> SameType<T *>;
  };

 template<typename T>
 concept CopyAssignable =
  requires (T t1, T t2) {
   { t1.operator=((T const&)t2) } -> SameType<T&>;
  }
  || requires (T t1, T t2) {
   { t1 = (T&)t2 } -> SameType<T&>;
  };

 template<typename T>
 concept MoveAssignable =
  requires (T t1, T t2) {
   { t1.operator=((T&&)t2) } -> SameType<T&>;
  }
  || requires (T t1, T t2) {
   { t1 = (T&&)t2 } -> SameType<T&>;
  };

 template<typename T>
 using Unqualified = typename MetaFunctions
  ::Unqualified<T>
  ::Type;

 template<typename T>
 using Decayed = typename MetaFunctions
  ::Unqualified<T>
  ::DecayedType;

 template<typename T>
 concept Const = MetaFunctions
  ::Const<T>
  ::Value;

 template<typename T>
 using ConstDecayed = typename MetaFunctions
  ::Const<T>
  ::ConstDecayed;

 template<typename T>
 concept Volatile = MetaFunctions
  ::Volatile<T>
  ::Value;

 template<typename T>
 using VolatileDecayed = typename MetaFunctions
  ::Volatile<T>
  ::VolatileDecayed;

 template<typename T>
 concept Array = MetaFunctions
  ::Array<T>
  ::Value;

 template<typename T>
 concept SizedArray = MetaFunctions
   ::Array<T>
   ::Sized;

 template<typename T>
 concept UnsizedArray = Array<T>
  && !MetaFunctions
   ::Array<T>
   ::Sized;

 template<typename T>
 using ArrayElementType = typename MetaFunctions
  ::Array<T>
  ::ElementType;

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

 //Signed identity
 template<typename T>
 concept Signed = MetaFunctions
  ::Sign<T>
  ::Signed;

 //Unsigned identity
 template<typename T>
 concept Unsigned = !MetaFunctions
  ::Sign<T>
  ::Unsigned;

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

 //Enum type identity
 template<typename T>
 concept Enum = __is_enum(T);

 //Union type identity
 template<typename T>
 concept Union = __is_union(T);

 //Integral type identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_integral
 template<typename T>
 concept Integral = MatchAnyType<
  SignDecayed<ConstDecayed<T>>,
  bool,
  char,
  wchar_t,
  char8_t,
  char16_t,
  char32_t,
  short,
  int,
  long,
  long long
 >;

 //Floating precision type identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_floating_point
 template<typename T>
 concept Floating = MatchAnyType<
  ConstDecayed<T>,
  float,
  double,
  long double
 >;

 //Arithmetic type identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_arithmetic
 template<typename T>
 concept Arithmetic = Integral<T>
  || Floating<T>;

 //Scalar type identity
 //Defined by: https://en.cppreference.com/w/cpp/types/is_scalar
 template<typename T>
 concept Scalar = Arithmetic<T>
  || Enum<T>
  || Pointer<T>
  || MemberPointer<T>;

 //Trivially copyable type identity
 //Defined by: https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable
 template<typename T>
 concept TriviallyCopyable = Constructible<ArrayElementType<T>>
  && Destructible<ArrayElementType<T>>
  && CopyConstructible<ArrayElementType<T>>
  && MoveConstructible<ArrayElementType<T>>
  && CopyAssignable<ArrayElementType<T>>
  && MoveAssignable<ArrayElementType<T>>;

 //Trivial type identity
 //Defined by: https://en.cppreference.com/w/cpp/named_req/TrivialType
 template<typename T>
 concept Trivial = Scalar<T>
  || TriviallyCopyable<T>;

 //Function identity concepts
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
   (SameType<Dummy<Args...>, typename MetaFunctions::MemberFunction<Unqualified<F>>::ArgumentTypes>),
   true
  );

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
   (SameType<Dummy<Args...>, typename MetaFunctions::StaticFunction<Unqualified<F>>::ArgumentTypes>),
   true
  );

 template<auto F>
 concept VirtualFunction = MetaFunctions::VirtualFunction<F>::Value;

 template<typename T>
 concept MemberField = MetaFunctions::MemberField<T>::Value;

 //Operator detection concepts
 #define DEFINE_OPERATOR_CONCEPT(name, op) \
 template<typename T, typename R = ImpossibleType<>, typename... Args>\
 concept name##Operator = \
  /*Member or static member non-overloaded operator*/\
  requires { expect(&T::operator op); }\
  /*Member or static member overloaded operator, match against return type and arguments*/\
  || (!SameType<R, ImpossibleType<>> && requires (T t, Args... args) {\
   { t.operator op(args...) } -> SameType<R>;\
  })\
  /*Member overloaded operator (R as template parameter + return type, Args as template parameters and function arguments)*/\
  || (!SameType<R, ImpossibleType<>> && requires (T t, Args... args) {\
   { t.template operator op<R, Args...>(args...) } -> SameType<R>;\
  })\
  /*Member overloaded operator (R + Args as template parameters and function arguments)*/\
  || (!SameType<R, ImpossibleType<>> && requires (T t, R r, Args... args) {\
   t.template operator op<R, Args...>(r, args...);\
  })\
  /*Member overloaded operator (R + Args as template parameters, R as the return type)*/\
  || (!SameType<R, ImpossibleType<>> && requires (T t, R (T::* fptr)()) {\
   fptr = &T::template operator op<R, Args...>;\
  })\
  || (!SameType<R, ImpossibleType<>> && requires (T t, R (T::* fptr)() const) {\
   fptr = &T::template operator op<R, Args...>;\
  })\
  /*Member overloaded operator (R + Args as template parameters)*/\
  || (!SameType<R, ImpossibleType<>> && requires (T t) {\
   t.template operator op<R, Args...>();\
  })

 //Binary operators
 DEFINE_OPERATOR_CONCEPT(Addition, +);
 DEFINE_OPERATOR_CONCEPT(Subtraction, -);
 DEFINE_OPERATOR_CONCEPT(Division, /);
 DEFINE_OPERATOR_CONCEPT(Multiplication, *);
 DEFINE_OPERATOR_CONCEPT(Modulus, %);
 DEFINE_OPERATOR_CONCEPT(BinaryAnd, &&);
 DEFINE_OPERATOR_CONCEPT(BinaryOr, ||);
 DEFINE_OPERATOR_CONCEPT(LeftShift, <<);
 DEFINE_OPERATOR_CONCEPT(RightShift, >>);
 #define MORE_THAN >
 DEFINE_OPERATOR_CONCEPT(GreaterThan, MORE_THAN);
 #undef MORE_THAN
 #define LESS_THAN <
 DEFINE_OPERATOR_CONCEPT(LessThan, LESS_THAN);
 #undef LESS_THAN
 DEFINE_OPERATOR_CONCEPT(Equality, ==);
 DEFINE_OPERATOR_CONCEPT(Inequality, !=);
 DEFINE_OPERATOR_CONCEPT(GreaterOrEqualThan, >=);
 DEFINE_OPERATOR_CONCEPT(LessOrEqualThan, <=);
 //Add default cases for the l/r-value assignment operators since they are
 //overloads of any user defined assignment operator
 DEFINE_OPERATOR_CONCEPT(Assignment, =)
  || (SameType<R, ImpossibleType<>> && (
   requires { expect<T& (T::*)(T const&)>(&T::operator=); }
   || requires { expect<T& (T::*)(T&&)>(&T::operator=); }
  ));
 DEFINE_OPERATOR_CONCEPT(AdditionAssignment, +=);
 DEFINE_OPERATOR_CONCEPT(SubtractionAssignment, -=);
 DEFINE_OPERATOR_CONCEPT(MultiplicationAssignment, *=);
 DEFINE_OPERATOR_CONCEPT(DivisionAssignment, /=);
 DEFINE_OPERATOR_CONCEPT(ModulusAssignment, %=);
 DEFINE_OPERATOR_CONCEPT(OrAssignment, |=);
 DEFINE_OPERATOR_CONCEPT(AndAssignment, &=);
 DEFINE_OPERATOR_CONCEPT(XORAssignment, ^=);
 DEFINE_OPERATOR_CONCEPT(LeftShiftAssignment, <<=);
 DEFINE_OPERATOR_CONCEPT(RightShiftAssignment, >>=);
 #define COMMA ,
 DEFINE_OPERATOR_CONCEPT(Comma, COMMA);
 #undef COMMA

 //Special operators
 DEFINE_OPERATOR_CONCEPT(ThreeWayComparison, <=>);
 DEFINE_OPERATOR_CONCEPT(MemberAccess, ->);
 DEFINE_OPERATOR_CONCEPT(MemberPointerAccess, ->*);
 DEFINE_OPERATOR_CONCEPT(Subscript, []);
 DEFINE_OPERATOR_CONCEPT(Function, ());

 //Unary operators
 template<typename T, typename R = ImpossibleType<>, typename... Args>
 concept DereferenceOperator = MultiplicationOperator<T, R, Args...>;
 DEFINE_OPERATOR_CONCEPT(Compliment, ~);
 DEFINE_OPERATOR_CONCEPT(Increment, ++);
 DEFINE_OPERATOR_CONCEPT(Decrement, --);
 DEFINE_OPERATOR_CONCEPT(Not, !);
 DEFINE_OPERATOR_CONCEPT(Address, &);

 //Clean up internal macro
 #undef DEFINE_OPERATOR_CONCEPT

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

 template<typename F, typename R = ImpossibleType<>, typename... Args>
 concept Function = MemberFunction<F, R, Args...>
  || StaticFunction<F, R, Args...>
  || FunctionOperator<F, R, Args...>;
}
