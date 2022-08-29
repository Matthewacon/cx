#[[
 This file maps all of the cx build flags to compiler arguments for Clang.

 Note: For feature flag descriptions, see: `../flags.cmake`.
]]

include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/../flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../cakemake/src/compiler.cmake)

#[[Defaults]]
add_cc_or_ld_arguments(
 COMPILER
 ARGUMENTS
  -Wall
  -Werror
  -Wextra
  -pedantic
  -Wno-zero-length-array
  -ftemplate-backtrace-limit=0
  -fno-common
  -fno-exceptions
)

#[[Feature flags]]
if(CX_PRE_COMPILED_HEADERS)
 message(
  WARNING
  "The Clang compiler backend for cx does not yet support pre-compiled"
  "headers!"
 )
endif()
mark_build_flag_as_processed(CX_PRE_COMPILED_HEADERS)

#`CX_DEBUG`
if(CX_DEBUG)
 set(CMAKE_BUILD_TYPE Debug)
 add_compiler_define(CX_DEBUG 1)
 add_cc_or_ld_arguments_for_build_flag(
  COMPILER
  CX_DEBUG
  ARGUMENTS
   -g
   -O0
   -fno-inline
   -fno-inline-functions
   -fno-omit-frame-pointer
   -fno-lto
   -fstandalone-debug
 )
endif()
mark_build_flag_as_processed(CX_DEBUG)

#`CX_QA`
if(CX_QA)
 add_compiler_define(CX_QA 1)
 add_cc_or_ld_arguments_for_build_flag(
  COMPILER
  CX_QA
  ARGUMENTS
   -fno-omit-frame-pointer
   -fno-sanitize-recover=null
   -fsanitize=address
   -fsanitize=undefined
   -fsanitize-address-use-after-scope
   -fsanitize=bounds
   -fsanitize=null
 )
endif()
mark_build_flag_as_processed(CX_QA)

#`CX_STL_SUPPORT`
if(CX_STL_SUPPORT)
 add_compiler_define(CX_STL_SUPPORT 1)
else()
 add_cc_or_ld_arguments_for_build_flag(
  COMPILER
  CX_STL_SUPPORT
  ARGUMENTS
   -nostdlib++
 )
endif()
mark_build_flag_as_processed(CX_STL_SUPPORT)

#`CX_LIBC_SUPPORT`
if(CX_LIBC_SUPPORT)
 add_compiler_define(CX_LIBC_SUPPORT 1)
else()
 add_cc_or_ld_arguments_for_build_flag(
  COMPILER
  CX_LIBC_SUPPORT
  ARGUMENTS
   -nostdlib
 )
endif()
mark_build_flag_as_processed(CX_LIBC_SUPPORT)

#`CX_ABI_SAFE`
if(CX_ABI_SAFE)
 add_compiler_define(CX_ABI_SAFE 1)
endif()
mark_build_flag_as_processed(CX_ABI_SAFE)

#`CX_VARARG_INTRENSICS`
if(CX_VARARG_INTRENSICS)
 add_compiler_define(CX_VARARG_INTRENSICS 1)
endif()
mark_build_flag_as_processed(CX_VARARG_INTRENSICS)

#`CX_ERROR_MSG`
if(CX_ERROR_MSG)
 add_compiler_define(CX_ERROR_MSG 1)
endif()
mark_build_flag_as_processed(CX_ERROR_MSG)

#`CX_ERROR_TRACE`
if(CX_ERROR_TRACE)
 add_compiler_define(CX_ERROR_TRACE 1)
endif()
mark_build_flag_as_processed(CX_ERROR_TRACE)

#`CX_COVERAGE`
if(CX_COVERAGE)
 message(WARNING "Coverage is currently unimplemented!")
endif()
mark_build_flag_as_processed(CX_COVERAGE)

#`[CX_INLINE_NAMESPACE_VERSION]`
add_compiler_define(
 CX_INLINE_NAMESPACE_VERSION
 "${CX_INLINE_NAMESPACE_VERSION}"
)
mark_build_flag_as_processed(CX_INLINE_NAMESPACE_VERSION)

#`[CX_GUARD_SYMBOL]`
add_compiler_define(
 CX_GUARD_SYMBOL
 "${CX_GUARD_SYMBOL}"
)
mark_build_flag_as_processed(CX_GUARD_SYMBOL)

#`[CX_DETECTED_COMPILER_ID]`
add_compiler_define(
 CX_DETECTED_COMPILER_ID
 "${CX_DETECTED_COMPILER_ID}"
)
mark_build_flag_as_processed(CX_DETECTED_COMPILER_ID)
