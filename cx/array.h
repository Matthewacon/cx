#pragma once

#include "cx/templates.h"

//TODO This one's not so easy...
//Cosntexpr array like std::array<typename T, std::size_t size>, except, the size need not be explicitly stated in
//constexpr construction. Can also accept a CX::Tuple with any parameters, with a predefined constexpr conversion
//function to unify all types to the array's target type, to create a homogeneous array of polymorphic values. - WIP
namespace CX {
 template<typename...>
 class Base {};

 template<typename T>
 class Base<T> {
 private:
  inline static constexpr Base<T, T> default_inst{};

 public:
  const Base<T, T> last;
  const T t;

  constexpr Base(const Base<T, T>& last, const T t) noexcept :
   t(t)
  {}
 };

 template<typename T, typename... TS>
 class Base<T, TS...> {
 public:
  const typename select_if_match_M<is_specialized<Base<T, T, TS...>>::value, true, const Base<T, T, TS...>&, nullptr_t>::type last;
  const Base<TS...> next;
  const T t;

  explicit constexpr Base(const T t, const TS... ts) noexcept :
   last(nullptr),
   next(Base<TS...>(*this, ts...)),
   t(t)
  {}

  explicit constexpr Base(const Base<T, T, TS...>& last, const T t, const TS... ts) noexcept :
   last(last),
   next(Base<TS...>(*this, ts...)),
   t(t)
  {}
 };

 int main() {
  constexpr Base<int, int, int> b(1, 2, 3);
  return 0;
 }
}