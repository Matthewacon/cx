include(CTest)
enable_testing()

#googletest setup
set(BUILD_GMOCK ON CACHE BOOL "Enable GMock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Disable GTest installation" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Enable building googletest as a shared library" FORCE)
add_latent_dependency(
 SCOPE_ID test
 NAME gtest
 GIT_REPOSITORY https://github.com/google/googletest.git
 GIT_TAG a3460d1aeeaa43fdf137a6adefef10ba0b59fe4b
 TARGET_NAMES gmock gmock_main
)

set(GOOGLE_TEST_TARGET gmock)
set(GOOGLE_TEST_MAIN_TARGET gmock_main)
set(GOOGLE_TEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include)

#googlebenchmark setup
if(${CX_BENCHMARKS})
 set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable googlebenchmark internal tests" FORCE)
 set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable googletest internal tests" FORCE)
 set(BENCHMARK_USE_LIBCXX OFF CACHE BOOL "Disable googlebenchmark use of libcxx" FORCE)
 add_latent_dependency(
  SCOPE_ID test
  NAME gbenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.5.2
  TARGET_NAMES benchmark::benchmark
 )

 set(GOOGLE_BENCHMARK_TARGET benchmark::benchmark)
 set(GOOGLE_BENCHMARK_MAIN_TARGET benchmark::benchmark_main)
 set(GOOGLE_BENCHMARK_INCLUDE_DIRS ${gbenchmark_SOURCE_DIR}/include)
endif()

#Fetch all test dependencies
fetch_latent_dependencies(SCOPE_ID test)

#Common testing utilities
add_subdirectory(common)

if(${CX_UNIT_TESTS})
 add_subdirectory(unit)
endif()

if(${CX_INTEGRATION_TESTS})
 add_subdirectory(integration)
endif()

if(${CX_BENCHMARKS})
 add_subdirectory(benchmark)
endif()

#Update parent list of tests
set(CX_TEST_TARGETS "${CX_TEST_TARGETS}" PARENT_SCOPE)
