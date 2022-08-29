cmake_minimum_required(VERSION 3.19)

#[[Ensure we generate `compile_commands.json` when configuring builds]]
set(ENV{CMAKE_EXPORT_COMPILE_COMMANDS} ON)

#[[Set up project]]
project(
 cx
 VERSION 0.1.0
 LANGUAGES CXX
 DESCRIPTION "TBD"
)

#[[Required defaults]]
#TODO Make configurable with cx build flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

#Explicitly disallow in-source builds
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
 message(
  FATAL_ERROR
  "In-source builds are not permitted."
 )
endif()

#[[Set up project feature flags]]
include(scripts/cmake/flags.cmake)

#[[Set up compiler and linker flags]]
include(scripts/cmake/compiler.cmake)

#[[
 Declare target

 Note: Done before including tests, so that they may depend on the library
 target.
]]
add_library(cx INTERFACE)

#[[Set up tests, if enabled]]
include(scripts/cmake/tests.cmake)

#[[Ensure all feature flags have been processed]]
assert_all_build_flags_processed()

#[[Set up build]]
target_include_directories(cx INTERFACE include)

#Add all compiler flags
get_cc_or_ld_arguments(COMPILER _CX_COMPILER_ARGUMENTS)
foreach(ARG ${_CX_COMPILER_ARGUMENTS})
 target_compile_options(cx INTERFACE "${ARG}")
endforeach()
unset(_CX_COMPILER_ARGUMENTS)

#Add all preprocessor defines
get_compiler_defines(_CX_PREPROCESSOR_DEFINES)
foreach(ARG ${_CX_PREPROCESSOR_DEFINES})
 get_formatted_compiler_define("${ARG}" _FORMATTED)
 target_compile_options(cx INTERFACE "${_FORMATTED}")
endforeach()
unset(_FORMATTED)
unset(_CX_PREPROCESSOR_DEFINES)

#Add all linker flags
get_cc_or_ld_arguments(LINKER _CX_LINKER_ARGUMENTS)
foreach(ARG ${_CX_LINKER_ARGUMENTS})
 target_link_options(cx INTERFACE "${ARG}")
endforeach()
unset(_CX_LINKER_ARGUMENTS)
