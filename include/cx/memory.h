#pragma once

#include <cx/lambda.h>
#include <cx/error.h>

#ifdef CX_STL_SUPPORT
 #include <vector>
#endif

namespace CX {
 struct DeferError final {
  char const * message;

  DeferError(char const * message) noexcept :
   message(message)
  {}

  constexpr char const * describe() const noexcept {
   return message;
  }
 };
 static_assert(IsError<DeferError>);

 //Non-allocating deferral mechanism
 template<auto N>
 struct Defer final {
 private:
  CX::Lambda<void ()> deferrals[N];
  decltype(N) count;

 public:
  Defer() :
   count(0)
  {};

  inline ~Defer() {
   drain();
  }

  template<typename F>
  requires requires (F f) {
   CX::Lambda{f};
  }
  Defer& operator+=(F f) noexcept {
   if (count >= N) {
    exit(DeferError{
     "Deferral mechanism is at capacity. Consider increasing the size of "
     "the deferral mechanism to accommodate your needs, or use the "
     "alternative `AllocDefer` mechanism, which does not have size "
     "restrictions."
    });
   } else {
    deferrals[count++] = f;
   }
   return *this;
  }

  inline void drain() {
   for (int i = 0; i < count; i++) {
    auto &deferred = deferrals[i];
    deferred();
    deferred.reset();
   }
   count = 0;
  }
 };

 //General purpose deferral mechanism
 #ifdef CX_STL_SUPPORT
  struct AllocDefer final {
  private:
   std::vector<CX::Lambda<void ()>> deferrals;

  public:
   AllocDefer() = default;

   inline ~AllocDefer() {
    drain();
   }

   template<typename F>
   requires requires (F f) {
    CX::Lambda{f};
   }
   AllocDefer& operator+=(F f) noexcept {
    deferrals.push_back(CX::Lambda{f});
    return *this;
   }

   inline void drain() {
    for (auto &deferred : deferrals) {
     deferred();
    }
    //TODO consider re-using existing lambda entries by copy-assigning
    //new deferred functions when they're pushed
    deferrals.clear();
   }
  };
 #endif
}
