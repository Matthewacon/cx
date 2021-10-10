#pragma once

#include <cx/common.h>
#include <cx/idioms.h>
#include <cx/templates.h>

//Temporarily disable exception keyword shadowing to avoid breaking STL/libc
//headers
#ifndef CX_NO_BELLIGERENT_ERRORS
 #undef throw
 #undef try
 #undef catch
 #undef finally
#endif

//Dependencies for structured binding declaration support
#ifdef CX_STL_SUPPORT
 //Use template definitions from STL
 #include <utility>
#endif

//Re-enable exception keyword shadowing
#ifndef CX_NO_BELLIGERENT_ERRORS
 //Disable clang warnings about macros shadowing keywords
 #ifdef CX_COMPILER_CLANG_LIKE
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wkeyword-macro"
 #endif

 //Re-define macros to shadow keywords related to exception handling
 #define throw CX_ERROR_EXCEPTIONS_ARE_BAD
 #define try CX_ERROR_EXCEPTIONS_ARE_BAD
 #define catch CX_ERROR_EXCEPTIONS_ARE_BAD
 #define finally CX_ERROR_EXCEPTIONS_ARE_BAD

 //Pop diagnostic context
 #ifdef CX_COMPILER_CLANG_LIKE
  #pragma GCC diagnostic pop
 #endif
#endif

//Declare base templates required for structured binding declaration support
#ifndef CX_STL_SUPPORT
 //Declare base templates to allow for specializations
 namespace std {
  //Value meta-function that yields the number of elements
  //in a tuple-like type
  template<typename MaybeTuple>
  struct tuple_size;

  //Type meta-function that yields the `I`th type of a
  //tuple-like's type parameters
  template<CX::SizeType I, typename MaybeTuple>
  struct tuple_element;
 }
#endif

namespace CX {
 //Forward declare `CX::Tuple` for use with tuple meta-functions
 template<typename...>
 struct Tuple;

 //Supporting tuple meta-functions and concepts
 namespace TupleMetaFunctions {
  //Tuple identity and size meta-function
  template<typename T, typename = Unqualified<T>>
  struct IsTuple : FalseType {};

  template<
   typename T,
   template<typename...> typename MaybeTuple,
   typename... Types
  >
  requires (SameTemplateType<Tuple, MaybeTuple>)
  struct IsTuple<T, MaybeTuple<Types...>> : TrueType {
   static constexpr auto const Size = sizeof...(Types);
  };
 }

 //Tuple identity concept
 template<typename T>
 concept IsTuple = TupleMetaFunctions
  ::IsTuple<T>
  ::Value;

 //Tuple size value meta-function
 template<typename T>
 requires (IsTuple<T>)
 constexpr auto const TupleSize = TupleMetaFunctions
  ::IsTuple<T>
  ::Size;

 //Supporting tuple meta-functions and concepts
 namespace TupleMetaFunctions {
  //Tuple element iterator
  template<CX::IsTuple T, SizeType Index = CX::TupleSize<T>>
  struct TupleElementIterator;

  template<CX::IsTuple T, SizeType Index>
  struct TupleElementIterator {
   template<CX::IsTuple, SizeType>
   friend struct TupleElementIterator;

  private:
   //TODO noexcept qualification
   template<typename... Args>
   static constexpr auto iterImpl(T const& tuple, auto& op, Args&... args) /*noexcept()*/ {
    return TupleElementIterator<T, Index - 1>::iterImpl(tuple, op, args..., tuple.template get<Index - 1>());
   }

  public:
   //Shim to `iterImpl`
   //TODO noexcept qualification
   static constexpr auto iter(T const& tuple, auto op) /*noexcept()*/ {
    return iterImpl(tuple, op);
   }
  };

  template<CX::IsTuple T>
  struct TupleElementIterator<T, 0> {
   template<CX::IsTuple, SizeType>
   friend struct TupleElementIterator;

