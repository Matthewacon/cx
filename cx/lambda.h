#pragma once

#include "cx/common.h"
#include "cx/idioms.h"
#include "cx/unsafe.h"

namespace CX {
 template<typename F, bool = CX::IsFunction<F>::value>
 struct Lambda;

 template<typename R, typename... Args>
 struct Lambda<R (Args...), true> {
  using lambda_t = Lambda<R (Args...)>;

  enum Type : unsigned char {
   PLAIN,
   ANONYMOUS,
   UINIT
  };

  Lambda() noexcept :
   functor([]() {
    auto data = new char[sizeof(Type)];
    *(Type *)data = UINIT;
    return data;
   }())
  {}

  Lambda(R (*func)(Args...)) noexcept :
   functor([&]() {
    auto data = new char[sizeof(Type) + sizeof(void *)];
    *(Type *)data = PLAIN;
    *(void **)(data + sizeof(Type)) = (void *)func;
    return data;
   }())
  {}

  template<typename T>
  Lambda(T inst) noexcept :
   functor(nullptr)
  {
   copy(ANONYMOUS, &functor, &inst);
   //TODO this is causing issues for some lambda functions?
//    static_assert(FunctionOperatorExists<T>::value);
  }

  Lambda(const lambda_t& other) noexcept :
   functor(nullptr)
  {
   auto pCopy = (void (*)(Type, char **, char *))(*(void **)(other.functor + sizeof(Type)));
   pCopy(*(Type *)other.functor, &functor, ((char *)other.functor + sizeof(Type) + 2 * sizeof(void *)));
  }

  Lambda(const lambda_t&& other) noexcept :
   functor(other.functor)
  {
   const_cast<lambda_t&>(other).functor = nullptr;
  }

  ~Lambda() {
   cleanup();
  }

  lambda_t& operator=(const lambda_t& other) noexcept {
   cleanup();
   switch (*(Type *)other.functor) {
    case PLAIN: {
     functor = new char[sizeof(Type) + sizeof(void *)];
     *(void **)(functor + sizeof(Type)) = *(void **)(other.functor + sizeof(Type));
     break;
    }
    case ANONYMOUS: {
     auto pCopy = (void (*)(Type, char **, char *))(*(void **)(other.functor + sizeof(Type)));
     pCopy(ANONYMOUS, &functor, ((char *)other.functor + sizeof(Type) + 2 * sizeof(void *)));
     break;
    }
    case UINIT: {
     functor = new char[sizeof(Type) + sizeof(void *)];
     *(void **)(functor + sizeof(Type)) = nullptr;
     break;
    }
   }
   *(Type *)functor = *(Type *)other.functor;
   return *this;
  }

  lambda_t& operator=(const lambda_t&& other) noexcept {
   functor = other.functor;
   other.functor = nullptr;
   return *this;
  }

  lambda_t& operator=(R (* const func)(Args...)) noexcept {
   cleanup();
   functor = new char[sizeof(Type) + sizeof(void *)];
   *(Type *)functor = PLAIN;
   *(void **)(functor + sizeof(Type)) = (void *)func;
   return *this;
  }

  template<typename T>
  lambda_t& operator=(T inst) noexcept {
   cleanup();
   copy(ANONYMOUS, &functor, &inst);
   return *this;
  }

  [[gnu::always_inline]]
  inline R operator()(Args... args) const {
   switch(*(Type *)functor) {
    case PLAIN: {
     return union_cast<R (*)(Args...)>(*(void **)(functor + sizeof(Type)))(args...);
    }
    case ANONYMOUS: {
     auto pMem = member_ptr_align_t{*(void **)(functor + sizeof(Type) + sizeof(void *)), nullptr};
     auto pInst = (void *)(functor + sizeof(Type) + 2*sizeof(void *));
     return (union_cast<Dummy<> *>(pInst)->*union_cast<R (Dummy<>::*)(Args...)>(pMem))(args...);
    }
    case UINIT: {
     throw std::runtime_error("FATAL: Lambda functor is uninitialized!");
    }
   }
  }

 private:
  char * functor;

  [[gnu::always_inline]]
  inline void cleanup() const noexcept {
   if (functor) {
    delete[] functor;
   }
  }

  template<typename T>
  static void copy(Type type, char ** target, T * functor) {
   constexpr R (T::* const func)(Args...) const = &T::operator();
   //Layout: [sizeof(type): type, sizeof(void *): copy, sizeof(void *): &T::operator(), sizeof(T): functor]
   *target = new char[sizeof(Type) + (2 * sizeof(void *)) + sizeof(T)];
   *(Type *)*target = type;
   *(void **)(*target + sizeof(Type)) = (void *)&copy<T>;
   *(void **)(*target + sizeof(Type) + sizeof(void *)) = (void *)union_cast<member_ptr_align_t>(func).ptr;
   memcpy((void *)(*target + sizeof(Type) + 2*sizeof(void *)), (void *)functor, sizeof(T));
  }
 };
}