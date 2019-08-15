#pragma once

#include "cx/common.h"
#include "cx/indirection.h"

namespace CX {
 //Given a variadic template T and the variadic template arguments VS, ReverseSpecialize will produce an instantiation
 //of T given VS, where VS has been reassembled in the reverse order for N types (N is default sizeof...(VS))
 //TODO auto templates
 namespace Internal {
  //Reverses the order of N type-template arguments
  template<unsigned int N, template<typename...> typename Target, typename... Args>
  class ReverseSpecialize;

  //Trampoline specialization for argument order reversal
  template<unsigned int N, template<typename...> typename Target, typename Dummy, typename Arg, typename... Args>
  class ReverseSpecialize<N, Target, Dummy, Arg, Args...> {
  private:
   template<typename>
   class DummyDeducer;

   template<typename... DArgs>
   class DummyDeducer<_Dummy<DArgs...>> {
   public:
    using dummy = _Dummy<Arg, DArgs...>;
   };

   using dummy = typename DummyDeducer<Dummy>::dummy;

  public:
   using type = typename ReverseSpecialize<N - 1U, Target, dummy, Args..., Arg>::type;
  };

  //Base specialization
  template<template<typename...> typename Target, typename Dummy, typename Arg, typename... Args>
  class ReverseSpecialize<0U, Target, Dummy, Arg, Args...> {
  public:
   using type = Dummy;
  };
 }

 //CX API
 template<template<typename...> typename Target, typename... Args>
 class ReverseSpecialize {
 private:
  template<typename>
  class DummyDeducer;

  template<typename... DArgs>
  class DummyDeducer<_Dummy<DArgs...>> {
  public:
   template<typename... Prefix>
   using type = Target<Prefix..., DArgs...>;
  };

  using deducer = DummyDeducer<typename Internal::ReverseSpecialize<sizeof...(Args), Target, _Dummy<>, Args...>::type>;

 public:
  template<typename... Prefix>
  using type = typename deducer::template type<Prefix...>;
 };

 struct true_type {
  static constexpr const auto value = true;
 };

 struct false_type {
  static constexpr const auto value = false;
 };

 template<typename T>
 using void_t = void;

//Detection idioms for constructor presence
 namespace Internal {
  //Negative idiom specialization
  template<typename Target, typename = void, typename... Args>
  class HasConstructor: public false_type {};

  //Positive idiom specialization
  template<typename Target, typename... Args>
  class HasConstructor<Target, void_t<decltype(Target{(std::declval<Args>(), ...)})>, Args...> :
   public true_type
  {};

  //Positive idiom specialization for empty parameter pack
  //This specialization must be explicitly defined since an empty parameter pack
  //evaluates to `{void}` which would interfere with the actual signature that the
  //user is testing for.
  //
  //Negative idiom for empty parameter pack selects the base specialization
  template<typename Target>
  class HasConstructor<Target, void_t<decltype(Target{})>> : public std::true_type {};
 }

 //CX API
 template<typename Target, typename... Args>
 using HasConstructor = Internal::HasConstructor<Target, void, Args...>;

 template<typename>
 class IsTemplateTemplate;

 template<template<typename...> typename T, typename... Args>
 class IsTemplateTemplate<T<Args...>> : public std::true_type {};

 template<typename... T>
 class IsTemplateTemplate<_Dummy<T...>> : public std::false_type {};

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
  class Dummy;

  using target_t = typename ComponentTypeResolver<Target>::type *;

 public:
  [[gnu::always_inline]]
  inline static target_t cast(void *) {
   //Assertion will always fail
   static_assert(
    std::is_same<Target, Dummy>::value,
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
    std::is_class<typename ComponentTypeResolver<Target>::type>::value
    && (std::is_class<typename ComponentTypeResolver<Steps>::type>::value && ...),
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