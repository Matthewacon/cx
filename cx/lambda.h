#pragma once

#include "cx/common.h"
#include "cx/idioms.h"
#include "cx/unsafe.h"

namespace CX {
 namespace Internal {
  template<typename T>
  struct FunctionCapture;

  //TODO combine this with the Lambda template
  //non-capturing lambdas are implicitly convertible to plain function pointers
  template<typename R, typename... Args>
  struct FunctionCapture<R (Args...)> {
   using functor_t = FunctionCapture<R (Args...)>;

   enum Type {
    PLAIN,
    ANONYMOUS,
    UINIT
   };

   Type type;

   constexpr FunctionCapture() noexcept :
    type(UINIT),
    functor(nullptr)
   {}

   constexpr FunctionCapture(R (*func)(Args...)) noexcept :
    type(PLAIN),
    functor(union_cast<char *>(func))
   {}

   template<typename T>
   constexpr FunctionCapture(T inst) noexcept :
    type(ANONYMOUS),
    functor([=]() {
     auto data = new char[sizeof(T) + sizeof(void *)];
     constexpr R (T::* const func)(Args...) const = &T::operator();
     *((void **)data) = union_cast<member_ptr_align_t>(func).ptr;
     memcpy(data + sizeof(void *), &inst, sizeof(T));
     return data;
    }())
   {
    //TODO this is causing issues for some lambda functions?
//    static_assert(FunctionOperatorExists<T>::value);
   }

   ~FunctionCapture() {
    cleanup();
   }

   //TODO anonymous struct size required...
// functor_t& operator=(const FunctionCapture<R (Args...)>& other) const noexcept {
//  cleanup();
//  switch (other.type) {
//   case PLAIN: {
//    functor = other.functor;
//    type = PLAIN;
//    break;
//   }
//   case ANONYMOUS: {
//    functor = ;
//    type = ANONYMOUS;
//    break;
//   }
//  }
//  return *this;
// }

   functor_t& operator=(R (* const func)(Args...)) const noexcept {
    cleanup();
    functor = union_cast<char *>(func);
    type = PLAIN;
    return *this;
   }

   template<typename T>
   functor_t& operator=(T inst) const noexcept {
    cleanup();
    functor = new char[sizeof(T) + sizeof(void *)];
    type = ANONYMOUS;
    constexpr R (T::* const func)(Args...) const = &T::operator();
    *((void **)functor) = union_cast<member_ptr_align_t>(func).ptr;
    memcpy(functor + sizeof(void *), &inst, sizeof(T));
    return *this;
   }

   [[gnu::always_inline]]
   inline R operator()(Args... args) const {
    switch(type) {
     case PLAIN: return (union_cast<R (*)(Args...)>(functor))(args...);
     case ANONYMOUS: {
      auto memPtr = member_ptr_align_t{*((void **)functor), nullptr};
      return (union_cast<Dummy<> *>(functor + sizeof(void *))->*union_cast<R (Dummy<>::*)(Args...)>(memPtr))(args...);
     }
     case UINIT: throw std::runtime_error("FATAL: Lambda functor is uninitialized!");
    }
   }

  private:
   char * const functor;

   [[gnu::always_inline]]
   inline void cleanup() const noexcept {
    if (type == ANONYMOUS) {
     delete[] functor;
    }
   }
  };
 }

 template<typename F, bool = CX::IsFunction<F>::value>
 struct Lambda;

 template<typename R, typename... Args>
 struct Lambda<R (Args...), true> {
  using func_t = R (Args...);

  Internal::FunctionCapture<func_t> capture;

  constexpr Lambda() noexcept = default;

  template<typename T>
  constexpr Lambda(T t) : capture(t)
  {}

  constexpr Lambda(func_t * const func) : capture(func)
  {}

  [[gnu::always_inline nodiscard]]
  inline constexpr R operator()(Args... args) const {
   return capture(args...);
  }
 };
}