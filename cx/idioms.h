#pragma once

#include "cx/common.h"
#include "cx/templates.h"

#define COMMA ,

#define DEFINE_MEMBER_FUNCTION_DETECTOR(detector, func)\
template<typename = void, typename...>\
struct _##detector##MemberFunctionExists : CX::false_type {\
private:\
 class _Dummy;\
public:\
 using return_t = _Dummy;\
};\
\
template<typename T, typename R, typename... Args>\
struct _##detector##MemberFunctionExists<\
 CX::void_t<decltype(CX::declval<T>().func((CX::declval<Args>(), ...)))>,\
 T,\
 R,\
 Args...\
> : CX::true_type {\
public:\
 using return_t = decltype(CX::declval<T>().func((CX::declval<Args>(), ...)));\
};\
\
template<typename T, typename R>\
struct _##detector##MemberFunctionExists<\
 CX::void_t<decltype(CX::declval<T>().func())>,\
 T,\
 R\
> : CX::true_type {\
public:\
 using return_t = decltype(CX::declval<T>().func());\
};\
\
template<typename T, typename R, typename... Args>\
struct detector##MemberFunctionExists : CX::select_if_true<\
 _##detector##MemberFunctionExists<void, T, R, Args...>::value \
  && CX::IsSame<R, typename _##detector##MemberFunctionExists<void, T, R, Args...>::return_t>::value,\
 CX::true_type,\
 CX::false_type\
>::type {};\

//TODO three-way comparison operator
#define DEFINE_BINARY_OPERATOR_DETECTOR(detector, _operator_)\
template<typename = void, typename...>\
struct detector##OperatorExists : CX::false_type {};\
\
template<typename T>\
struct detector##OperatorExists<\
 CX::void_t<decltype((CX::declval<T>() _operator_ CX::declval<T>()))>,\
 T\
> : CX::true_type{};\
\
template<typename T1, typename T2>\
struct detector##OperatorExists<\
 CX::void_t<decltype((CX::declval<T1>() _operator_ CX::declval<T2>()))>,\
 T1,\
 T2\
> : CX::true_type {};

#define DEFINE_LHS_UNARY_OPERATOR_DETECTOR(detector, _operator_)\
template<typename = void, typename...>\
struct detector##OperatorExists : CX::false_type {};\
\
template<typename T>\
struct detector##OperatorExists<\
 CX::void_t<decltype(_operator_ CX::declval<T>())>,\
 T\
> : CX::true_type {};\

#define DEFINE_RHS_UNARY_OPERATOR_DETECTOR(detector, _operator_)\
template<typename = void, typename...>\
struct detector##OperatorExists : CX::false_type {};\
\
template<typename T>\
struct detector##OperatorExists<\
 CX::void_t<decltype(CX::declval<T>() _operator_)>,\
 T\
> : CX::true_type {};\

namespace CX {
 DEFINE_BINARY_OPERATOR_DETECTOR(Plus, +)
 DEFINE_BINARY_OPERATOR_DETECTOR(Minus, -)
 DEFINE_BINARY_OPERATOR_DETECTOR(Div, /)
 DEFINE_BINARY_OPERATOR_DETECTOR(Mult, *)
 DEFINE_BINARY_OPERATOR_DETECTOR(Mod, %)
 DEFINE_BINARY_OPERATOR_DETECTOR(And, &&)
 DEFINE_BINARY_OPERATOR_DETECTOR(Or, ||)
 DEFINE_BINARY_OPERATOR_DETECTOR(LShift, <<)
 DEFINE_BINARY_OPERATOR_DETECTOR(RShift, >>)
 DEFINE_BINARY_OPERATOR_DETECTOR(GThan, >)
 DEFINE_BINARY_OPERATOR_DETECTOR(LThan, <)
 DEFINE_BINARY_OPERATOR_DETECTOR(Equality, ==)
 DEFINE_BINARY_OPERATOR_DETECTOR(InEquality, !=)
 DEFINE_BINARY_OPERATOR_DETECTOR(GEthan, >=)
 DEFINE_BINARY_OPERATOR_DETECTOR(LEthan, <=)
 DEFINE_BINARY_OPERATOR_DETECTOR(Assign, =)
 DEFINE_BINARY_OPERATOR_DETECTOR(OrAssign, |=)
 DEFINE_BINARY_OPERATOR_DETECTOR(AndAssign, &=)
 DEFINE_BINARY_OPERATOR_DETECTOR(XORAssign, &=)
 DEFINE_BINARY_OPERATOR_DETECTOR(LShiftAssign, <<=)
 DEFINE_BINARY_OPERATOR_DETECTOR(RShiftAssign, >>=)
 DEFINE_BINARY_OPERATOR_DETECTOR(Comma, COMMA)

 DEFINE_LHS_UNARY_OPERATOR_DETECTOR(Compl, ~)
 DEFINE_LHS_UNARY_OPERATOR_DETECTOR(Incr, ++)
 DEFINE_LHS_UNARY_OPERATOR_DETECTOR(Decr, --)
 DEFINE_LHS_UNARY_OPERATOR_DETECTOR(Not, !)
 DEFINE_LHS_UNARY_OPERATOR_DETECTOR(AddrOf, &)
 DEFINE_LHS_UNARY_OPERATOR_DETECTOR(Deref, *)
 //TODO new, new[], delete, delete[] and following operators
// DEFINE_RHS_UNARY_OPERATOR_DETECTOR(Ptr, ->)
// DEFINE_RHS_UNARY_OPERATOR_DETECTOR(MemberAccess, ->*)

