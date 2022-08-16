cmake_minimum_required(VERSION 3.19)

#Ensure we generate `compile_commands.json` when configuring builds
set(ENV{CMAKE_EXPORT_COMPILE_COMMANDS} ON)

project(
 cx
 VERSION 0.1.0
 LANGUAGES CXX
 DESCRIPTION "TBD"
)

#Required defaults
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

#Explicitly disallow in-source builds
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
 message(
  FATAL_ERROR
  "In-source builds are not permitted."
 )
endif()

#Set up project feature flags
include(scripts/cmake/flags.cmake)

#Set up compiler and linker flags
include(scripts/cmake/compiler.cmake)

#TODO Set up build
#TODO Set up tests

