#pragma once

//Substitutes for stl equivalents and additions for missing
//constructs
namespace CX {
 //Value producer meta-function with no external linkage; useful
 //for both SFINAE and concept meta-functions
 //Note: Can only be used in unevaluated contexts
 template<typename T>
 T&& declval() noexcept(true);

 //Value consumer meta-function with no external linkage; useful
 //for both SFINAE and concept meta-functions
 //Note: Can only be used in unevaluated contexts
 template<typename T>
 void expect(T) noexcept(true);

 //Type-pack `void` alias; useful for both SFINAE and concept
 //style meta-functions
 template<typename...>
 using VoidT = void;

 //Value-pack `void` alias; useful for both SFINAE and concept
 //style meta-functions
 template<auto...>
 using VoidA = void;

 //Meta-function truthy base
 struct TrueType {
  static constexpr auto const Value = true;
 };

 //Meta-function falsy base
 struct FalseType {
  static constexpr auto const Value = false;
 };

 //Implementation defined size type; equivalent to `std::size_t`
 using SizeType = decltype(sizeof(0));

 //Implementation defined alignment type
 using AlignType = decltype(alignof(int));

 //Implementation defined nullptr type
 using NullptrType = decltype(nullptr);

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