 template<typename = void, typename...>
 struct FunctionOperatorExists : CX::false_type {};

 template<typename T, typename... Args>
 struct FunctionOperatorExists<
  CX::void_t<decltype(CX::declval<T>()((CX::declval<Args>(), ...)))>,
  T,
  Args...
 > : CX::true_type {};

 template<typename T>
 struct FunctionOperatorExists<
  CX::void_t<decltype(CX::declval<T>()())>,
  T
 > : CX::true_type {};

 template<typename = void, typename...>
 struct SubscriptOperatorExists : CX::false_type {};

 template<typename T, typename E>
 struct SubscriptOperatorExists<
  CX::void_t<decltype(CX::declval<T>()[CX::declval<E>()])>,
  T,
  E
 > : CX::true_type {};

 template<typename = void, typename...>
 struct ConversionOperatorExists : false_type {};

 template<typename T, typename C>
 struct ConversionOperatorExists<
  CX::void_t<decltype((C)CX::declval<T>())>,
  T,
  C
 > : true_type {};

 template<typename T1, typename T2>
 struct IsSame : false_type {};

 template<typename T>
 struct IsSame<T, T> : true_type {};

 //Similar to IsSame<typename, typename>, but enables comparisons between partially specialized or unspecialized templates
 template<template<typename...> typename T1, template<typename...> typename T2>
 struct TemplateIsSame : false_type {};

 template<template<typename...> typename T>
 struct TemplateIsSame<T, T> : true_type {};

 template<typename T>
 struct IsPointer : false_type {};

 template<typename T>
 struct IsPointer<T*> : true_type {};

 template<typename T>
 struct IsLValueReference : false_type {};

 template<typename T>
 struct IsLValueReference<T&> : true_type {};

 template<typename T>
 struct IsRValueReference : false_type {};

 template<typename T>
 struct IsRValueReference<T&&> : true_type {};

 template<typename...>
 struct MatchAny;

 template<typename T1, typename T2, typename... TS>
 struct MatchAny<T1, T2, TS...> : select_if_true<
  IsSame<T1, T2>::value || MatchAny<T1, TS...>::value,
  true_type,
  false_type
 >::type {};

 template<typename T>
 struct MatchAny<T> : false_type {};

 namespace Internal {
  template<typename T1, typename T2, typename = void>
  struct _IsCastable : false_type {};

  template<typename T1, typename T2>
  struct _IsCastable<T1, T2, void_t<decltype((T1)declval<T2>())>> : true_type {};
}

template<typename T1, typename T2>
using IsCastable = Internal::_IsCastable<T1, T2, void>;

// //Detection idioms for constructor presence
// namespace Internal {
//  //Negative idiom specialization
//  template<typename Target, typename = void, typename... Args>
//  struct HasConstructor : false_type {};
//
//  //Positive idiom specialization
//  template<typename Target, typename... Args>
//  struct HasConstructor<Target, void_t<decltype(Target{(declval<Args>(), ...)})>, Args...> : true_type {};
//
//  //Positive idiom specialization for empty parameter pack
//  //This specialization must be explicitly defined since an empty parameter pack
//  //evaluates to `{void}` which would interfere with the actual signature that the
//  //user is testing for.
//  //
//  //Negative idiom for empty parameter pack selects the base specialization
//  template<typename Target>
//  struct HasConstructor<Target, void_t<decltype(Target{})>> : public true_type {};
// }
//
// //CX API
// template<typename Target, typename... Args>
// using HasConstructor = Internal::HasConstructor<Target, void, Args...>;

 template<typename Target, typename... Args>
 struct HasConstructor : select_if_true<__is_constructible(Target, Args...), true_type, false_type>::type {};

 template<typename>
 struct IsMemberFunction : false_type {};

 template<typename T, typename R, typename... Args>
 struct IsMemberFunction<R (T::*)(Args...)> : true_type {};

 template<typename T, typename R, typename... Args>
 struct IsMemberFunction<R (T::*)(Args...) const> : true_type {};

 template<typename>
 struct IsStaticFunction : false_type {};

 template<typename R, typename... Args>
 struct IsStaticFunction<R (*)(Args...)> : true_type {};

 template<typename R, typename... Args>
 struct IsStaticFunction<R (&)(Args...)> : true_type {};

 template<typename R, typename... Args>
 struct IsStaticFunction<R (Args...)> : true_type {};

 template<typename T>
 struct IsFunction : select_if_true<
  IsStaticFunction<T>::value || IsMemberFunction<T>::value,
  true_type,
  false_type
 >::type {};

 template<auto T, bool = IsMemberFunction<decltype(T)>::value, typename = void>
 struct IsVirtualFunction : true_type {};

 template<auto T>
 struct IsVirtualFunction<T, true, void_a<T == T>> : false_type {};

 //Specialization for static member functions and non-member functions
 template<auto T>
 struct IsVirtualFunction<T, false> : false_type {};

 template<typename>
 struct IsMemberField : false_type {};

 template<typename T, typename C>
 struct IsMemberField<T C::*> : true_type {};

 template<typename T>
 struct IsField : select_if_true<
  IsPointer<T>::value || IsMemberField<T>::value,
  true_type,
  false_type
 >::type {};

 template<auto, typename = void, typename...>
 struct DefinesFunction : false_type {};

 template<auto F, typename T, typename R, typename... Args>
 struct DefinesFunction<F, void_t<decltype((declval<T>.*F)((declval<Args>, ...)))>, T, R, Args...> : true_type {};
}
