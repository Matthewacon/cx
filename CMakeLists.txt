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
 if(CMAKE_C_COMPILER_ID STREQUAL COMPILER AND CMAKE_CXX_COMPILER_ID STREQUAL COMPILER)
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
set(CX_COVERAGE OFF CACHE BOOL "Enable CX coverage profiling (ONLY WORKS WITH CLANG)")

#Library support flags
set(CX_STL_SUPPORT OFF CACHE BOOL "Enable CX->stl integration")
set(CX_LIBC_SUPPORT ON CACHE BOOL "Enable CX->libc integration")

#Behavioural flags
set(CX_VARIANT_HARD_CLEAR OFF CACHE BOOL "Enable clearing of variant cache before initialization and after destruction")
set(CX_VARARG_INTRENSICS OFF CACHE BOOL
 "Enable definition of va_list function intrensics in 'cx/vararg.h', that would \
  otherwise be defined by '<cstdarg>'\
 "
)

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

#Set default compile flags
if(
 CX_DETECTED_COMPILER_ID STREQUAL "Clang"
 OR CX_DETECTED_COMPILER_ID STREQUAL "AppleClang"
 OR CX_DETECTED_COMPILER_ID STREQUAL "IntelLLVM"
)
 #Flag for clang-like compilers
 set(CX_COMPILER_CLANG_LIKE TRUE)

 #Define compiler macro for CX
 if(CX_DETECTED_COMPILER_ID STREQUAL "AppleClang")
  #Apple clang
  list(
   APPEND CX_COMPILE_FLAGS
   -DCX_COMPILER_APPLE
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Clang")
  #Clang
  list(
   APPEND CX_COMPILE_FLAGS
   -DCX_COMPILER_CLANG
  )
 else()
  #IntelLLVM
  list(
   APPEND CX_COMPILE_FLAGS
   -DCX_COMPILER_INTEL_LLVM
  )
 endif()

 #Default clang compile flags
 list(
  APPEND CX_COMPILE_FLAGS
  -DCX_COMPILER_CLANG_LIKE
  -Wall
  -Wextra
  -Werror
  -pedantic
  -ftemplate-backtrace-limit=0
  -fno-common
  -Wno-zero-length-array
 )
elseif(CX_DETECTED_COMPILER_ID STREQUAL "GNU")
 #Default gcc compile flags
 list(
  APPEND CX_COMPILE_FLAGS
  -DCX_COMPILER_GCC
  -Wall
  -Wextra
  -Wno-cast-function-type
  -ftemplate-backtrace-limit=0
  -fno-common
 )
elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
 #TODO default intel compile flags
 list(
  APPEND CX_COMPILE_FLAGS
  -DCX_COMPILER_INTEL
 )
 message(
  FATAL_ERROR
  "Default compiler flags for Intel are not yet implemented."
 )
elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
 #Default msvc compile flags
 list(
  APPEND CX_COMPILE_FLAGS
  /DCX_COMPILER_MSVC
  /std:c++20
  /Wall
  /WX
 )
endif()

#Conditional compile flags
if(${CX_STL_SUPPORT})
 #Add source flag for stl integration
 if(CX_COMPILER_CLANG_LIKE OR CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  list(
   APPEND CX_COMPILE_FLAGS
   -DCX_STL_SUPPORT
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  message(
   FATAL_ERROR
   "Setting 'CX_STL_SUPPORT' on Intel is not yet supported."
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  list(
   APPEND CX_COMPILE_FLAGS
   /DCX_STL_SUPPORT
  )
 endif()
else()
 #Disable linking against stl lib
 if(CX_COMPILER_CLANG_LIKE OR CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  list(
   APPEND CX_COMPILE_FLAGS
   -nostdlib++
   -fno-exceptions
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  message(
   FATAL_ERROR
   "Setting 'CX_STL_SUPPORT' on Intel is not yet supported."
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  #Compile flags
  list(
   APPEND CX_COMPILE_FLAGS
   /EHsc
  )
  #Link flags
  list(
   APPEND CX_LINK_FLAGS
   /NODEFAULTLIB:libcpmt.lib
   /NODEFAULTLIB:msvcprt.lib
   /NODEFAULTLIB:libcpmtd.lib
   /NODEFAULTLIB:msvcprtd.lib
  )
 endif()
endif()

if(${CX_LIBC_SUPPORT})
 #Add source flag for libc integration
 if(CX_COMPILER_CLANG_LIKE OR CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  list(
   APPEND CX_COMPILE_FLAGS
   -DCX_LIBC_SUPPORT
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  message(
   FATAL_ERROR
   "Setting 'CX_LIBC_SUPPORT' on Intel is not yet supported."
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  list(
   APPEND CX_COMPILE_FLAGS
   /DCX_LIBC_SUPPORT
  )
 endif()
else()
 #Disable linking against platform libc equivalent (uCRT on Windows)
 if(CX_COMPILER_CLANG_LIKE OR CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  list(
   APPEND CX_COMPILE_FLAGS
   -nostdlib
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  message(
   FATAL_ERROR
   "Setting 'CX_LIBC_SUPPORT' on Intel is not yet supported."
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  #Compile flags
  list(
   APPEND CX_COMPILE_FLAGS
   /NODEFAULTLIB
   /GS-
   #/MT
   /clr:nostdlib
   /Zl
  )
  #Link flags
  list(
   APPEND CX_LINK_FLAGS
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
endif()

if(${CX_VARIANT_HARD_CLEAR})
 if(CX_COMPILER_CLANG_LIKE OR CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  list(
   APPEND CX_COMPILE_FLAGS
   -DCX_VARIANT_HARD_CLEAR
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  message(
   FATAL_ERROR
   "Setting 'CX_VARIANT_HARD_CLEAR' on Intel is not yet supported."
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  list(
   APPEND CX_COMPILE_FLAGS
   /DCX_VARIANT_HARD_CLEAR
  )
 endif()
endif()

if(${CX_VARARG_INTRENSICS})
 if(CX_COMPILER_CLANG_LIKE OR CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  list(
   APPEND CX_COMPILE_FLAGS
   -DCX_VARARG_INTRENSICS
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  message(
   FATAL_ERROR
   "Setting 'CX_VARARG_INTRENSICS' on Intel is not yet supported."
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  list(
   APPEND CX_COMPILE_FLAGS
   /DCX_VARARG_HARD_CLEAR
  )
 endif()
endif()

if(${CX_DEBUG})
 #Debug build flags
 set(CMAKE_BUILD_TYPE Debug)
 if(${CX_COMPILER_CLANG_LIKE})
  #Clang-like debug flags
  list(
   APPEND CX_COMPILE_FLAGS
   -g
   -O0
   -fno-inline
   -fno-inline-functions
   -fno-omit-frame-pointer
   -fno-lto
   -fstandalone-debug
   -DCX_DEBUG
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  #GCC debug flags
  list(
   APPEND CX_COMPILE_FLAGS
   -gdwarf-2
   -O0
   -fno-inline
   -fno-inline-functions
   -fno-omit-frame-pointer
   -fno-lto
   -DCX_DEBUG
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  #TODO Intel compiler flags
  message(
   FATAL_ERROR
   "Debug compiler flags for Intel are not yet implemented."
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  #MSVC compiler flags
  list(
   APPEND CX_COMPILE_FLAGS
   /Z7
   /DEBUG:FULL
   /Od
   /DCX_DEBUG
  )
 endif()
else()
 if(${CX_COMPILER_CLANG_LIKE})
  #Clang-like release flags
  list(
   APPEND CX_COMPILE_FLAGS
   -O3
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  #GNU release flags
  list(
   APPEND CX_COMPILE_FLAGS
   -O3
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  #Intel release flags
  message(
   FATAL_ERROR
   "Build flags for Intel are not yet implemented."
  )
 elseif(CX_DTECTED_COMPILER_ID STREQUAL "MSVC")
  #MSVC release flags
  list(
   APPEND CX_COMPILE_FLAGS
   /Ox
   /Oy
   /Oi
   /Ob2
  )
 endif()
endif()

#Conditional compile/link flags
if(${CX_QA})
 if(CX_COMPILER_CLANG_LIKE OR CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  #Clang-like and GNU QA flags
  list(
   APPEND CX_COMPILE_FLAGS
   -fno-omit-frame-pointer
   -fno-sanitize-recover=null
   -fsanitize=address
   -fsanitize=undefined
   -fsanitize-address-use-after-scope
   -fsanitize=bounds
   -fsanitize=null
  )
  list(
   APPEND CX_LINK_FLAGS
   -fsanitize=address
   -fsanitize=undefined
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  #TODO Intel QA flags
  message(
   FATAL_ERROR
   "Setting 'CX_QA' on Intel is not yet supported."
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  #MSVC QA flags
  message(
   WARNING
   "(CX_QA): MSVC only currently supports ASAN."
  )
  list(
   APPEND CX_COMPILE_FLAGS
   /fsanitize=address
  )
 endif()
endif()

#Conditional src flags for CX
if(${CX_HEADER_ONLY})
 #Add source flag for header-only builds
 if(CX_COMPILER_CLANG_LIKE OR CX_DETECTED_COMPILER_ID STREQUAL "GNU")
  list(
   APPEND CX_COMPILE_FLAGS
   -DCX_HEADER_ONLY
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "Intel")
  message(
   FATAL_ERROR
   "Setting 'CX_HEADER_ONLY' on Intel is not yet supported."
  )
 elseif(CX_DETECTED_COMPILER_ID STREQUAL "MSVC")
  list(
   APPEND CX_COMPILE_FLAGS
   /DCX_HEADER_ONLY
  )
 endif()
endif()

#Conditional library build type
if(${CX_HEADER_ONLY})
 #Set up header-only build
 if(${CX_TOP_LEVEL})
  set(CX_OPTION_SCOPE INTERFACE)
 else()
  set(CX_OPTION_SCOPE PRIVATE)
 endif()
 add_library(cx INTERFACE)
 target_include_directories(cx INTERFACE include)
 target_compile_options(cx ${CX_OPTION_SCOPE} ${CX_COMPILE_FLAGS})
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

 #TODO add llvm-coverage custom targets
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
