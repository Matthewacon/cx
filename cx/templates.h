#pragma once

#include "cx/common.h"

namespace CX {
 //TODO for all template related tools, add parameter separation for construction
 //Produces the type at 'N' in the vararg list 'T, (TS...)'
 template<unsigned int N, typename T, typename... TS>
 struct TemplateTypeIterator {
  using type = typename TemplateTypeIterator<N - 1, TS...>::type;
 };

 template<typename T, typename... TS>
 struct TemplateTypeIterator<0, T, TS...> {
  using type = T;
 };

 //Produces the value at 'N' in the vararg list 'T, (TS...)'
 template<unsigned int N, auto T, auto... TS>
 struct TemplateAutoIterator {
  inline static constexpr const auto value = TemplateAutoIterator<N - 1, TS...>::value;
 };

 template<auto T, auto... TS>
 struct TemplateAutoIterator<0, T, TS...> {
  inline static constexpr const auto value = T;
 };

 //Produces a specialized Target<...> with the template parameters from 'Start' to 'End' inclusive
 template<unsigned int Start, unsigned int End, template<typename...> typename Target, typename T, typename... TS>
 struct TemplateTypeAssembler {
  using type = typename TemplateTypeAssembler<Start - 1, End - 1, Target, TS...>::type;
 };

 template<unsigned int Crop, unsigned int Cycles, template<typename...> typename Target, typename T, typename... TS>
 struct TemplateTypeArgCycler;

 template<unsigned int End, template<typename...> typename Target, typename T, typename... TS>
 struct TemplateTypeAssembler<0, End, Target, T, TS...> {
  using type = typename TemplateTypeArgCycler<sizeof...(TS) + 1 - End, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<template<typename...> typename Target, typename T, typename... TS>
 struct TemplateTypeAssembler<0, sizeof...(TS) + 1, Target, T, TS...> {
  using type = typename TemplateTypeArgCycler<0, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<unsigned int Crop, unsigned int Cycles, template<typename...> typename Target, typename T, typename... TS>
 struct TemplateTypeArgCycler {
  using type = typename TemplateTypeArgCycler<Crop, Cycles - 1, Target, TS..., T>::type;
 };

 template<unsigned int Crop, template<typename...> typename Target, typename T, typename... TS>
 struct TemplateTypeArgCycler<Crop, 0, Target, T, TS...> {
  using type = typename TemplateTypeAssembler<Crop, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<unsigned int Cycles, template<typename...> typename Target, typename T, typename... TS>
 struct TemplateTypeArgCycler<0, Cycles, Target, T, TS...> {
  using type = typename TemplateTypeArgCycler<0, Cycles - 1, Target, TS..., T>::type;
 };

 template<template<typename...> typename Target, typename T, typename... TS>
 struct TemplateTypeArgCycler<0, 0, Target, T, TS...> {
  using type = Target<T, TS...>;
 };

 //Similar to TemplateTypeArgAssembler, but for use with non-type integral template arguments
 template<unsigned int Start, unsigned int End, template<auto...> typename Target, auto T, auto... TS>
 struct TemplateAutoAssembler {
  using type = typename TemplateAutoAssembler<Start - 1, End - 1, Target, TS...>::type;
 };

 template<unsigned int Crop, unsigned int Cycles, template<auto...> typename Target, auto T, auto... TS>
 struct TemplateAutoArgCycler;

 template<unsigned int End, template<auto...> typename Target, auto T, auto... TS>
 struct TemplateAutoAssembler<0, End, Target, T, TS...> {
  using type = typename TemplateAutoArgCycler<sizeof...(TS) + 1 - End, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<unsigned int Crop, unsigned int Cycles, template<auto...> typename Target, auto T, auto... TS>
 struct TemplateAutoArgCycler {
  using type = typename TemplateAutoArgCycler<Crop, Cycles - 1, Target, TS..., T>::type;
 };

 template<unsigned int Crop, template<auto...> typename Target, auto T, auto... TS>
 struct TemplateAutoArgCycler<Crop, 0, Target, T, TS...> {
  using type = typename TemplateAutoAssembler<Crop, sizeof...(TS) + 1, Target, T, TS...>::type;
 };

 template<unsigned int Cycles, template<auto...> typename Target, auto T, auto... TS>
 struct TemplateAutoArgCycler<0, Cycles, Target, T, TS...> {
  using type = typename TemplateAutoArgCycler<0, Cycles - 1, Target, TS..., T>::type;
 };

 template<template<auto...> typename Target, auto T, auto... TS>
 struct TemplateAutoArgCycler<0, 0, Target, T, TS...> {
  using type = Target<T, TS...>;
 };

 //SFINAE Utilities
 template<auto Match, auto Arg, typename Success, typename Failure>
 struct select_if_match_T {
  using type = Failure;
 };

 template<auto Match, typename Success, typename Failure>
 struct select_if_match_T<Match, Match, Success, Failure> {
  using type = Success;
 };

 template<auto Match, auto Arg, auto Success, auto Failure>
 struct select_if_match_A {
  inline static constexpr const auto result = Failure;
 };

 template<auto Match, auto Success, auto Failure>
 struct select_if_match_A<Match, Match, Success, Failure> {
  inline static constexpr const auto result = Success;
 };

 template<bool, typename Success, typename Failure>
 struct select_if_true : false_type {
  using type = Failure;
 };

 template<typename Success, typename Failure>
 struct select_if_true<true, Success, Failure> : true_type {
  using type = Success;
 };
}
