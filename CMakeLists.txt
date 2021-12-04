cmake_minimum_required(VERSION 3.19)

project(cx VERSION 0.1.0)

include(scripts/cmake-utils/cmake-fetch.cmake)

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

#TODO Add support for clang-cl (and dpc++-cl?)
#Display error if compiler is not clang, gcc, icc/icx or msvc
list(
 APPEND CX_SUPPORTED_COMPILERS
 "Clang"
 "AppleClang"
 "GNU"
 "Intel"
 "IntelLLVM"
 "MSVC"
)
set(_CX_SUPPORTED_COMPILER_DETECTED FALSE)
foreach(COMPILER ${CX_SUPPORTED_COMPILERS})
 if(
  CMAKE_C_COMPILER_ID STREQUAL COMPILER
  AND CMAKE_CXX_COMPILER_ID STREQUAL COMPILER
 )
  set(_CX_SUPPORTED_COMPILER_DETECTED TRUE)
  set(CX_DETECTED_COMPILER_ID "${COMPILER}")
 endif()
endforeach()
if(NOT ${_CX_SUPPORTED_COMPILER_DETECTED})
 message(
  FATAL_ERROR
  "You are using an unsupported compiler, please build with any of:\
  ${CX_SUPPORTED_COMPILERS}\
  "
 )
endif()

#List of supported clang-like compilers
list(
 APPEND CX_CLANG_LIKE_COMPILERS
 "Clang"
 "AppleClang"
 "IntelLLVM"
)

#TODO Handle passing arguments on different platforms (*nix, windows, etc)
#Sets the supplied argument for the current compiler
function(add_cc_or_ld_argument)
 #Parse and check arguments
 cmake_parse_arguments(
  aca
  "COMPILER;LINKER"
  ""
  "${CX_SUPPORTED_COMPILERS}"
  ${ARGN}
 )

 #Sanitize `aca_COMPILER` and `aca_LINKER` flags
 if((NOT aca_COMPILER AND NOT aca_LINKER) OR (aca_COMPILER AND aca_LINKER))
  message(
   FATAL_ERROR
   "You must specifiy either `COMPILER` or `LINKER` for the arguments that you\
   want to append\
   "
  )
 endif()

 #Set arguemnt target var
 if(aca_COMPILER)
  set(aca_TARGET_VAR "CX_COMPILE_FLAGS")
 elseif(aca_LINKER)
  set(aca_TARGET_VAR "CX_LINK_FLAGS")
 endif()

 #Add arguments for target compiler
 foreach(argument ${aca_${CX_DETECTED_COMPILER_ID}})
  list(
   APPEND ${aca_TARGET_VAR}
   "${argument}"
  )
 endforeach()

 #Propagate changes outside function
 set(${aca_TARGET_VAR} "${${aca_TARGET_VAR}}" PARENT_SCOPE)
endfunction()

#Add a source preprocessor define through the compiler cmdline
function(add_cc_src_flags)
 foreach(flag ${ARGN})
  add_cc_or_ld_argument(
   COMPILER
   Clang "-D${flag}"
   AppleClang "-D${flag}"
   GNU "-D${flag}"
   #TODO Intel
   IntelLLVM "-D${flag}"
   MSVC "/D${flag}"
  )
 endforeach()

 #Propagate `CX_COMPILE_FLAGS` changes outside function
 set(CX_COMPILE_FLAGS "${CX_COMPILE_FLAGS}" PARENT_SCOPE)
endfunction()

#Determine whether or not this is the top-level project
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}")
 set(CX_TOP_LEVEL TRUE)
else()
 set(CX_TOP_LEVEL FALSE)
endif()

#All CX build flags
set(CX_DEBUG OFF CACHE BOOL "Enable CX debug building")
set(CX_QA OFF CACHE BOOL "Builds with sanitizers")
set(CX_HEADER_ONLY ON CACHE BOOL "Builds CX as a header-only library")
set(CX_UNIT_TESTS OFF CACHE BOOL "Builds CX unit tests")
set(CX_BENCHMARKS OFF CACHE BOOL "Builds CX benchmarks")
set(CX_COVERAGE OFF CACHE BOOL "Enable CX coverage profiling")

