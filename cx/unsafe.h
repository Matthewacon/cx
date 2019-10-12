#pragma once

namespace CX {
 namespace Internal {
  template<typename T1, typename T2>
  union union_cast_t {
  private:
   T1 t1;
   T2 t2;

  public:
   constexpr union_cast_t(T2 t2) noexcept :
    t2(t2)
   {}

   constexpr T1& operator()() noexcept {
    return t1;
   }
  };
 }

 template<typename T1, typename T2>
 constexpr T1& union_cast(T2 t2) noexcept {
  return Internal::union_cast_t<T1, T2>(t2)();
 }
}
