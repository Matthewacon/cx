#pragma once

namespace CX {
 //TODO for all template related tools, add parameter separation for construction
//Produces the type at 'N' in the vararg list 'T, (TS...)'
 template<unsigned int N, typename T, typename... TS>
 class TemplateTypeIterator {
 public:
  using type = typename TemplateTypeIterator<N - 1, TS...>::type;
 };

 template<typename T, typename... TS>
 class TemplateTypeIterator<0, T, TS...> {
 public:
  using type = T;
 };

 //Produces the value at 'N' in the vararg list 'T, (TS...)'
 template<unsigned int N, auto T, auto... TS>
 class TemplateAutoIterator {
 public:
  inline static constexpr const auto value = TemplateAutoIterator<N - 1, TS...>::value;
 };

 template<auto T, auto... TS>
 class TemplateAutoIterator<0, T, TS...> {
 public:
  inline static constexpr const auto value = T;
 };

 //Produces a specialized Target<...> with the template parameters from 'Start' to 'End' inclusive
 template<unsigned int Start, unsigned int End, template<typename...> typename Target, typename T, typename... TS>
 class TemplateTypeAssembler {
 public:
  using type = typename TemplateTypeAssembler<Start - 1, End - 1, Target, TS...>::type;
 };

 template<unsigned int Crop, unsigned int Cycles, template<typename...> typename Target, typename T, typename... TS>
 class TemplateTypeArgCycler;

 template<unsigned int End, template<typename...> typename Target, typename T, typename... TS>
 class TemplateTypeAssembler<0, End, Target, T, TS...> {
 public:
  using type = typename TemplateTypeArgCycler<sizeof...(TS) + 1 - End, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<template<typename...> typename Target, typename T, typename... TS>
 class TemplateTypeAssembler<0, sizeof...(TS) + 1, Target, T, TS...> {
 public:
  using type = typename TemplateTypeArgCycler<0, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<unsigned int Crop, unsigned int Cycles, template<typename...> typename Target, typename T, typename... TS>
 class TemplateTypeArgCycler {
 public:
  using type = typename TemplateTypeArgCycler<Crop, Cycles - 1, Target, TS..., T>::type;
 };

 template<unsigned int Crop, template<typename...> typename Target, typename T, typename... TS>
 class TemplateTypeArgCycler<Crop, 0, Target, T, TS...> {
 public:
  using type = typename TemplateTypeAssembler<Crop, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<unsigned int Cycles, template<typename...> typename Target, typename T, typename... TS>
 class TemplateTypeArgCycler<0, Cycles, Target, T, TS...> {
 public:
  using type = typename TemplateTypeArgCycler<0, Cycles - 1, Target, TS..., T>::type;
 };

 template<template<typename...> typename Target, typename T, typename... TS>
 class TemplateTypeArgCycler<0, 0, Target, T, TS...> {
 public:
  using type = Target<T, TS...>;
 };

 //Similar to TemplateTypeArgAssembler, but for use with non-type template arguments
 template<unsigned int Start, unsigned int End, template<auto...> typename Target, auto T, auto... TS>
 class TemplateAutoAssembler {
 public:
  using type = typename TemplateAutoAssembler<Start - 1, End - 1, Target, TS...>::type;
 };

 template<unsigned int Crop, unsigned int Cycles, template<auto...> typename Target, auto T, auto... TS>
 class TemplateAutoArgCycler;

 template<unsigned int End, template<auto...> typename Target, auto T, auto... TS>
 class TemplateAutoAssembler<0, End, Target, T, TS...> {
 public:
  using type = typename TemplateAutoArgCycler<sizeof...(TS) + 1 - End, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<unsigned int Crop, unsigned int Cycles, template<auto...> typename Target, auto T, auto... TS>
 class TemplateAutoArgCycler {
 public:
  using type = typename TemplateAutoArgCycler<Crop, Cycles - 1, Target, TS..., T>::type;
 };

 template<unsigned int Crop, template<auto...> typename Target, auto T, auto... TS>
 class TemplateAutoArgCycler<Crop, 0, Target, T, TS...> {
 public:
  using type = typename TemplateAutoAssembler<Crop, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<unsigned int Cycles, template<auto...> typename Target, auto T, auto... TS>
 class TemplateAutoArgCycler<0, Cycles, Target, T, TS...> {
 public:
  using type = typename TemplateAutoArgCycler<0, Cycles - 1, Target, TS..., T>::type;
 };

 template<template<auto...> typename Target, auto T, auto... TS>
 class TemplateAutoArgCycler<0, 0, Target, T, TS...> {
 public:
  using type = Target<T, TS...>;
 };

 //SFINAE Utilities
 template<auto Match, auto Arg, typename Success, typename Failure>
 struct select_if_match_T {
 public:
  using type = Failure;
 };

 template<auto Match, typename Success, typename Failure>
 struct select_if_match_T<Match, Match, Success, Failure> {
 public:
  using type = Success;
 };

 template<auto Match, auto Arg, auto Success, auto Failure>
 struct select_if_match_A {
 public:
  inline static const auto result = Failure;
 };

 template<auto Match, auto Success, auto Failure>
 struct select_if_match_A<Match, Match, Success, Failure> {
 public:
  inline static const auto result = Success;
 };

 template<auto Match, auto Arg, typename Success, typename Failure>
 class select_if_match_M {
 public:
  using type = Failure;
 };

 template<auto Match, typename Success, typename Failure>
 class select_if_match_M<Match, Match, Success, Failure> {
 public:
  using type = Success;
 };

 //Check whether or not a type is specialized (sizeof cannot be used on incomplete types)
 template <class T, unsigned int = sizeof(T)>
 std::true_type check_specialized(T *);

 std::false_type check_specialized(...);

 template<typename T>
 using is_specialized = decltype(check_specialized(std::declval<T *>()));

 //Similar to std::is_same<typename, typename>, but enables comparisons between partially specialized or unspecialized templates
 template<template<typename...> typename T1, template<typename...> typename T2>
 class IsSame : public std::false_type {};

 template<template<typename...> typename T>
 class IsSame<T, T> : public std::true_type {};

 //Deconstructs recursive templates
 template<typename T>
 class Decompose {
 public:
  using type = T;
  inline static constexpr auto recursion_depth = 0U;
 };

 template<template<typename...> typename Target1, typename Target2>
 class _Decompose : public std::false_type {};

 template<template<typename...> typename Target1, template<typename...> typename Target2, typename T, typename... Args>
 class _Decompose<Target1, Target2<T, Args...>> : public IsSame<Target1, Target2> {};

 template<template<typename...> typename Target, typename T, typename... Args>
 class Decompose<Target<T, Args...>> {
 public:
  using type = typename Decompose<T>::type;
  inline static constexpr auto recursion_depth = Decompose<T>::recursion_depth + 1U;
  inline static constexpr bool is_homogeneous = _Decompose<Target, T>::value;
 };
}