  private:
   //TODO noexcept qualification
   template<typename... Args>
   static constexpr auto iterImpl(T const&, auto& op, Args&... args) {
    return op.template operator()<Args&...>(args...);
   }

  public:
   //TODO noexcept qualification
   static constexpr auto iter(T const& tuple, auto op) /*noexcept(...)*/ {
    return iterImpl(tuple, op);
   }
  };

  //Tuple concatenation utilities
  template<
   typename T1,
   typename T2,
   typename U1 = Unqualified<T1>,
   typename U2 = Unqualified<T2>
  >
  requires (CX::IsTuple<U1> && CX::IsTuple<U2>)
  struct TuplePairOperations;

  template<
   typename T1,
   typename T2,
   template<typename...> typename TT1,
   template<typename...> typename TT2,
   typename... T1Args,
   typename... T2Args
  >
  requires (IsTuple<TT1<T1Args...>>::Value && IsTuple<TT2<T2Args...>>::Value)
  struct TuplePairOperations<T1, T2, TT1<T1Args...>, TT2<T2Args...>> {
   //TODO noexcept qualification
   static constexpr auto concatenate(T1 const& t1, T2 const& t2) /*noexcept(...)*/{
    return TupleElementIterator<T1>::iter(t1, [&]<typename...>(T1Args&... t1args) {
     return TupleElementIterator<T2>::iter(t2, [&]<typename...>(T2Args&... t2args) {
      return Tuple<T1Args..., T2Args...> {
       t1args...,
       t2args...
      };
     });
    });
   }
  };
 }
}

//Glue to enable use of structured binding declarations
namespace std {
 template<typename T>
 requires (CX::IsTuple<T>)
 struct tuple_size<T> {
  static constexpr auto const value = CX::TupleSize<T>;
 };

 template<CX::SizeType I, typename T>
 requires (CX::IsTuple<T>)
 struct tuple_element<I, T> {
 private:
  template<typename... Types>
  struct TupleParameterReceiver {
   using Type = CX::TypeAtIndex<I, Types...>;
  };

 public:
  using type = typename CX
   ::TypeParameterDeducer<TupleParameterReceiver, T>
   ::Type;
 };
}

//Define `CX::Tuple<...>` partial specializations
namespace CX {
 //Empty tuple partial specialization
 template<>
 struct Tuple<> final {
  template<typename...>
  friend struct Tuple;

  constexpr Tuple() noexcept = default;
  constexpr Tuple(Tuple const&) noexcept = default;
  constexpr Tuple(Tuple&&) noexcept = default;

  constexpr ~Tuple() noexcept = default;

  template<SizeType Index>
  requires (Index != Index)
  constexpr auto& get() noexcept {
   return 0;
  }

  template<SizeType Index>
  requires (Index != Index)
  constexpr auto&& rget() noexcept {
   return 0;
  }

  //TODO generic tuple iterator

  //Tuple concatenation operator
  template<IsTuple OtherTuple>
  constexpr auto operator+(OtherTuple const& other) const noexcept(noexcept(
   TupleMetaFunctions
    ::TuplePairOperations<Tuple, OtherTuple>
    ::concatenate(*this, other)
  )) {
   return TupleMetaFunctions
    ::TuplePairOperations<Tuple, OtherTuple>
    ::concatenate(*this, other);
  }

  //Tuple copy-assignment operator
  constexpr Tuple& operator=(Tuple const&) noexcept {
   //nop
   return *this;
  }

  //Tuple move-assignment operator
  constexpr Tuple& operator=(Tuple&&) noexcept {
   //nop
   return *this;
  }
 };

 //Populated tuple partial specialization `(1 <= sizeof...(Types))`
 template<typename T, typename... Types>
 struct Tuple<T, Types...> final {
  template<typename...>
  friend struct Tuple;

 private:
  template<SizeType Index>
  requires (Index <= sizeof...(Types))
  using TupleTypeAtIndex = TypeAtIndex<Index, T, Types...>;

