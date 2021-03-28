#pragma once

#include <cx/idioms.h>

namespace CX {
 namespace MetaFunctions {
  //Produce max value from pack of non-type template parameters
  template<auto... Values>
  struct MaxValue;

  template<>
  struct MaxValue<> {
   static constexpr auto const Value = 0;
  };

  template<auto V>
  struct MaxValue<V> {
   static constexpr auto const Value = V;
  };

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

  template<>
  struct MinValue<> {
   static constexpr auto const Value = 0;
  };

  template<auto V>
  struct MinValue<V> {
   static constexpr auto const Value = V;
  };

  template<auto V1, auto V2>
  requires (requires { { V1 < V2 } -> CX::SameType<bool>; })
  struct MinValue<V1, V2> {
   static constexpr auto const Value = V1 < V2 ? V1 : V2;
  };

  template<auto V1, auto V2, auto... Values>
  struct MinValue<V1, V2, Values...> {
   static constexpr auto const Value = MinValue<
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

  template<typename Match, typename... Types>
  struct IndexOfType {
   template<typename...>
   struct Matcher;
  };

  template<typename Match, typename... Types>
  template<typename... Remaining>
  requires (sizeof...(Remaining) == 0)
  struct IndexOfType<Match, Types...>::Matcher<Remaining...> {
   static constexpr long const Value = -1;
  };

  template<typename Match, typename... Args>
  template<typename T>
  struct IndexOfType<Match, Args...>::Matcher<T> {
   static constexpr long const Value = CX::SameType<Match, T>
    ? sizeof...(Args) - 1
    : Matcher<>::Value;
  };

  template<typename Match, typename... Args>
  template<typename T, typename... Remaining>
  struct IndexOfType<Match, Args...>::Matcher<T, Remaining...> {
   static constexpr long const Value = CX::SameType<Match, T>
    ? (sizeof...(Args) - sizeof...(Remaining)) - 1
    : Matcher<Remaining...>::Value;
  };

  template<long Index, typename... Types>
  struct TypeAtIndex;

  template<long Index, typename T, typename... Types>
  requires (Index > 0)
  struct TypeAtIndex<Index, T, Types...> {
   using Type = typename TypeAtIndex<Index - 1, Types...>::Type;
  };

  template<typename T, typename... Types>
  struct TypeAtIndex<0, T, Types...> {
   using Type = T;
  };

  template<long Index, typename... Types>
  requires (Index < 0 || Index >= sizeof...(Types))
  struct TypeAtIndex<Index, Types...> {
   using Type = ImpossibleType<>;
  };

  template<template<typename...> typename Match, template<typename...> typename... Types>
  struct IndexOfTemplateType {
   template<template<typename...> typename...>
   struct Matcher;
  };

  template<template<typename...> typename Match, template<typename...> typename... Types>
  template<template<typename...> typename... Remaining>
  requires (sizeof...(Remaining) == 0)
  struct IndexOfTemplateType<Match, Types...>::Matcher<Remaining...> {
   static constexpr int const Value = -1;
  };

  template<template<typename...> typename Match, template<typename...> typename... Types>
  template<template<typename...> typename T>
  struct IndexOfTemplateType<Match, Types...>::Matcher<T> {
   static constexpr auto const Value = CX::SameTemplateType<Match, T>
    ? sizeof...(Types) - 1
    : Matcher<>::Value;
  };

  template<template<typename...> typename Match, template<typename...> typename... Types>
  template<template<typename...> typename T, template<typename...> typename... Remaining>
  struct IndexOfTemplateType<Match, Types...>::Matcher<T, Remaining...> {
   static constexpr auto const Value = CX::SameTemplateType<Match, T>
    ? (sizeof...(Types) - sizeof...(Remaining)) - 1
    : Matcher<Remaining...>::Value;
  };

  template<
   long Index,
   template<template<typename...> typename> typename Receiver,
   template<typename...> typename... Types
  >
  struct TemplateTypeAtIndex;

  template<
   long Index,
   template<template<typename...> typename> typename Receiver,
   template<typename...> typename T, template<typename...> typename... Types
  >
  struct TemplateTypeAtIndex<Index, Receiver, T, Types...> {
   using Type = typename TemplateTypeAtIndex<Index - 1, Receiver, Types...>::Type;
  };

