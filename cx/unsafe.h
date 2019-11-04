#pragma once

namespace CX {
 struct member_ptr_align_t {
  void *ptr, *offset;
 } __attribute__((packed));

 template<typename R, typename T>
 constexpr R union_cast(T t) noexcept {
  union cast {
   T t;
   R r;
   constexpr cast(T t) noexcept : t(t) {}
   ~cast() noexcept {};
  };
  return cast{t}.r;
 }
}
