#pragma once

namespace CX {
 template<typename R, typename T>
 constexpr R union_cast(T t) noexcept {
  union cast {
   T t;
   R r;
   constexpr cast(T t) noexcept : t(t) {}
  };
  return cast{t}.r;
 }
}