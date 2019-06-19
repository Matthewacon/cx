#pragma once

namespace CX {
 //General purpose dummy template type for nested template argument deduction
 template<typename...>
 class _Dummy {};

 //TODO auto templates
 //Reverses the order of N type-template arguments
 template<unsigned int N, template<typename...> typename Target, typename... Args>
 class _ReverseSpecialize;

 template<unsigned int N, template<typename...> typename Target, typename Dummy, typename Arg, typename... Args>
 class _ReverseSpecialize<N, Target, Dummy, Arg, Args...> {
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
  using type = typename _ReverseSpecialize<N - 1U, Target, dummy, Args..., Arg>::type;
 };

 template<template<typename...> typename Target, typename Dummy, typename Arg, typename... Args>
 class _ReverseSpecialize<0U, Target, Dummy, Arg, Args...> {
 public:
  using type = Dummy;
 };

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

  using deducer = DummyDeducer<typename _ReverseSpecialize<sizeof...(Args), Target, _Dummy<>, Args...>::type>;

 public:
  template<typename... Prefix>
  using type = typename deducer::template type<Prefix...>;
 };

 //Negative idiom specialization
 template<typename Target, typename = void, typename... Args>
 class _HasConstructor: public std::false_type {};

 //Positive idiom specialization
 template<typename Target, typename... Args>
 class _HasConstructor<Target, std::void_t<decltype(Target{(std::declval<Args>(), ...)})>, Args...> :
 public std::true_type
 {};

 //Positive idiom specialization for empty parameter pack
 //Negative idiom for empty parameter pack selects the base specialization
 template<typename Target>
 class _HasConstructor<Target, std::void_t<decltype(Target{})>> : public std::true_type {};

 //CX API
 template<typename Target, typename... Args>
 using HasConstructor = _HasConstructor<Target, void, Args...>;

 template<typename>
 class IsTemplateTemplate;

 template<template<typename...> typename T, typename... Args>
 class IsTemplateTemplate<T<Args...>> : public std::true_type {};

 template<typename... T>
 class IsTemplateTemplate<_Dummy<T...>> : public std::false_type {};
}