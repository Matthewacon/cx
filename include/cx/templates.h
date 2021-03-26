#pragma once
 
#include <cx/idioms.h>

namespace CX {
 namespace MetaFunctions {
  //Produce max value from pack of non-type template parameters
  template<auto... Values>
  struct MaxValue;

  template<auto V1, auto V2>
  requires (requires { { V1 > V2 } -> CX::SameType<bool>; })
  struct MaxValue<V1, V2> {
   static constexpr auto const Value = V1 > V2 ? V1 : V2;
  };

  template<auto V1, auto V2, auto... Values>
  struct MaxValue<V1, V2, Values...> {
   static constexpr auto const Value = MaxValue<
    MaxValue<V1, V2>::Value,
    Values...
   >::Value;
  };

  //Produce min value from pack of non-type template parameters
  template<auto...>
  struct MinValue;

  template<auto V1, auto V2>
  requires (requires { { V1 < V2 } -> CX::SameType<bool>; })
  struct MinValue<V1, V2> {
   static constexpr auto const Value = V1 < V2 ? V1 : V2;
  };

  template<auto V1, auto V2, auto... Values>
  struct MinValue<V1, V2, Values...> {
   static constexpr auto const Vlaue = MinValue<
    MinValue<V1, V2>::Value,
    Values...
   >::Value;
  };

  //Utilites for `MaxTypeSize` and `MinTypeSize`
  namespace Internal {
   template<template<auto...> typename Producer, typename... Types>
   struct TypeSizeCommon;

   template<template<auto...> typename Producer, typename T, typename... Types>
   struct TypeSizeCommon<Producer, T, Types...> {
    template<auto... Values>
    struct Collector {
     static constexpr auto const Value = TypeSizeCommon<Producer, Types...>
      ::template Collector<Values..., sizeof(T)>
      ::Value;
    };

    static constexpr auto const Value = Collector<>::Value;
   };

   template<template<auto...> typename Producer, typename T>
   struct TypeSizeCommon<Producer, T> {
    template<auto... Values>
    struct Collector {
     //Add `0` to `MaxValue` parameter list to cover single type argument case
     static constexpr auto const Value = MaxValue<Values..., 0, sizeof(T)>::Value;
    };
   };

   template<template<auto...> typename Producer>
   struct TypeSizeCommon<Producer> {
    static constexpr auto const Value = 0;
   };
  }

  template<template<auto...> typename Producer, typename... Types>
  constexpr auto const TypeSize = Internal
   ::TypeSizeCommon<Producer, Types...>
   ::Value;

  template<typename Match, typename... Args>
  struct IndexOfType {
   template<typename...>
   struct Matcher;
   
   static constexpr auto const Value = Matcher<Args...>::Value;
  };

  template<typename Match, typename... Args>
  template<typename T, typename... Remaining>
  struct IndexOfType<Match, Args...>::Matcher<T, Remaining...> {
   static constexpr auto const Value = CX::SameType<Match, T>
    ? (sizeof...(Args) - sizeof...(Remaining)) - 1
    : Matcher<Remaining...>::Value;
  };

  template<typename Match, typename... Args>
  template<typename T>
  struct IndexOfType<Match, Args...>::Matcher<T> {
   static constexpr auto const Value = CX::SameType<Match, T> 
    ? sizeof...(Args) - 1
    : -1;
  };

  template<template<typename...> typename Match, template<typename...> typename... Args>
  struct IndexOfTemplateType {
   template<template<typename...> typename...>
   struct Matcher;

   static constexpr auto const Value = Matcher<Args...>::Value; 
  };

  template<template<typename...> typename Match, template<typename...> typename... Args>
  template<template<typename...> typename T, template<typename...> typename... Remaining>
  struct IndexOfTemplateType<Match, Args...>::Matcher<T, Remaining...> {
   static constexpr auto const Value = CX::SameTemplateType<Match, T>
    ? (sizeof...(Args) - sizeof...(Remaining)) - 1
    : Matcher<Remaining...>::Value;
  };

  template<template<typename...> typename Match, template<typename...> typename... Args>
  template<template<typename...> typename T>
  struct IndexOfTemplateType<Match, Args...>::Matcher<T> {
   static constexpr auto const Value = CX::SameTemplateType<Match, T> 
    ? sizeof...(Args) - 1
    : -1;
  }; 

  template<auto Match, auto... Args>
  struct IndexOfValue {
   template<auto...>
   struct Matcher;

   static constexpr auto const Value = Matcher<Args...>::Value;
  };

  template<auto Match, auto... Args>
  template<auto V, auto... Remaining>
  struct IndexOfValue<Match, Args...>::Matcher<V, Remaining...> {
   static constexpr auto const Value = CX::SameValue<Match, V>
    ? (sizeof...(Args) - sizeof...(Remaining)) - 1
    : Matcher<Remaining...>::Value;
  };

  template<auto Match, auto... Args>
  template<auto V>
  struct IndexOfValue<Match, Args...>::Matcher<V> {
   static constexpr auto const Value = CX::SameValue<Match, V>
    ? sizeof...(Args) - 1
    : -1;
  };
 }

 template<auto... Values>
 constexpr auto const MaxValue = MetaFunctions::MaxValue<Values...>::Value;

 template<auto... Values>
 constexpr auto const MinValue = MetaFunctions::MinValue<Values...>::Value;

 template<typename... Types>
 constexpr auto const MaxTypeSize = MetaFunctions::TypeSize<MetaFunctions::MaxValue, Types...>;

 template<typename... Types>
 constexpr auto const MinTypeSize = MetaFunctions::TypeSize<MetaFunctions::MinValue, Types...>;

 template<typename Match, typename... Args>
 constexpr auto const IndexOfType = MetaFunctions::IndexOfType<Match, Args...>::Value;

 //TODO
 //template<decltype(IndexOfType<void, void>) Index, typename... Types>
 //using TypeAtIndex = ImpossibleType<>/*MetaFunctions::TypeAtIndex<Index, Types...>::Type*/;

 template<template<typename...> typename Match, template<typename...> typename... Args>
 constexpr auto const IndexOfTemplateType = MetaFunctions::IndexOfTemplateType<Match, Args...>::Value;

 //TODO
 //template<decltype(IndexOfTemplateType<VoidT, VoidT>) Index, template<typename...> typename... Types>
 //using TemplateTypeAtIndex = ImpossibleType<>/*MetaFunctions::TemplateTypeAtIndex<Index, Types...>::Type*/;

 template<auto Match, auto... Values>
 constexpr auto const IndexOfValue = MetaFunctions::IndexOfValue<Match, Values...>::Value;

 //TODO
 //template<decltype(IndexOfValue<0, 0>) Index, auto... Values>
 //constexpr auto const ValueAtIndex = 0/*MetaFunctions::ValueAtIndex<Index, Values...>::Value*/;
}