#Library support flags
set(CX_STL_SUPPORT OFF CACHE BOOL "Enable CX->stl integration")
set(CX_LIBC_SUPPORT ON CACHE BOOL "Enable CX->libc integration")

#Behavioural flags
set(CX_VARIANT_HARD_CLEAR OFF CACHE BOOL
 "Enable clearing of variant cache before initialization and after destruction"
)
set(CX_VARARG_INTRENSICS OFF CACHE BOOL
 "Enable definition of va_list function intrensics in 'cx/vararg.h', that\
 would otherwise be defined by '<cstdarg>'\
 "
)
set(CX_ERROR_TRACE OFF CACHE BOOL "Enable error tracing")
set(CX_ERROR_MSG OFF CACHE BOOL "Enable error messages")
set(CX_CONSTEXPR_SEMANTICS OFF CACHE BOOL
 "Enables all constexpr functionality at the cost of space overhead"
)

#Sanitize error behaviour flags
if(NOT CX_STL_SUPPORT)
 if(CX_ERROR_TRACE)
  message(
   FATAL_ERROR
   "`CX_ERROR_TRACE` requires STL support"
  )
 endif()
endif()

#Sanitize coverage flag
if(
 ${CX_COVERAGE}
 AND NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_COMPILER_ID STREQUAL "Clang"))
 message(
  FATAL_ERROR
  "'CX_COVERAGE' can only be used when building with Clang"
 )
endif()

#Sanitize header-only flag
if(NOT ${CX_HEADER_ONLY})
 message(
  FATAL_ERROR
  "CX is currently a header-only library"
 )
endif()

#Check for clang-like compiler
if(CX_DETECTED_COMPILER_ID IN_LIST CX_CLANG_LIKE_COMPILERS)
 set(CX_COMPILER_CLANG_LIKE TRUE)
endif()

#Set default compiler flags and compiler id flags
add_cc_or_ld_argument(
 COMPILER
 Clang
  -DCX_COMPILER_CLANG
  -DCX_COMPILER_CLANG_LIKE
  -Wall
  -Wextra
  -Werror
  -pedantic
  -ftemplate-backtrace-limit=0
  -fno-common
  -Wno-zero-length-array
 AppleClang
  -DCX_COMPILER_APPLE
  -DCX_COMPILER_CLANG_LIKE
  -Wall
  -Wextra
  -Werror
  -pedantic
  -ftemplate-backtrace-limit=0
  -fno-common
  -Wno-zero-length-array
 GNU
  -DCX_COMPILER_GCC
 Intel
  -DCX_COMPILER_INTEL
 IntelLLVM
  -DCX_COMPILER_INTEL_LLVM
  -DCX_COMPILER_CLANG_LIKE
 MSVC
  /DCX_COMPILER_MSVC
  /std:c++20
  #TODO These flags are leaked to gtest when compiling against it
  #and cause issues
  #/Wall
  #/WX
  /D_CRT_NO_VA_START_VALIDATION
)

#Error on compilers pending official support
list(
 APPEND CX_COMPILERS_AWAITING_SUPPORT
 Intel
 MSVC
)
if(CX_DETECTED_COMPILER_ID IN_LIST CX_COMPILERS_AWAITING_SUPPORT)
 message(
  FATAL_ERROR
  "'${CX_DETECTED_COMPILER_ID}' is currently awaiting official support."
 )
endif()

#Conditional compile flags
#Set the `CX_STL_SUPPORT` preprocessor flag or disable stl support
if(${CX_STL_SUPPORT})
 #Add source flag for stl integration
 add_cc_src_flags(CX_STL_SUPPORT)
