#pragma once

#include <cx/idioms.h>

namespace CX {
 //Union casting utility
 //Warning: Union casting is technically UB, but is supported
 //by most major compilers. Use at your own risk.
 template<typename R, typename S>
 R unionCast(S s) {
  union {
   R r;
   S s;
  } u{ .s = s };
  return u.r;
 }

 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
 #pragma GCC diagnostic ignored "-Wnested-anon-types"
  //Componentized member pointer utility type
  //Note: Assumes Intel Itanium C++ ABI
  struct MemberPtr {
   struct [[gnu::packed]] Components {
    void
     * ptr,
     * adj;
   };

   union {
    Components components;
    void (MemberPtr::*memberPtr)();
   };

   static_assert(
    sizeof(Components) == sizeof(memberPtr),
    "\"CX::MemberPtr\" cannot be used with this platform as it is using a"
    "different ABI. Please file a bug report to "
    "https://github.com/Matthewacon/CX so support for your platform can "
    "be added."
   );

   template<MemberFunction F>
   MemberPtr(F f) :
    memberPtr((decltype(memberPtr))f)
   {}
  };
 #pragma GCC diagnostic pop
}
