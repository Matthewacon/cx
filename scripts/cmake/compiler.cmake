#[[
 This file contains all of the compiler-specific configurations required to
 build cx. Flags, defined in `flags.cmake`, are used to determine the build
 flags for the given compiler and linker here.
]]

include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../cakemake/src/compiler.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../cakemake/src/compiler-defaults.cmake)

#[[Detect compiler and set fixed build flags]]

#[[TODO
 - EDG support, pick up QNX support again
 - Add `ALLOW_UNSUPPORTED` flag if `CX_ALLOW_UNSUPPORTED_COMPILER` is specified
   Note: When this is specified, the user must also specify which compiler
   argument formatter to use, otherwise we will not know how to build the
   project
]]
#Detect compiler
detect_compiler(
 CX_DETECTED_COMPILER_ID
 COMPILER_ID CMAKE_CXX_COMPILER_ID
 SUPPORTED_COMPILERS
  Clang
  AppleClang
  GNU
  Intel
  IntelLLVM
  MSVC
)

#Add fixed build flag with compiler ID
add_fixed_build_flag(
 CX_DETECTED_COMPILER_ID
 VALUE "${CX_DETECTED_COMPILER_ID}"
 DESCRIPTION "The detected cx compiler"
)

#[[Emit diagnostic build information]]
get_build_flags_pretty(CX_PRETTY_BUILD_FLAGS)
message(STATUS "${CX_PRETTY_BUILD_FLAGS}")
unset(CX_PRETTY_BUILD_FLAGS)

#[[TODO Add all compiler/linker flags for all feature flags]]
get_supported_compilers(_CX_SUPPORTED_COMPILERS)
if(NOT CX_DETECTED_COMPILER_ID IN_LIST _CX_SUPPORTED_COMPILERS)
 message(
  FATAL_ERROR
  "Compiler '${CX_DETECTED_COMPILER_ID}' is a supported compiler!"
 )
endif()
unset(_CX_SUPPORTED_COMPILERS)

#Include the compiler backend file, for the current compiler
if(CX_DETECTED_COMPILER_ID STREQUAL "Clang")
 include(${CMAKE_CURRENT_LIST_DIR}/compiler/clang.cmake)
else()
 message(
  FATAL_ERROR
  "No CMake compiler backend exists for the compiler "
  "'${CX_DETECTED_COMPILER_ID}', cannot build compiler arguments list! You "
  "must either add support for this compiler or specify a compiler backend "
  "with the `CX_ALLOW_UNSUPPORTED_COMPILER` flag!"
 )
endif()
