#pragma once

#include "cx/templates.h"

namespace CX {
 //Completely type-safe constexpr tuple
 template<typename...>
 class Tuple {};

//Consider downward-linking the tuple upon construction from the base
 template<typename T, typename... TS>
 class Tuple<T, TS...> : public Tuple<TS...> {
 private:
  template<unsigned int N, typename TT, typename... TTS>
  static constexpr typename TemplateTypeIterator<N, TT, TTS...>::type get(const Tuple<TT, TTS...> &tuple) {
   const Tuple<TTS...> &t = tuple;
   if constexpr(N != 0) {
    return get<N - 1>(t);
   } else {
    return tuple.t;
   }
  }

 public:
  const T t;

  constexpr Tuple(T t, TS... ts) : Tuple<TS...>::Tuple{ts...}, t(t) {}

  template<unsigned int N>
  constexpr typename TemplateTypeIterator<N, T, TS...>::type get() const {
   return get<N, T, TS...>(*this);
  }

  static constexpr unsigned int length() {
   return sizeof...(TS) + 1;
  }

  template<typename I, unsigned int N = length() - 1>
  constexpr void reverseIterate() const {
   I::process(get<N>());
   if constexpr(N != 0) {
    reverseIterate<I, N - 1>();
   }
  }

  template<template<typename> typename I, unsigned int N = length() - 1>
  constexpr void specializeReverseIterate() const {
   I<typename TemplateTypeIterator<N, T, TS...>::type>::process(get<N>());
   if constexpr(N != 0) {
    specializeReverseIterate<I, N - 1>();
   }
  }

  template<typename I, unsigned int N = 0>
  constexpr void iterate() const {
   I::process(get<N>());
   if constexpr(N != length() - 1) {
    iterate<I, N + 1>();
   }
  }

  template<template<typename> typename I, unsigned int N = 0>
  constexpr void specializeIterate() const {
   I<typename TemplateTypeIterator<N, T, TS...>::type>::process(get<N>());
   if constexpr(N != length() - 1) {
    specializeIterate<I, N + 1>();
   }
  }
 };
}