  template<
   template<template<typename...> typename> typename Receiver,
   template<typename...> typename T,
   template<typename...> typename... Types
  >
  struct TemplateTypeAtIndex<0, Receiver, T, Types...> {
   using Type = Receiver<T>;
  };

  template<
   long Index,
   template<template<typename...> typename> typename Receiver,
   template<typename...> typename... Types
  >
  requires (Index < 0 || Index >= sizeof...(Types))
  struct TemplateTypeAtIndex<Index, Receiver, Types...> {
   using Type = Receiver<ImpossibleType>;
  };

  template<auto Match, auto... Values>
  struct IndexOfValue {
   template<auto...>
   struct Matcher;
  };

  template<auto Match, auto... Values>
  template<auto... Remaining>
  requires (sizeof...(Remaining) == 0)
  struct IndexOfValue<Match, Values...>::Matcher<Remaining...> {
   static constexpr int const Value = -1;
  };

  template<auto Match, auto... Values>
  template<auto V>
  struct IndexOfValue<Match, Values...>::Matcher<V> {
   static constexpr auto const Value = CX::SameValue<Match, V>
    ? sizeof...(Values) - 1
    : Matcher<>::Value;
  };

  template<auto Match, auto... Values>
  template<auto V, auto... Remaining>
  struct IndexOfValue<Match, Values...>::Matcher<V, Remaining...> {
   static constexpr auto const Value = CX::SameValue<Match, V>
    ? (sizeof...(Values) - sizeof...(Remaining)) - 1
    : Matcher<Remaining...>::Value;
  };

  template<long Index, auto... Values>
  struct ValueAtIndex;

  template<auto V, auto... Values>
  struct ValueAtIndex<0, V, Values...> {
   static constexpr auto const Value = V;
  };

  template<auto Index, auto... Values>
  requires (Index < 0 || Index >= sizeof...(Values))
  struct ValueAtIndex<Index, Values...> {
   static constexpr auto const Value = 0;
  };
 }

 template<auto... Values>
 constexpr auto const MaxValue = MetaFunctions
  ::MaxValue<Values...>
  ::Value;

 template<auto... Values>
 constexpr auto const MinValue = MetaFunctions
  ::MinValue<Values...>
  ::Value;

 template<typename... Types>
 constexpr auto const MaxTypeSize = MetaFunctions::TypeSize<MetaFunctions::MaxValue, Types...>;

 template<typename... Types>
 constexpr auto const MinTypeSize = MetaFunctions::TypeSize<MetaFunctions::MinValue, Types...>;

 template<typename Match, typename... Args>
 constexpr auto const IndexOfType = MetaFunctions
  ::IndexOfType<Match, Args...>
  ::template Matcher<Args...>
  ::Value;

 template<decltype(IndexOfType<void, void>) Index, typename... Types>
 using TypeAtIndex = typename MetaFunctions
  ::TypeAtIndex<Index, Types...>
  ::Type;

 template<template<typename...> typename Match, template<typename...> typename... Args>
 constexpr auto const IndexOfTemplateType = MetaFunctions
  ::IndexOfTemplateType<Match, Args...>
  ::template Matcher<Args...>
  ::Value;

 template<
  decltype(IndexOfTemplateType<VoidT, VoidT>) Index,
  template<template<typename...> typename> typename Receiver,
  template<typename...> typename... Types
 >
 using TemplateTypeAtIndex = typename MetaFunctions
  ::TemplateTypeAtIndex<Index, Receiver, Types...>
  ::Type;

 template<auto Match, auto... Values>
 constexpr auto const IndexOfValue = MetaFunctions
  ::IndexOfValue<Match, Values...>
  ::template Matcher<Values...>
  ::Value;

 template<decltype(IndexOfValue<0, 0>) Index, auto... Values>
 constexpr auto const ValueAtIndex = MetaFunctions
  ::ValueAtIndex<Index, Values...>
  ::Value;

 //Runtime type, template type and template value iterators
 template<typename... Types>
 struct TypeIterator;

 template<>
 struct TypeIterator<> {
 private:
  template<typename...>
  friend struct TypeIterator;

  [[gnu::always_inline]]
  inline static constexpr void exec(auto&) noexcept {}

 public:
  [[gnu::always_inline]]
  inline static constexpr void run(auto) noexcept {}

  [[gnu::always_inline]]
  inline static constexpr void run(auto&) noexcept {}
 };

