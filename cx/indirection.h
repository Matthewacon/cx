#pragma once

#include "cx/common.h"
#include "cx/templates.h"
#include "cx/idioms.h"

namespace CX {
 //Will strip the pointers, references and const qualifications off of a type argument, to reveal the component type
 //TODO if anybody asks for individual counters for reference and pointer type qualifications, implement it
 namespace Internal {
  //N tracks the indirection count
  template<auto C, auto I, typename T>
  class ComponentTypeResolver {
  public:
   using type = T;
   inline static constexpr const auto constCount = C;
   inline static constexpr const auto indirectionCount = I;
  };

  template<auto C, auto I, typename T>
  class ComponentTypeResolver<C, I, T[]> {
  private:
   using resolver = ComponentTypeResolver<C, I + 1U, T>;

  public:
   using type = typename resolver::type;
   static constexpr const auto constCount = resolver::constCount;
   static constexpr const auto indirectionCount = resolver::indirectionCount;
  };

  template<auto C, auto I, typename T>
  class ComponentTypeResolver<C, I, T*> {
  private:
   using resolver = ComponentTypeResolver<C, I + 1U, T>;

  public:
   using type = typename resolver::type;
   inline static constexpr const auto constCount = resolver::constCount;
   inline static constexpr const auto indirectionCount = resolver::indirectionCount;
  };

  template<auto C, auto I, typename T>
  class ComponentTypeResolver<C, I, T&> {
  private:
   using resolver = ComponentTypeResolver<C, I + 1U, T>;

  public:
   using type = typename resolver::type;
   inline static constexpr const auto constCount = resolver::constCount;
   inline static constexpr const auto indirectionCount = resolver::indirectionCount;
  };

  template<auto C, auto I, typename T>
  class ComponentTypeResolver<C, I, const T> {
  private:
   //Const qualifications are not added to the indirection count
   using resolver = ComponentTypeResolver<C + 1U, I, T>;

  public:
   using type = typename resolver::type;
   inline static constexpr const auto constCount = resolver::constCount;
   inline static constexpr const auto indirectionCount = resolver::indirectionCount;
  };
 }

 template<typename T>
 class ComponentTypeResolver {
 private:
  using resolver = Internal::ComponentTypeResolver<0U, 0U, T>;

 public:
  using type = typename resolver::type;
  inline static constexpr const auto constCount = resolver::constCount;
  inline static constexpr const auto indirectionCount = resolver::indirectionCount;
 };

 //Produces a pointer type, with N levels of indirection
 template<typename Type, auto N>
 struct AppendPointer {
  using type = typename AppendPointer<Type *, N - 1>::type;
 };

 template<typename Type>
 struct AppendPointer<Type, 0U> {
  using type = Type;
 };

 //Produces a reference type, with N levels of indirection
 template<typename Type, auto N>
 struct AppendReference {
  using type = typename AppendReference<Type &, N - 1>::type;
 };

 template<typename Type>
 struct AppendReference<Type, 0U> {
  using type = Type;
 };

 //Deconstructs recursive templates
 namespace Internal {
  template<template<typename...> typename Target1, typename Target2>
  struct Decompose : false_type {};

  template<template<typename...> typename Target1, template<typename...> typename Target2, typename T, typename... Args>
  struct Decompose<Target1, Target2<T, Args...>> : TemplateIsSame<Target1, Target2> {};
 }

 template<typename T>
 struct Decompose {
  using type = T;
  inline static constexpr auto recursion_depth = 0U;
 };

 //CX API
 template<template<typename...> typename Target, typename T, typename... Args>
 struct Decompose<Target<T, Args...>> {
  using type = typename Decompose<T>::type;
  inline static constexpr const auto recursion_depth = Decompose<T>::recursion_depth + 1U;
  inline static constexpr const bool is_homogeneous = Internal::Decompose<Target, T>::value;
 };
}