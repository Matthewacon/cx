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

#Display error if compiler is not clang or gcc
if(NOT (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"
 OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang"
 OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"
 OR ${CMAKE_C_COMPILER_ID} STREQUAL "GNU"))
 message(
  FATAL_ERROR
  "You are using an unsupported compiler, please build with Clang or GCC."
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
set(CX_HEADER_ONLY OFF CACHE BOOL "Builds CX as a header-only library")
set(CX_UNIT_TESTS OFF CACHE BOOL "Builds CX unit tests")
set(CX_BENCHMARKS OFF CACHE BOOL "Builds CX benchmarks")
set(CX_COVERAGE OFF CACHE BOOL "Enable CX coverage profiling (ONLY WORKS WITH CLANG)")
set(CX_STL_SUPPORT OFF CACHE BOOL "Enable CX->stl integration")
set(CX_LIBC_SUPPORT ON CACHE BOOL "Enable CX->libc integration")

#Sanitize coverage flag
if(
 ${CX_COVERAGE}
 AND NOT (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND ${CMAKE_C_COMPILER_ID} STREQUAL "Clang"))
 message(
  FATAL_ERROR
  "'CX_COVERAGE' can only be used when building with Clang"
 )
endif()

#Default compile flags
list(
 APPEND CX_COMPILE_FLAGS
 -Wall
 -Wextra
 -Werror
 -pedantic
 -ftemplate-backtrace-limit=0
 -fno-common
 -Wno-zero-length-array
)

#Conditional compile flags
if(${CX_STL_SUPPORT})
 #Add source flag for stl integration
 list(
  APPEND CX_COMPILE_FLAGS
  -DCX_STL_SUPPORT
 )
endif()

if(${CX_LIBC_SUPPORT})
 #Add source flag for libc integration
 list(
  APPEND CX_COMPILE_FLAGS
  -DCX_LIBC_SUPPORT
 )
else()
 #Warn user of unavailable headers when building without `-DCX_LIBC_SUPPORT`
 message(
  WARNING
  "'CX_LIBC_SUPPORT' is disabled; the following headers will be unavailable:\
   <cx/vararg.h>\
  "
 )
endif()

if(${CX_DEBUG})
 #Debug build flags
 set(CMAKE_BUILD_TYPE Debug)
 list(
  APPEND CX_COMPILE_FLAGS
  -g
  -O0
  -fno-inline
  -fno-inline-functions
  -fno-omit-frame-pointer
  -fno-lto
  -DCX_DEBUG
 )
 #Clang-specific debug flags
 if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
  list(
   APPEND CX_COMPILE_FLAGS
   -fstandalone-debug
  )
 endif()
else()
 #Release build flags
 list(
  APPEND CX_COMPILE_FLAGS
  -O3
  #TODO LTO enabled builds
  #-flto
 )
endif()

#Conditional link flags
if(${CX_QA})
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
endif()

#Conditional library build type
if(${CX_HEADER_ONLY})
 #Warning for header-only builds and flag to disable headers with linkage
 message(
  WARNING
  "CX is building in header-only mode; the following headers will be unavailable:\
   <cx/memory.h>\
  "
 )

 #Add source flag for header-only builds
 list(
  APPEND CX_COMPILE_FLAGS
  -DCX_HEADER_ONLY
 )

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
