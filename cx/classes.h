#pragma once

#include "cx/common.h"
#include "cx/indirection.h"
#include "cx/templates.h"
#include "cx/idioms.h"

namespace CX {
 //Given a variadic template T and the variadic template arguments VS, ReverseSpecialize will produce an instantiation
 //of T given VS, where VS has been reassembled in the reverse order for N types (N is default sizeof...(VS))
 //TODO auto templates
 namespace Internal {
  //Reverses the order of N type-template arguments
  template<unsigned int N, template<typename...> typename Target, typename... Args>
  class ReverseSpecialize;

  //Trampoline specialization for argument order reversal
  template<unsigned int N, template<typename...> typename Target, typename _Dummy, typename Arg, typename... Args>
  class ReverseSpecialize<N, Target, _Dummy, Arg, Args...> {
  private:
   template<typename>
   class DummyDeducer;

   template<typename... DArgs>
   class DummyDeducer<Dummy<DArgs...>> {
   public:
    using dummy = Dummy<Arg, DArgs...>;
   };

   using dummy = typename DummyDeducer<_Dummy>::dummy;

  public:
   using type = typename ReverseSpecialize<N - 1U, Target, dummy, Args..., Arg>::type;
  };

  //Base specialization
  template<template<typename...> typename Target, typename _Dummy, typename Arg, typename... Args>
  class ReverseSpecialize<0U, Target, _Dummy, Arg, Args...> {
  public:
   using type = _Dummy;
  };
 }

 //CX API
 template<template<typename...> typename Target, typename... Args>
 class ReverseSpecialize {
 private:
  template<typename>
  class DummyDeducer;

  template<typename... DArgs>
  class DummyDeducer<Dummy<DArgs...>> {
  public:
   template<typename... Prefix>
   using type = Target<Prefix..., DArgs...>;
  };

  using deducer = DummyDeducer<typename Internal::ReverseSpecialize<sizeof...(Args), Target, Dummy<>, Args...>::type>;

 public:
  template<typename... Prefix>
  using type = typename deducer::template type<Prefix...>;
 };

 template<typename>
 struct IsTemplateTemplate;

 template<template<typename...> typename T, typename... Args>
 struct IsTemplateTemplate<T<Args...>> : true_type {};

 template<typename... T>
 struct IsTemplateTemplate<Dummy<T...>> : false_type {};

 //Explicit casting generators to disambiguate casting with multiple inheritance types
 namespace Internal {
  template<typename T>
  using CX_ComponentTypeResolver = CX::ComponentTypeResolver<T>;
  template<typename...>
  class UpcastingGenerator;

  template<typename Target, typename Step, typename... Steps>
  class UpcastingGenerator<Target, Step, Steps...> {
  private:
   using target_t = typename CX_ComponentTypeResolver<Target>::type *;
   using step_t = typename CX_ComponentTypeResolver<Step>::type *;
   using next_t = typename CX_ComponentTypeResolver<typename CX::TemplateTypeIterator<0U, Steps...>::type>::type *;

  public:
   [[gnu::always_inline]]
   inline static target_t cast(step_t step) {
    return UpcastingGenerator<Target, Steps...>::cast((next_t)step);
   }
  };

  template<typename Target, typename Step>
  class UpcastingGenerator<Target, Step> {
  private:
   using target_t = typename CX_ComponentTypeResolver<Target>::type *;
   using step_t = typename CX_ComponentTypeResolver<Step>::type *;

  public:
   [[gnu::always_inline]]
   inline static target_t cast(step_t step) {
    return (target_t)step;
   }
  };

  template<typename...>
  class DowncastingGenerator;

  template<typename Target, typename Step, typename... Steps>
  class DowncastingGenerator<Target, Step, Steps...> {
  private:
   using target_t = typename CX_ComponentTypeResolver<Target>::type *;
   using step_t = typename CX_ComponentTypeResolver<Step>::type *;
   using next_t = typename CX_ComponentTypeResolver<typename CX::TemplateTypeIterator<0U, Steps...>::type>::type *;

  public:
   [[gnu::always_inline]]
   inline static target_t cast(step_t step) {
    return DowncastingGenerator<Target, Steps...>::cast((next_t)step);
   }
  };

  template<typename Target, typename Step>
  class DowncastingGenerator<Target, Step> {
  private:
   using target_t = typename CX_ComponentTypeResolver<Target>::type *;
   using step_t = typename CX_ComponentTypeResolver<Step>::type *;

  public:
   [[gnu::always_inline]]
   inline static target_t cast(step_t step) {
    return (target_t)step;
   }
  };
 }

 //CX API
 template<typename...>
 class ExplicitCastGenerator;

 //Static assertion to prevent the compiler from spewing template errors and provide users with relevant information
 //about their incorrect usage of ExplicitCastGenerator
 template<typename Target>
 class ExplicitCastGenerator<Target> {
 private:
  class _Dummy;

  using target_t = typename ComponentTypeResolver<Target>::type *;

 public:
  [[gnu::always_inline]]
  inline static target_t cast(void *) {
   //Assertion will always fail
   static_assert(
    IsSame<Target, _Dummy>::value,
    "No explicit cast is required if a polymorphic derived type is not going to be casted to a polymorphic base that "
    "is at least one type removed from the derivation. ExplicitCastGenerator is intended to be used with three or "
    "more template parameters!"
   );
   //Execution will never reach this line, it is simply here to avoid compiler errors
   return nullptr;
  }
 };

 //Also incorrect usage, causes static assertion in ExplicitCastGenerator<typename> to trip
 template<typename T1, typename T2>
 class ExplicitCastGenerator<T1, T2> : public ExplicitCastGenerator<T1> {
 private:
  using target_t = typename ComponentTypeResolver<T1>::type *;

 public:
  [[gnu::always_inline]]
  inline static target_t cast(void *) {
   return ExplicitCastGenerator<T1>::cast(nullptr);
  }
 };

 template<typename Target, typename... Steps>
 class ExplicitCastGenerator<Target, Steps...> {
 private:
  using steps_end_t = typename CX::TemplateTypeIterator<sizeof...(Steps) - 1U, Steps...>::type;

  //Upcasting types
  using target_u_t = typename ComponentTypeResolver<steps_end_t>::type *;
  using step_u_t = typename ComponentTypeResolver<Target>::type *;

  //Downcasting types
  using target_d_t = step_u_t;
  using step_d_t = target_u_t;

  [[gnu::always_inline]]
  inline static constexpr void assertArgumentCompliance() {
   static_assert(
    __is_class(typename ComponentTypeResolver<Target>::type)
    && (__is_class(typename ComponentTypeResolver<Steps>::type) && ...),
    "Explicit casting routes can only be generated for polymorphic types!"
   );
  }

 public:
  using upcast_generator = typename CX::TemplateTypeArgCycler<
   0,
   sizeof...(Steps),
   Internal::UpcastingGenerator,
   Target,
   Steps...
  >::type;

  using downcast_generator = typename CX::ReverseSpecialize<
   Internal::DowncastingGenerator,
   Steps...
  >::template type<Target>;

  //Upcasting function
  [[gnu::always_inline]]
  inline static target_u_t cast(step_u_t arg) {
   assertArgumentCompliance();
   return upcast_generator::cast(arg);
  }

  //Downcasting function
  [[gnu::always_inline]]
  inline static target_d_t cast(step_d_t arg) {
   assertArgumentCompliance();
   return downcast_generator::cast(arg);
  }
 };
}