  T t;
  Tuple<Types...> next;

 public:
  //TODO copy or move(preferentially)
  //Tuple element constructor
  constexpr Tuple(T t, Types... types) noexcept(
   noexcept(T{t}) && noexcept(decltype(next){types...})
  ) :
   t{t},
   next{types...}
  {}

  //Tuple copy-constructor
  constexpr Tuple(Tuple const& other)
   noexcept(noexcept(T{other.t}) && noexcept(decltype(next){other.next}))
   requires (CopyConstructible<T> && CopyConstructible<decltype(next)>)
  :
   t{other.t},
   next{other.next}
  {}

  //Tuple move-constructor
  constexpr Tuple(Tuple&& other)
   noexcept(noexcept(T{(T&&)other.t})
    && noexcept(decltype(next){(decltype(next)&&)other.next})
   )
   requires (MoveConstructible<T> && MoveConstructible<decltype(next)>)
  :
   t{(T&&)other.t},
   next{(decltype(next)&&)other.next}
  {}

  constexpr ~Tuple() noexcept(
   noexcept(t.~T()) && noexcept(next.~Tuple<Types...>())
  ) = default;

  //Returns l-value reference to tuple element at `Index`
  template<SizeType Index>
  constexpr auto& get() const noexcept {
   if constexpr (Index == 0) {
    return (TupleTypeAtIndex<Index>&)t;
   } else {
    return next.template get<Index - 1>();
   }
  }

  //Returns r-value reference to tuple element at `Index`
  template<SizeType Index>
  constexpr auto&& rget() const noexcept {
   if constexpr (Index == 0) {
    return (TupleTypeAtIndex<Index>&&)t;
   } else {
    return next.template rget<Index - 1>();
   }
  }

  //TODO generic lambda iterator

  //Tuple concatenation operator
  template<IsTuple OtherTuple>
  constexpr auto operator+(OtherTuple const& other) const noexcept(noexcept(
   TupleMetaFunctions
    ::TuplePairOperations<Tuple, OtherTuple>
    ::concatenate(*this, other)
  )) {
   return TupleMetaFunctions
    ::TuplePairOperations<Tuple, OtherTuple>
    ::concatenate(*this, other);
  }

  //Tuple copy-assignment operator
  constexpr Tuple& operator=(Tuple const& other)
   noexcept(noexcept(t = other.t) && noexcept(next = other.next))
   requires (CopyAssignable<T> && CopyAssignable<decltype(next)>)
  {
   t = other.t;
   next = other.next;
   return *this;
  }

  //Tuple move-assignment operator
  constexpr Tuple& operator=(Tuple&& other)
   noexcept(noexcept(t = (T&&)other.t)
    && noexcept(next = (decltype(next)&&)other.next)
   )
   requires (MoveAssignable<T> && MoveAssignable<decltype(next)>)
  {
   t = (T&&)other.t;
   next = (decltype(next)&&)other.next;
   return *this;
  }
 };

 //Tuple deduction guides
 template<typename... Types>
 requires (!(Array<Types> || ...))
 Tuple(Types...) -> Tuple<Types...>;

 /*
 //TODO this may be problematic; test
 template<typename... Types>
 Tuple(Types&...) -> Tuple<Types&...>;

 //TODO this may be problematic; test
 template<typename... Types>
 Tuple(Types&&...) -> Tuple<Types&&...>;
 */

 Tuple() -> Tuple<>;

 //TODO doc (same as std::tie)
 template<typename... Types>
 constexpr auto tie(Types&... types) noexcept {
  return Tuple{(Types&)types...};
 }

 //TODO
 // - doc
 // - noexcept qualifier
 template<typename... Types>
 constexpr auto rtie(Types&&... types) /*noexcept()*/ {
  return Tuple<Types&&...>{(Types&&)types...};
 }
}
