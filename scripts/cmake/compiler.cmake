#[[
 This file contains all of the compiler-specific configurations required to
 build cx. Flags, defined in `flags.cmake`, are used to determine the build
 flags for the given compiler and linker here.
]]

include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../cakemake/src/compiler.cmake)

#[[Detect compiler and set fixed build flags]]

#[[TODO
 - EDG support, pick up QNX support again
 - Add `ALLOW_UNSUPPORTED` flag if `CX_ALLOW_UNSUPPORTED_COMPILER` is specified
]]
#Detect compiler
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

#Add fixed build flag with compiler ID
add_fixed_build_flag(
 CX_DETECTED_COMPILER_ID
 VALUE "${CX_DETECTED_COMPILER_ID}"
 DESCRIPTION "The detected cx compiler"
)

#Generate inline namespace version and add fixed build flag
generate_inline_namespace(_CX_INLINE_NAMESPACE_VERSION)
add_fixed_build_flag(
 CX_INLINE_NAMESPACE_VERSION
 VALUE "${_CX_INLINE_NAMESPACE_VERSION}"
 DESCRIPTION "The inline namespace string for this build of cx"
)
unset(_CX_INLINE_NAMESPACE_VERSION)

#Generate guard symbol and add fixed build flag
generate_guard_symbol(
 _CX_GUARD_SYMBOL
 ABI_BREAKING_FLAGS
  CX_ABI_SAFE
  CX_STL_SUPPORT
  CX_LIBC_SUPPORT
  CX_ERROR_MSG
  CX_ERROR_TRACE
)
add_fixed_build_flag(
 CX_GUARD_SYMBOL
 VALUE "${_CX_GUARD_SYMBOL}"
 DESCRIPTION
  "The name of the guard symbol to prevent users from linking against "
  "ABI-incompatibile builds of cx"
)
unset(_CX_GUARD_SYMBOL)

#[[Emit diagnostic build information]]
get_build_flags_pretty(CX_PRETTY_BUILD_FLAGS)
message(STATUS "${CX_PRETTY_BUILD_FLAGS}")
unset(CX_PRETTY_BUILD_FLAGS)

#[[TODO Add all compiler/linker flags for all feature flags]]
