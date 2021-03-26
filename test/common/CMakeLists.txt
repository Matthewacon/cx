file(GLOB_RECURSE CX_TEST_COMMON_SRC "src/**.c" "src/**.cpp")
list(LENGTH CX_TEST_COMMON_SRC CX_TEST_COMMON_SRC_LEN)

#Local build variables
set(CX_TEST_COMMON_TARGET cx_test_common)

#If there areno sources, set up a header-only library
#Otherwise, set up a shared library
if(CX_TEST_COMMON_SRC_LEN EQUAL 0)
 #Set up header-only library
 add_library(${CX_TEST_COMMON_TARGET} INTERFACE)
 set(CX_TEST_COMMON_OPTION_SCOPE INTERFACE)
else()
 #Set up shared library
 add_library(${CX_TEST_COMMON_TARGET} SHARED ${CX_TEST_COMMON_SRC})
 set(CX_TEST_COMMON_OPTION_SCOPE PUBLIC)
endif()

target_include_directories(
 ${CX_TEST_COMMON_TARGET} ${CX_TEST_COMMON_OPTION_SCOPE} 
 include
 ${GOOGLE_TEST_INCLUDE_DIRS}
)
target_link_libraries(
 ${CX_TEST_COMMON_TARGET} ${CX_TEST_COMMON_OPTION_SCOPE}
 cx
 ${GOOGLE_TEST_TARGET}
)
target_compile_options(
 ${CX_TEST_COMMON_TARGET} ${CX_TEST_COMMON_OPTION_SCOPE} 
 ${CX_COMPILE_FLAGS}
)
target_link_options(
 ${CX_TEST_COMMON_TARGET} ${CX_TEST_COMMON_OPTION_SCOPE} 
 ${CX_LINK_FLAGS}
)

#Global build variables, required for dependents
set(CX_TEST_COMMON_TARGET ${CX_TEST_COMMON_TARGET} PARENT_SCOPE) 
set(CX_TEST_COMMON_INCLUDE_DIRS "${${CX_TEST_COMMON_TARGET}_SOURCE_DIR}/include" PARENT_SCOPE)
