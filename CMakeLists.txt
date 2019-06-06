cmake_minimum_required(VERSION 3.8)
project(cx)

SET(CMAKE_CXX_STANDARD 17)
#SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")

add_library(cx INTERFACE)
target_include_directories(cx INTERFACE cx)
target_compile_options(cx INTERFACE -Werror -Wall -Wextra -pendantic -Wno-unused-parameter)
