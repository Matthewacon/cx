#pragma once

#include "cx/common.h"
#include "cx/idioms.h"

#include <stdexcept>
#include <memory>

namespace CX {
 template<typename>
 struct Lambda;

 template<typename R, typename... Args>
 struct Lambda<R (Args...)> {
  friend Lambda<R (Args...) const>;
  friend Lambda<R (Args...) noexcept>;
  friend Lambda<R (Args...) const noexcept>;

 public:
  struct FunctorEncapsulator {
   virtual ~FunctorEncapsulator() = default;
   [[gnu::always_inline]]
   inline virtual R operator()(Args...) const = 0;
  };

  using lambda_t = Lambda<R (Args...)>;

 private:
  std::shared_ptr<FunctorEncapsulator> functor;

 public:
  Lambda() noexcept = default;

  template<typename T>
  Lambda(T&& t) noexcept {
   operator=(static_cast<typename ComponentTypeResolver<T>::type&&>(t));
  }

  Lambda(R (* const func)(Args...)) noexcept {
   operator=(func);
  }

  Lambda(lambda_t& l) noexcept {
   operator=(l);
  }

  template<>
  Lambda(lambda_t&& l) noexcept {
   operator=((lambda_t&&)l);
  }

  template<typename T>
  Lambda& operator=(const T&& t) noexcept {
   functor.reset();
   struct LambdaEncapsulator : FunctorEncapsulator {
    static_assert(FunctionOperatorExists<T>::value);

    mutable typename StripReferences<T>::type t;

    LambdaEncapsulator(decltype(t)&& t) noexcept : t(t) {}

    [[gnu::always_inline]]
    inline R operator()(Args... args) const override {
     return t(args...);
    }
   };
   functor = std::make_shared<LambdaEncapsulator>((T&&)t);
   return *this;
  }

  Lambda& operator=(R (* const func)(Args...)) noexcept {
   struct FunctionEncapsulator {
    R (* const func)(Args...);

    FunctionEncapsulator(decltype(func) func) noexcept : func(func) {}

    [[gnu::always_inline]]
    inline R operator()(Args... args) noexcept override {
     return func(args...);
    }
   };
   functor = std::make_shared<FunctionEncapsulator>(func);
  }

  Lambda& operator=(const lambda_t& l) noexcept {
   functor = l.functor;
   return *this;
  }

  template<>
  Lambda& operator=(const lambda_t&& l) noexcept {
   functor = l.functor;
   const_cast<lambda_t&&>(l).functor.reset();
   return *this;
  }

  R operator()(Args... args) const {
   if (!functor) {
    throw std::runtime_error("Lambda is uninitialized!");
   }
   return (*functor)(args...);
  }
 };

 template<typename R, typename... Args>
 struct Lambda<R (Args...) const> : Lambda<R (Args...)> {
  using Lambda<R (Args...)>::Lambda;
 };

 template<typename R, typename... Args>
 struct Lambda<R (Args...) noexcept> : Lambda<R (Args...)> {
  using Lambda<R (Args...)>::Lambda;
 };

 template<typename R, typename... Args>
 struct Lambda<R (Args...) const noexcept> : Lambda<R (Args...) noexcept> {
  using Lambda<R (Args...) noexcept>::Lambda;
 };
}