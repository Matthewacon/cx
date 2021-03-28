#pragma once

//Substitutes for stl equivalents and additions for missing
//constructs
namespace CX {
 template<typename T>
 T&& declval() noexcept(true);

 template<typename T>
 void expect(T) noexcept(true);

 template<typename...>
 using VoidT = void;

 template<auto...>
 using VoidA = void;

 struct TrueType {
  static constexpr auto const Value = true;
 };

 struct FalseType {
  static constexpr const auto Value = false;
 };

 //Converts type and value meta-functions to stl-compatible equivalents
 namespace MetaFunctions {
  template<typename T>
  struct AsStlCompatible;

  //Type meta-function specialization
  template<typename T>
  requires (requires (VoidT<typename T::Type> * a) { a = nullptr; })
  struct AsStlCompatible<T> {
   using type = typename T::Type;
  };

  //Value meta-function specialization
  template<typename T>
  requires (requires { expect(&T::Value); })
  struct AsStlCompatible<T> {
   static constexpr auto const value = T::Value;
  };
 }

 template<typename T>
 using AsStlCompatible = MetaFunctions::AsStlCompatible<T>;

 //General purpose dummy template class for nested template argument deduction
 template<typename...>
 struct Dummy {};

 //Unique internal type with no-linkage, used for template meta-functions
 template<typename...>
 struct ImpossibleType;

 //Unique internal type with no-linkage, used for template meta-functions
 template<template<typename...> typename...>
 struct ImpossibleTemplateType;

 //Unique internal type with no-linkage, used for template meta-functions
 template<auto...>
 struct ImpossibleValueType;
}
