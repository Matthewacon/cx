#pragma once

namespace CX {
 template<typename T>
 union union_cast;

 template<typename T>
 union union_cast<T*> final {
  void * ptr;
  T * t_ptr;

  template<typename A>
  constexpr union_cast(A * ptr) noexcept : ptr((void *)ptr)
  {}

  constexpr T* operator->() {
   return t_ptr;
  }

  template<typename A>
  decltype(t_ptr + std::declval<A>()) operator+(A a) {
   return t_ptr + a;
  }

  template<typename A>
  decltype(t_ptr - std::declval<A>()) operator-(A a) {
   return t_ptr - a;
  }

  decltype(t_ptr++) operator++() {
   return t_ptr++;
  }

  decltype(t_ptr--) operator--() {
   return t_ptr--;
  }

  template<typename A>
  decltype(t_ptr += std::declval<A>()) operator+=(A a) {
   return t_ptr += a;
  }

  template<typename A>
  decltype(t_ptr -= std::declval<A>()) operator-=(A a) {
   return t_ptr -= a;
  }

  constexpr T* operator()() {
   return t_ptr;
  }
 };
}