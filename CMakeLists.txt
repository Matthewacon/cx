cmake_minimum_required(VERSION 3.8)
project(cx)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")

#set(SOURCE_FILES include/array.h include/strings.h include/templates.h include/tuple.h)
set(SOURCE_FILES src/main.cpp)
include_directories(include/)
#add_library(cx SHARED ${SOURCE_FILES})
add_executable(cx ${SOURCE_FILES})