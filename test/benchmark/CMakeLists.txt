file(GLOB_RECURSE CX_BENCHMARKS "src/**.c" "src/**.cpp")

foreach(BENCHMARK ${CX_BENCHMARKS})
 #Convert the fully-qualified path in `BENCHMARK` to a path relative to
 #the current source directory
 file(RELATIVE_PATH BENCHMARK "${CMAKE_CURRENT_SOURCE_DIR}" "${BENCHMARK}")

 #Convert the path to a cmake-style list
 string(REPLACE "/" ";" BENCHMARK_FILE_PATH "${BENCHMARK}")
 
 #Get file name from the path and remove the extension
 list(GET BENCHMARK_FILE_PATH -1 BENCHMARK_FILE_NAME)
 string(REPLACE ".cpp" "" BENCHMARK_FILE_NAME "${BENCHMARK_FILE_NAME}")
 string(REPLACE ".c" "" BENCHMARK_FILE_NAME "${BENCHMARK_FILE_NAME}")
 
 #Build preface string with all leading directories unitl `src/cx` to
 #distinguish tests with the same name. (removes the `src` and `cx` 
 #elements as well as the target name, since it will be combined later)
 list(REMOVE_AT BENCHMARK_FILE_PATH 1 0 -1)
 #Reset the preface string before building it
 unset(BENCHMARK_FILE_PATH_PREFIX)
 foreach(PATH_PREFIX ${BENCHMARK_FILE_PATH})
  string(APPEND BENCHMARK_FILE_PATH_PREFIX "${PATH_PREFIX}.")
 endforeach()

 #Combine `BENCHMARK_FILE_PATH_PREFIX` and `BENCHMARK_FILE_NAME` to form the unique
 #test target name
 string(PREPEND BENCHMARK_FILE_NAME "${BENCHMARK_FILE_PATH_PREFIX}")

 set(BENCHMARK_TARGET_NAME "${BENCHMARK_FILE_NAME}_benchmark")
 add_executable(${BENCHMARK_TARGET_NAME} ${BENCHMARK})
 target_include_directories(
  ${BENCHMARK_TARGET_NAME} PRIVATE
  include
  ${CX_TEST_COMMON_INCLUDE_DIRS} 
  ${GOOGLE_BENCHMARK_INCLUDE_DIRS}
 )
 target_compile_options(
  ${BENCHMARK_TARGET_NAME} PRIVATE
  "${CX_COMPILE_FLAGS}"
 )
 target_link_libraries(
  ${BENCHMARK_TARGET_NAME}
  cx
  ${CX_TEST_COMMON_TARGET}
  ${GOOGLE_BENCHMARK_TARGET}
  ${GOOGLE_BENCHMARK_MAIN_TARGET}
 )
 target_link_options(${BENCHMARK_TARGET_NAME} PUBLIC ${CX_LINK_FLAGS})
 add_test(
  NAME ${BENCHMARK_TARGET_NAME}
  COMMAND ./${BENCHMARK_TARGET_NAME}
  WORKING_DIRECTORY benchmark 
 )
 list(APPEND CX_BENCHMARK_TARGETS "${BENCHMARK_TARGET_NAME}")
endforeach()

set(CX_TEST_TARGETS "${CX_BENCHMARK_TARGETS}" PARENT_SCOPE)