else()
 #Disable compiling / linking against stl lib
 add_cc_or_ld_argument(
  COMPILER
  Clang
   -nostdlib++
   -fno-exceptions
  AppleClang
   -nostdlib++
   -fno-exceptions
  GNU
   -nostdlib++
   -fno-exceptions
  #TODO Intel
  MSVC
   /EHsc
 )

 #Disable linking against stl libs (MSVC-specific)
 add_cc_or_ld_argument(
  LINKER
  MSVC
   /NODEFAULTLIB:libcpmt.lib
   /NODEFAULTLIB:msvcprt.lib
   /NODEFAULTLIB:libcpmtd.lib
   /NODEFAULTLIB:msvcprtd.lib
 )
endif()

#Set the `CX_LIBC_SUPPORT` preprocessor flag or disable libc support
if(${CX_LIBC_SUPPORT})
 #Add source flag for libc integration
 add_cc_src_flags(CX_LIBC_SUPPORT)
else()
 #Disable linking against platform libc equivalent (uCRT on Windows)
 add_cc_or_ld_argument(
  COMPILER
  Clang
   -nostdlib
  AppleClang
   -nostdlib
  GNU
   -nostdlib
  #TODO Intel
  MSVC
   /NODEFAULTLIB
   /GS-
   #/MT
   /clr:nostdlib
   /Zl
 )

 #Disable linking against any libc versions (MSVC specific)
 add_cc_or_ld_argument(
  LINKER
  MSVC
   /NODEFAULTLIB:libucrt.lib
   /NODEFAULTLIB:libucrtd.lib
   /NODEFAULTLIB:ucrt.lib
   /NODEFAULTLIB:ucrtd.lib
   /NODEFAULTLIB:libvcruntime.lib
   /NODEFAULTLIB:libvcruntimed.lib
   /NODEFAULTLIB:vcruntime.lib
   /NODEFAULTLIB:vcruntimed.lib
   /NODEFAULTLIB:libcmt.lib
   /NODEFAULTLIB:libcmtd.lib
   /NODEFAULTLIB:msvcrt.lib
   /NODEFAULTLIB:msvcrtd.lib
   /NODEFAULTLIB:msvcmrt.lib
   /NODEFAULTLIB:msvcmrtd.lib
   /NODEFAULTLIB:msvcurt.lib
   /NODEFAULTLIB:msvcurtd.lib
 )
endif()

#TODO remove since variant does not support this behaviour anymore
#Set the `CX_VARIANT_HARD_CLEAR` preprocessor flag
if(${CX_VARIANT_HARD_CLEAR})
 add_cc_src_flags(CX_VARIANT_HARD_CLEAR)
endif()

#Set the `CX_VARARG_INTRENSICS` preprocessor flag
if(${CX_VARARG_INTRENSICS})
 add_cc_src_flags(CX_VARARG_INTRENSICS)
endif()

#Set `CX_ERROR_TRACE` preprocessor flag
if(${CX_ERROR_TRACE})
 add_cc_src_flags(CX_ERROR_TRACE)
endif()

#Set `CX_ERROR_MSG` preprocessor flag
if(${CX_ERROR_MSG})
 add_cc_src_flags(CX_ERROR_MSG)
endif()

#Set the `CX_CONSTEXPR_SEMANTICS` preprocessor flag
if(${CX_CONSTEXPR_SEMANTICS})
 add_cc_src_flags(CX_CONSTEXPR_SEMANTICS)
endif()

#Set the `CX_DEBUG` preprocessor flag and enable debug builds
if(${CX_DEBUG})
 #Debug build flags
 set(CMAKE_BUILD_TYPE Debug)
 #Set preprocessor flag
 add_cc_src_flags(CX_DEBUG)
 #Set compiler debug args
 add_cc_or_ld_argument(
  COMPILER
  Clang
   -g
   -O0
   -fno-inline
   -fno-inline-functions
   -fno-omit-frame-pointer
   -fno-lto
   -fstandalone-debug
  AppleClang
   -g
   -O0
   -fno-inline
   -fno-inline-functions
   -fno-omit-frame-pointer
   -fno-lto
   -fstandalone-debug
  GNU
   -gdwarf-2
   -O0
   -fno-inline
   -fno-inline-functions
   -fno-omit-frame-pointer
   -fno-lto
  #TODO Intel
  MSVC
   /Z7
   /DEBUG:FULL
   /Od
 )
