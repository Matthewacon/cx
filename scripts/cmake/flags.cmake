#[[This file contains all of the build flags supported by cx.]]

include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/../cakemake/src/flags.cmake)

#[[Danger zone]]
add_build_flag(
 CX_ALLOW_UNSUPPORTED_COMPILER
 VALUE FALSE
 CACHE BOOL
 DESCRIPTION "Allow builds with unsupported compilers"
)

add_build_flag(
 CX_PRE_COMPILED_HEADERS
 VALUE FALSE
 CACHE BOOL
 DESCRIPTION "Precompile headers and reuse outputs, where possible"
)

#[[Feature flags]]
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
  "Enable building c-variadic argument list intrensics, enables <cx/vararg>"
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

#[[Test flags]]
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

#[[Flag validation]]
if(CX_STL_SUPPORT AND NOT CX_LIBC_SUPPORT)
 message(
  FATAL_ERROR
  "(CX_STL_SUPPORT): Building against the STL requires libc as well"
)
endif()

if(CX_ERROR_MSG AND NOT (CX_STL_SUPPORT OR CX_LIBC_SUPPORT))
 message(
  FATAL_ERROR
  "(CX_ERROR_MSG): Error message support requires an allocator from the STL "
  "or libc; enable CX_STL_SUPPORT or CX_LIBC_SUPPORT"
 )
endif()

if(CX_ERROR_TRACE AND NOT (CX_STL_SUPPORT OR CX_LIBC_SUPPORT))
 message(
  FATAL_ERROR
  "(CX_ERROR_TRACE): Error backtraces requires an allocator from the STL "
  "or libc; enable CX_STL_SUPPORT or CX_LIBC_SUPPORT"
 )
endif()

if(CX_COVERAGE AND NOT (CX_UNIT_TESTS OR CX_INTEGRATION_TESTS OR CX_BENCHMARKS))
 message(
  WARNING
  "(CX_COVERAGE): Coverage will have no effect on build since no tests are "
  "enabled"
 )
endif()

if(CX_QA AND NOT (CX_UNIT_TESTS OR CX_INTEGRATION_TESTS OR CX_BENCHMARKS))
 message(
  WARNING
  "(CX_QA): Sanitizers will have no effect on build since no tests are enabled"
 )
endif()
