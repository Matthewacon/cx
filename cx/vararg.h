#pragma once

#include <cstdarg>

#include "cx/common.h"

namespace CX {
 namespace Internal {
  template<typename T>
  constexpr T doNothing(T) noexcept;

  template<typename T>
  constexpr T doNothing(T*) noexcept;

  using __va_list_t = decltype(doNothing(CX::declval<va_list>()));

  template<typename = va_list>
  struct FunctionSelector;

  template<>
  struct FunctionSelector<__va_list_t> {
   template<typename T, typename LT>
   [[gnu::always_inline]]
   inline static T safe_va_arg(LT& t) {
    return internal_va_arg<T>((__va_list_t&)t);
   }

  private:
   //everyone else
   template<typename T, typename LT>
   [[gnu::always_inline]]
   inline static T internal_va_arg(LT& t) {
    return (T)va_arg(t, T);
   }
  };

  //x86_64 and amd64
  template<>
  struct FunctionSelector<__va_list_t[1]> {
   template<typename T, typename LT>
   [[gnu::always_inline]]
   inline static T safe_va_arg(LT& t) {
    return (T)va_arg(&t, T);
   }
  };

  using __va_list_function_selector = FunctionSelector<>;

  struct va_list_wrapper final {
   __va_list_t list;

   [[gnu::always_inline]]
   inline __va_list_t* operator&() const {
    auto& ref = const_cast<va_list_wrapper &>(*this);
    return &ref.list;
   }

   [[gnu::always_inline]]
   operator __va_list_t&() const {
    auto& ref = const_cast<va_list_wrapper&>(*this);
    return ref.list;
   }

   [[gnu::always_inline]]
   operator __va_list_t*() const {
    auto& ref = const_cast<va_list_wrapper&>(*this);
    return &ref.list;
   }
  };
 }
 
 using va_list_t = Internal::va_list_wrapper;

 template<typename T>
 [[gnu::always_inline]]
 T safe_va_arg(va_list_t& list) {
  using selector_t = Internal::__va_list_function_selector;
  constexpr const auto t_size = sizeof(T);
  if constexpr(t_size > sizeof(long double)) {
   //this assertion will always fail
   static_assert(
    t_size <= sizeof(long double),
    "Type too large to consume off of va_list, use pointer instead!"
   );
  } else if constexpr (t_size > sizeof(double)) {
   return selector_t::safe_va_arg<long double>(list);
  } else if constexpr(t_size > sizeof(int)) {
   return selector_t::safe_va_arg<double>(list);
  } else {
   return selector_t::safe_va_arg<int>(list);
  }
 }
}
