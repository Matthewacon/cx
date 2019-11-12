#pragma once

#include <cstdarg>

#include "cx/common.h"
#include "cx/idioms.h"

namespace CX {
 namespace Internal {
  struct va_list_wrapper;

  template<typename T>
  constexpr T doNothing(T) noexcept;

  template<typename T>
  constexpr T doNothing(T*) noexcept;

  using __va_list_t = decltype(doNothing(CX::declval<va_list>()));

  template<typename = va_list>
  struct FunctionSelector;

  //everyone else
  template<>
  struct FunctionSelector<__va_list_t> {
   using conformant_va_list_t = __va_list_t&;

   template<typename T, typename LT>
   [[gnu::always_inline]]
   inline static T safe_va_arg(LT& t) {
    static_assert(IsSame<LT, va_list_wrapper>::value);
    return (T)va_arg(t, T);
   }

   [[gnu::always_inline]]
   inline static __va_list_t& normalize(conformant_va_list_t& list) noexcept {
    return list;
   }
  };

  //x86_64 and amd64
  template<>
  struct FunctionSelector<__va_list_t[1]> {
   using conformant_va_list_t = __va_list_t *;

   template<typename T, typename LT>
   [[gnu::always_inline]]
   inline static T safe_va_arg(LT& t) {
    static_assert(IsSame<LT, va_list_wrapper>::value);
    return (T)va_arg(&t, T);
   }

   [[gnu::always_inline]]
   inline static __va_list_t& normalize(const conformant_va_list_t& list) noexcept {
    return *const_cast<conformant_va_list_t&>(list);
   }
  };

  using __va_list_function_selector = FunctionSelector<>;
  using __conformant_va_list_t = __va_list_function_selector::conformant_va_list_t;

  struct va_list_wrapper final {
   __va_list_t _list;

   va_list_wrapper() = default;

   va_list_wrapper(const __conformant_va_list_t& list) :
    _list(__va_list_function_selector::normalize(list))
   {}

   va_list_wrapper(const __conformant_va_list_t&& list) :
    _list(__va_list_function_selector::normalize(list))
   {}

   va_list_wrapper(const va_list_wrapper& wrapper) :
    _list(wrapper._list)
   {}

   va_list_wrapper(const va_list_wrapper&& wrapper) :
    _list(wrapper._list)
   {}

   [[nodiscard]]
   va_list_wrapper& operator=(const va_list_wrapper& wrapper) {
    _list = wrapper._list;
    return *this;
   }

   [[nodiscard]]
   va_list_wrapper& operator=(const va_list& list) {
    _list = __va_list_function_selector::normalize(const_cast<va_list&>(list));
    return *this;
   }

   [[gnu::always_inline]]
   inline __va_list_t* operator&() const {
    auto& ref = const_cast<va_list_wrapper &>(*this);
    return &ref._list;
   }

   [[gnu::always_inline]]
   operator __va_list_t&() const {
    auto& ref = const_cast<va_list_wrapper&>(*this);
    return ref._list;
   }

   [[gnu::always_inline]]
   operator __va_list_t*() const {
    auto& ref = const_cast<va_list_wrapper&>(*this);
    return &ref._list;
   }
  };
 }
 
 using va_list_t = Internal::va_list_wrapper;

 template<typename T>
 [[gnu::always_inline]]
 inline T safe_va_arg(va_list_t& list) {
  using selector_t = Internal::__va_list_function_selector;
  constexpr const auto t_size = sizeof(T);
  if constexpr(t_size > sizeof(long double)) {
   //this assertion will always fail
   static_assert(
    t_size <= sizeof(long double),
    "Type too large to consume off of va_list, use pointer instead!"
   );
  } else if constexpr (IsPointer<T>::value) {
   return selector_t::safe_va_arg<T>(list);
  } else if constexpr (t_size > sizeof(double)) {
   return (T)selector_t::safe_va_arg<long double>(list);
  } else if constexpr(t_size > sizeof(int)) {
   return (T)selector_t::safe_va_arg<double>(list);
  } else {
   return (T)selector_t::safe_va_arg<int>(list);
  }
 }

 [[gnu::always_inline nodiscard]]
 inline typename Internal::__conformant_va_list_t conform(va_list_t& list) noexcept {
  return list;
 }
}