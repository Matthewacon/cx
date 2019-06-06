#pragma once

//TODO String slicing and replacement
namespace CX {
 constexpr unsigned strLength(const char *str, unsigned length = 0) noexcept {
  return (str[0] == '\0') ? length : strLength(str + 1, length + 1);
 }

 template<const char* S1, const char* S2, unsigned int I, const char... S>
 class StringConcatenator {
 public:
  inline static constexpr const auto length = strLength(S1) + strLength(S2);
  inline static constexpr const auto data = StringConcatenator<S1, S2, I + 1, S..., S1[I]>::data;
 };

 template<const char* S1, const char* S2, const char... S>
 class StringConcatenator<S1, S2, strLength(S1), S...> {
 public:
  inline static constexpr auto data = StringConcatenator<nullptr, S2, 0, S...>::data;
 };

 template<const char* S2, unsigned int I, const char... S>
 class StringConcatenator<nullptr, S2, I, S...> {
 public:
  inline static constexpr auto data = StringConcatenator<nullptr, S2, I + 1, S..., S2[I]>::data;
 };

 template<const char* S2, const char... S>
 class StringConcatenator<nullptr, S2, strLength(S2), S...> {
 public:
  inline static constexpr const char data[] = {S..., '\0'};
 };

 template<const char* S1, const char* S2, const char*... SS>
 constexpr const char* concat() noexcept {
  if constexpr(sizeof...(SS) != 0) {
   return concat<StringConcatenator<S1, S2, 0>::data, SS...>();
  } else {
   return StringConcatenator<S1, S2, 0>::data;
  }
 }

 //TODO
 template<const char* S1, const char S2>
 constexpr const char* split() noexcept {
  return nullptr;
 }

 //TODO implement all specifiers from printf
 //Only supports %s
 template<const char* Format, const char*... SS>
 constexpr const char* format() noexcept {
  return nullptr;
 }
}