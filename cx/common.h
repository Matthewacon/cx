#pragma once

//Substitutes for stl equivalents
namespace CX {
 template<typename T>
 T&& declval();

 template<typename...>
 using void_t = void;

 struct true_type {
  static constexpr const auto value = true;
 };

 struct false_type {
  static constexpr const auto value = false;
 };

 //General purpose dummy template class for nested template argument deduction
 template<typename...>
 class _Dummy {};
}
