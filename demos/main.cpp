#include <iostream>
#include <vector>

//TODO Move each demo into its own test suite
//TODO Write demos for the remaining tools in 'templaets.h'

//Tuple Demo
#include "tuple.h"
static constexpr void func1() {}
static constexpr int func2(int i) { return i; }
static constexpr const char * func3() { return "Goodbye world!"; }

void tuple() {
 constexpr CX::Tuple<int, unsigned int, char, const char *> test(-1, 1, 'c', "Hello World!");
 constexpr CX::Tuple<void (*)(), int (*)(int), const char *(*)()> test2(func1, func2, func3);
 constexpr unsigned int bruh = test.get<1>();
}

//Template Decomposer Demo
#include "templates.h"

using TestType = std::vector<std::vector<std::vector<int>>>;

void decompose() {
 constexpr auto template_depth = CX::Decompose<TestType>::recursion_depth;
 std::cout << template_depth << std::endl;
 std::cout << typeid(template_depth).name() << std::endl;
 std::cout << true << std::endl;
 std::cout << CX::Decompose<TestType>::is_homogeneous << std::endl;
 std::cout << typeid(CX::Decompose<TestType>).name() << std::endl;
 std::cout << typeid(CX::Decompose<TestType>::type).name() << std::endl;
}

//Strings Demo
#include "strings.h"

constexpr const char
 str1[] = "Hello ",
 str2[] = "World!",
 str3[] = "\nGoodbye ",
 str4[] = "World!";

using Concatenated = CX::StringConcatenator<str1, str2, 0>;
constexpr const char* concatenated = CX::concat<str1, str2, str3, str4, str4, str4, str4>();

void string() {
 std::cout << Concatenated::length << std::endl;
 std::cout << typeid(Concatenated::length).name() << std::endl;
 std::cout << Concatenated::data << std::endl;
}