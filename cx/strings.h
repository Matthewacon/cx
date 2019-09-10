#pragma once

//TODO String slicing and replacement
namespace CX {
 namespace Internal {
  constexpr unsigned strLength(const char *str, unsigned length = 0) noexcept {
   return (str[0] == '\0') ? length : strLength(str + 1, length + 1);
  }

  //Convert S1 into a char[]
  template<const char* S1, const char* S2, unsigned int I, const char... S>
  struct StringConcatenator {
//   static constexpr const auto length = strLength(S1) + strLength(S2);
   static constexpr const auto data = StringConcatenator<S1, S2, I + 1, S..., S1[I]>::data;
  };

  //Proxy template to StringConcatenator<nullptr, const char *, unsigned int, const char... S>
  template<const char* S1, const char* S2, const char... S>
  struct StringConcatenator<S1, S2, strLength(S1), S...> {
   static constexpr auto data = StringConcatenator<nullptr, S2, 0, S...>::data;
  };

  //Convert S2 into a char[] and concatenate it with the char[] representation of S1
  //from StringConcatenator<const char *, const char *, unsigned int, const char... S>
  template<const char* S2, unsigned int I, const char... S>
  struct StringConcatenator<nullptr, S2, I, S...> {
   static constexpr auto data = StringConcatenator<nullptr, S2, I + 1, S..., S2[I]>::data;
  };

  //Append a null-terminator to the end of the char[]
  template<const char* S2, const char... S>
  struct StringConcatenator<nullptr, S2, strLength(S2), S...> {
   static constexpr const char data[] = {S..., '\0'};
  };
 }

 template<const char *...>
 class Concat;

 //For use with other templates
 template<const char * S1, const char * S2, const char *... SS>
 class Concat<S1, S2, SS...> {
 public:
  static constexpr const auto result = Concat<Internal::StringConcatenator<S1, S2, 0>::data, SS...>::result;
 };

 template<const char * S>
 class Concat<S> {
 private:
  static constexpr const char empty[] = "";

 public:
  static constexpr const auto result = Internal::StringConcatenator<S, empty, 0>::data;
 };

 //For use in constexpr contexts, ie. constexpr functions
 template<const char* S1, const char* S2, const char*... SS>
 constexpr const char* concat() noexcept {
  if constexpr(sizeof...(SS) != 0) {
   return concat<Internal::StringConcatenator<S1, S2, 0>::data, SS...>();
  } else {
   return Internal::StringConcatenator<S1, S2, 0>::data;
  }
 }

// namespace Internal {
//  using test_t = const char*[];
//
//  template<const char* S1, const char* Delimiter, const char* Split[], auto...>
//  struct StringSplitter;
//
//  template<const char* S1, const char* Delimiter>
//  struct StringSplitter<S1, Delimiter> {
//   static constexpr const auto result = StringSplitter<>::result;
//  };
// }

 template<auto...>
 struct Split;

 template<const char* S1, const char* Delimiter>
 struct Split<S1, Delimiter> {

 };

 template<const char* S1, const char Delimiter>
 struct Split<S1, Delimiter> {
 private:
  static constexpr const auto delimiter = Delimiter;

 public:
//  static constexpr const auto result = Internal::StringSplitter<S1, &delimiter>::result;
 };

 template<const char* S1, const char Delimiter>
 constexpr const char* split() noexcept {
  return nullptr;
 }

 template<const char* S1, const char* Delimiter>
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