 template<typename T, typename... Types>
 struct TypeIterator<T, Types...> {
 private:
  template<typename...>
  friend struct TypeIterator;

  template<typename F>
  static constexpr bool const OpIsNoexcept = noexcept(
   declval<F>().template operator()<T>()
  );

  [[gnu::always_inline]]
  inline static constexpr void exec(auto &op) noexcept(OpIsNoexcept<decltype(op)>) {
   //If `op` is a bool producer, allow for conditional iteration
   if constexpr (SameType<bool, decltype(op.template operator()<T>())>) {
    auto const next = [&]<typename...>() constexpr {
     return op.template operator()<T>();
    }();
    if (next) {
     TypeIterator<Types...>::exec(op);
    }
   } else {
    op.template operator()<T>();
    TypeIterator<Types...>::exec(op);
   }
  }

 public:
  [[gnu::always_inline]]
  inline static constexpr void run(auto op) noexcept(OpIsNoexcept<decltype(op)>) {
   exec(op);
  }

  [[gnu::always_inline]]
  inline static constexpr void run(auto &op) noexcept(OpIsNoexcept<decltype(op)>) {
   exec(op);
  }
 };

 template<template<typename...> typename... Types>
 struct TemplateTypeIterator;

 template<>
 struct TemplateTypeIterator<> {
 private:
  template<template<typename...> typename...>
  friend struct TemplateTypeIterator;

  [[gnu::always_inline]]
  inline static constexpr void exec(auto&) noexcept {}

 public:
  [[gnu::always_inline]]
  inline static constexpr void run(auto) noexcept {}

  [[gnu::always_inline]]
  inline static constexpr void run(auto&) noexcept {}
 };

 template<template<typename...> typename T, template<typename...> typename... Types>
 struct TemplateTypeIterator<T, Types...> {
 private:
  template<template<typename...> typename...>
  friend struct TemplateTypeIterator;

  template<typename F>
  static constexpr bool const OpIsNoexcept = noexcept(
   declval<F>().template operator()<T>()
  );

  [[gnu::always_inline]]
  inline static constexpr void exec(auto &op) noexcept {
   //If `op` is a bool producer, allow for conditional iteration
   if constexpr(SameType<bool, decltype(op.template operator()<T>())>) {
    auto const next = [&]<typename...> {
     return op.template operator()<T>();
    }();
    if (next) {
     TemplateTypeIterator<Types...>::exec(op);
    }
   } else {
    op.template operator()<T>();
    TemplateTypeIterator<Types...>::exec(op);
   }
  }

 public:
  [[gnu::always_inline]]
  inline static constexpr void run(auto op) noexcept(OpIsNoexcept<decltype(op)>) {
   exec(op);
  }

  [[gnu::always_inline]]
  inline static constexpr void run(auto &op) noexcept(OpIsNoexcept<decltype(op)>) {
   exec(op);
  }
 };

 template<auto... Values>
 struct ValueIterator;

 template<>
 struct ValueIterator<> {
 private:
  template<auto...>
  friend struct ValueIterator;

  [[gnu::always_inline]]
  inline static constexpr void exec(auto&) noexcept {}

 public:
  [[gnu::always_inline]]
  inline static constexpr void run(auto) noexcept {}

  [[gnu::always_inline]]
  inline static constexpr void run(auto&) noexcept {}
 };

 template<auto V, auto... Values>
 struct ValueIterator<V, Values...> {
 private:
  template<auto...>
  friend struct ValueIterator;

  template<typename F>
  static constexpr bool const OpIsNoexcept = noexcept(
   declval<F>().template operator()<V>()
  );

  [[gnu::always_inline]]
  inline static constexpr void exec(auto op) noexcept {
   if constexpr (SameType<bool, decltype(op.template operator()<V>())>) {
    auto const next = [&]<typename...> {
     return op.template operator()<V>();
    }();
    if (next) {
     ValueIterator<Values...>::exec(op);
    }
   } else {
    op.template operator()<V>();
    ValueIterator<Values...>::exec(op);
   }
  }

 public:
  [[gnu::always_inline]]
  inline static constexpr void run(auto op) noexcept(OpIsNoexcept<decltype(op)>) {
   exec(op);
  }

  [[gnu::always_inline]]
  inline static constexpr void run(auto &op) noexcept(OpIsNoexcept<decltype(op)>) {
   exec(op);
  }
 };
}
