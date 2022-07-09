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

#Set up build flags
include(scripts/cakemake/src/flags.cmake)
add_build_flag(
 CX_ALLOW_UNSUPPORTED_COMPILER
 VALUE FALSE
 CACHE BOOL
 DESCRIPTION "Allow builds with unsupported compilers"
)

#Feature flags
add_build_flag(
 CX_DEBUG
 VALUE FALSE
 CACHE BOOL
 DESCRIPTION "Enable debug code for build"
)
add_build_flag(
 CX_QA
 VALUE FALSE
 CACHE BOOL
 DESCRIPTION "Enable building with sanitizers"
)
add_build_flag(
 CX_STL_SUPPORT
 VALUE FALSE
 CACHE BOOL
 DESCRIPTION "Enable building against STL"
)
add_build_flag(
 CX_LIBC_SUPPORT
 VALUE FALSE
 CACHE BOOL
 DESCRIPTION "Eanble building against libc"
)
add_build_flag(
 CX_ABI_SAFE
 VALUE TRUE
 CACHE BOOL
 DESCRIPTION "Disable features that are potentially ABI breaking"
)
add_build_flag(
 CX_VARARG_INTRENSICS
 VALUE TRUE
 CACHE BOOL
 DESCRIPTION
  "Enable building c-variadic argument list intrensics, enables cx/vararg>"
)
add_build_flag(
 CX_ERROR_MSG
 VALUE FALSE
 CACHE BOOL
 DESCRIPTION "Enable error messages for <cx/error>"
)
add_build_flag(
 CX_ERROR_TRACE
 VALUE FALSE
 CACHE BOOL
 DESCRIPTION "Enable error tracing for <cx/error>"
)

#Test flags
add_build_flag(
 CX_UNIT_TESTS
 VALUE TRUE
 CACHE BOOL
 DESCRIPTION "Enable building of unit tests"
)
add_build_flag(
 CX_INTEGRATION_TESTS
 VALUE TRUE
 CACHE BOOL
 DESCRIPTION "Enable building of integration tests"
)
add_build_flag(
 CX_BENCHMARKS
 VALUE TRUE
 CACHE BOOL
 DESCRIPTION "Enable building of benchmarks"
)
add_build_flag(
 CX_COVERAGE
 VALUE TRUE
 CACHE BOOL
 DESCRIPTION "Enable building with test coverage"
)

#TODO EDG support, pick up QNX support again
#Detect compiler and set up compiler-specific flags for build
include(scripts/cakemake/src/compiler.cmake)
detect_compiler(
 CX_DETECTED_COMPILER_ID
 COMPILER_ID CMAKE_CXX_COMPILER_ID
 SUPPORTED_COMPILERS NOT_A_COMPILER
  Clang
  AppleClang
  GNU
  Intel
  IntelLLVM
  MSVC
)

add_fixed_build_flag(
 CX_DETECTED_COMPILER_ID
 VALUE "${CX_DETECTED_COMPILER_ID}"
 DESCRIPTION "The detected cx compiler"
)

#Sanitize feature flags
if(CX_STL_SUPPORT AND NOT CX_LIBC_SUPPORT)
 message(
  FATAL_ERROR
  "(CX_STL_SUPPORT): Building against the STL requires libc as well"
)
endif()

if(CX_ERROR_MSG AND NOT (CX_STL_SUPPORT OR CX_LIBC_SUPPORT))
 message(
  FATAL_ERROR
  "(CX_ERROR_MSG): Error message support requires an allocator from the STL \
or libc; enable CX_STL_SUPPORT or CX_LIBC_SUPPORT"
 )
endif()

if(CX_ERROR_TRACE AND NOT (CX_STL_SUPPORT OR CX_LIBC_SUPPORT))
 message(
  FATAL_ERROR
  "(CX_ERROR_TRACE): Error backtraces requires an allocator from the STL \
or libc; enable CX_STL_SUPPORT or CX_LIBC_SUPPORT"
 )
endif()

if(CX_COVERAGE AND NOT (CX_UNIT_TESTS OR CX_INTEGRATION_TESTS OR CX_BENCHMARKS))
 message(
  WARNING
  "(CX_COVERAGE): Coverage will have no effect on build since no tests are
enabled"
 )
endif()

if(CX_QA AND NOT (CX_UNIT_TESTS OR CX_INTEGRATION_TESTS OR CX_BENCHMARKS))
 message(
  WARNING
  "(CX_QA): Sanitizers will have no effect on build since no tests are enabled"
 )
endif()

#Assemble version inline namespace and set it as preprocessor flag. This should
#prevent linking against binaries compiled with a different version of cx.
string(
 REPLACE "." "_"
 _CX_D_INLINE_NAMESPACE_VERSION
 "${CMAKE_PROJECT_VERSION}"
)
add_fixed_build_flag(
 CX_INLINE_NAMESPACE_VERSION
 VALUE "cx_${_CX_D_INLINE_NAMESPACE_VERSION}"
 DESCRIPTION "The inline namespace string for this build of cx"
)
#TODO
#add_cc_src_flags(
# "CX_INLINE_NAMESPACE_VERSION=${_CX_D_INLINE_NAMESPACE_VERSION}"
#)
unset(_CX_D_INLINE_NAMESPACE_VERSION)

#TODO Assemble unique symbol guard based on build configuration, to ensure that
# it is not possible to link against binaries compiled with
# differently-configured cx builds
string(
 APPEND _CX_SYMBOL_GUARD
 "if_you_are_seeing_this_symbol_in_a_linker_related_error_then_you_are_trying_"
 "to_link_against_another_binary_with_a_differently_configured_build_of_cx__"
 "this_is_not_allowed_as_some_cx_build_flags_may_break_abi_compatibility_"
 "between_builds_with_different_configurations__your_configuration_is_as_"
 "follows"
)
list(
 APPEND _CX_ABI_BREAKING_FLAGS
 CX_ABI_SAFE
 CX_ERROR_MSG
 CX_ERROR_TRACE
)
foreach(FLAG ${_CX_ABI_BREAKING_FLAGS})
 string(
  APPEND _CX_SYMBOL_GUARD
  "____${FLAG}__${${FLAG}}"
 )
endforeach()
unset(_CX_ABI_BREAKING_FLAGS)
#add_cc_src_flags("CX_SYMBOL_GUARD=${_CX_SYMBOL_GUARD}")
message(STATUS "GUARD: ${_CX_SYMBOL_GUARD}")
unset(_CX_SYMBOL_GUARD)

#Emit diagnostic build information
get_build_flags_pretty(CX_PRETTY_BUILD_FLAGS)
message(STATUS "${CX_PRETTY_BUILD_FLAGS}")
unset(CX_PRETTY_BUILD_FLAGS)

message(FATAL_ERROR "STOP!")