else()
 #Set compiler release flags
 add_cc_or_ld_argument(
  COMPILER
  Clang
   -O3
  AppleClang
   -O3
  GNU
   -O3
  #TODO Intel
  MSVC
   /Ox
   /Oy
   /Oi
   /Ob2
 )
endif()

#Set the `CX_QA` preprocessor flag and enable a QA build
if(${CX_QA})
 #Set preprocessor flag
 add_cc_src_flags(CX_QA)
 #Set compiler QA args
 add_cc_or_ld_argument(
  COMPILER
  Clang
   -fno-omit-frame-pointer
   -fno-sanitize-recover=null
   -fsanitize=address
   -fsanitize=undefined
   -fsanitize-address-use-after-scope
   -fsanitize=bounds
   -fsanitize=null
  AppleClang
   -fno-omit-frame-pointer
   -fno-sanitize-recover=null
   -fsanitize=address
   -fsanitize=undefined
   -fsanitize-address-use-after-scope
   -fsanitize=bounds
   -fsanitize=null
  GNU
   -fno-omit-frame-pointer
   -fno-sanitize-recover=null
   -fsanitize=address
   -fsanitize=undefined
   -fsanitize-address-use-after-scope
   -fsanitize=bounds
   -fsanitize=null
  #TODO Intel
  #Note: MSVC only supports ASAN
  MSVC
   /fsanitize=address
 )

 #Set linker QA args
 add_cc_or_ld_argument(
  LINKER
  Clang
   -fsanitize=address
   -fsanitize=undefined
  AppleClang
   -fsanitize=address
   -fsanitize=undefined
  GNU
   -fsanitize=address
   -fsanitize=undefined
  IntelLLVM
   -fsanitize=address
   -fsanitize=undefined
 )

 #Warn about incomplete `CX_QA` support on MSVC
 if(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  message(
   WARNING
   "(CX_QA): MSVC only currently supports ASAN."
  )
 endif()
endif()

#Set the `CX_HEADER_ONLY` preprocessor flag
if(${CX_HEADER_ONLY})
 #Add preprocessor flag
 add_cc_src_flags(CX_HEADER_ONLY)
endif()

#Conditional library build type
if(${CX_HEADER_ONLY})
 #Set up header-only build
 #if(${CX_TOP_LEVEL})
 # set(CX_OPTION_SCOPE INTERFACE)
 #else()
 # set(CX_OPTION_SCOPE PRIVATE)
 #endif()
 add_library(cx INTERFACE)
 target_include_directories(cx INTERFACE include)
 #TODO Fix scope
 #target_compile_options(cx ${CX_OPTION_SCOPE} ${CX_COMPILE_FLAGS})
 target_compile_options(cx INTERFACE ${CX_COMPILE_FLAGS})
else()
 #Collect sources
 file(
  GLOB_RECURSE CX_SRC
  "src/**.c"
  "src/**.cpp"
 )

 #Set up shared library build
 if(${CX_TOP_LEVEL})
  set(CX_OPTION_SCOPE PUBLIC)
 else()
  set(CX_OPTION_SCOPE PRIVATE)
 endif()

 add_library(cx SHARED ${CX_SRC})
 target_include_directories(cx PUBLIC include)
 target_compile_options(cx ${CX_OPTION_SCOPE} ${CX_COMPILE_FLAGS})
 target_link_options(cx ${CX_OPTION_SCOPE} ${CX_LINK_FLAGS})
endif()

#Testing
if(${CX_UNIT_TESTS} OR ${CX_BENCHMARKS})
 add_subdirectory(test)

 #TODO coverage for multiple platforms
 if(${CX_COVERAGE})
  message(FATAL_ERROR "Coverage runs are not yet implemented")
 endif()
else()
 #Warn if `CX_COVERAGE` is enabled but tests are not
 if(${CX_COVERAGE})
  message(
   WARNING
   "'CX_COVERAGE' is enabled but none of [CX_UNIT_TESTS CX_BENCHMARKS] are \
   enabled, so no coverage runs will be performed.\
   "
  )
 endif()
endif()
