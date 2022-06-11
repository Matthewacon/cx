cmake_minimum_required(VERSION 3.19)

include(${CMAKE_CURRENT_LIST_DIR}/util.cmake)

#[[
 Detect the compiler and validate it against a user-supplied list of supported
 compilers
]]
function(detect_compiler dc_DESTINATION_VARIABLE)
 #Help message utility function
 function(print_help dc_ph_LEVEL)
  message(
   "'detect_compiler' takes the following arguments:"
   "\n - (REQUIRED) <DESTINATION_VARIABLE> - The name of the variable to "
   "place the detected compiler ID"
   "\n - (REQUIRED) 'COMPILER_ID' - CMake compiler ID variable to track; ie. "
   "CMAKE_CXX_COMPILER_ID. For more information, see: "
   "https://cmake.org/cmake/help/v3.19/variable/CMAKE_LANG_COMPILER_ID.html"
   "\n - (REQUIRED) 'SUPPORTED_COMPILERS'... - Space separated list of "
   "compiler IDs supported by your project"
   "\n\nExamples:"
   "\n detect_compiler("
   "\n  MY_DETECTED_COMPILER_ID_RESULT"
   "\n  COMPILER_ID CMAKE_C_COMPILER_ID"
   "\n  SUPPORTED_COMPILERS Clang GNU Intel MSVC"
   "\n )"
   "\n message(\"DETECTED COMPILER: \${MY_DETECTED_COMPILER_ID_RESULT}\")"
   "\n ---"
   "\n detect_compiler("
   "\n  MY_DETECTED_COMPILER_ID_RESULT"
   "\n  ALLOW_UNSUPPORTED #do not emit an error if no suitable compiler is found"
   "\n  COMPILER_ID CMAKE_C_COMPILER_ID"
   "\n  SUPPORTED_COMPILERS Clang GNU Intel MSVC"
   "\n )"
   "\n message(\"DETECTED COMPILER: \${MY_DETECTED_COMPILER_ID_RESULT}\")"
  )

  #Print diagnostic
  list(LENGTH ARGN dc_ph_DYNAMIC_ARGUMENT_LENGTH)
  if(dc_ph_DYNAMIC_ARGUMENT_LENGTH GREATER 0)
   message(${dc_ph_LEVEL} ${ARGN})
  else()
   message(${dc_ph_LEVEL} "Illegal arguments supplied to 'detect_compiler'!")
  endif()
  unset(dc_ph_DYNAMIC_ARGUMENT_LENGTH)
 endfunction()

 #Validate detected compiler destination variable name
 is_empty(dc_DESTINATION_VARIABLE_EMPTY "${dc_DESTINATION_VARIABLE}")
 if(dc_DESTINATION_VARIABLE_EMPTY)
  message(
   FATAL_ERROR
   "detect_compiler: Destination variable name cannot be empty!"
  )
 endif()
 unset(dc_DESTINATION_VARIABLE_EMPTY)

 #Parse arguments
 cmake_parse_arguments(
  dc
  "ALLOW_UNSUPPORTED"
  "COMPILER_ID"
  "SUPPORTED_COMPILERS"
  ${ARGN}
 )

 #Validate `dc_COMPILER`
 if(NOT DEFINED dc_COMPILER_ID)
  print_help(
   FATAL_ERROR
   "detect_compiler: The 'COMPILER_ID' argument must be provided!"
  )
 endif()

 if(NOT DEFINED "${dc_COMPILER_ID}")
  print_help(
   FATAL_ERROR
   "detect_compiler: The provided compiler ID variable, '${dc_COMPILER}', is "
   "not set! Are you sure that you provided the correct compiler ID variable?"
  )
 endif()

 #Validate `dc_SUPPORTED_COMPILERS`
 if(NOT DEFINED dc_SUPPORTED_COMPILERS)
  print_help(
   FATAL_ERROR
   "detect_compiler: At least one compiler must be provided for the "
   "'SUPPORTED_COMPILERS' argument! See "
   "https://cmake.org/cmake/help/v3.19/variable/CMAKE_LANG_COMPILER_ID.html "
   "for more information."
  )
 endif()

 #Determine whether `${dc_COMPILER_ID}` is in the list of supported compilers
 set(dc_SUPPORTED_COMPILER_DETECTED FALSE)
 foreach(COMPILER ${dc_SUPPORTED_COMPILERS})
  #If supported compiler is found, set destination variable on parent scope
  if("${COMPILER}" STREQUAL "${${dc_COMPILER_ID}}")
   set(dc_SUPPORTED_COMPILER_DETECTED TRUE)
   set("${dc_DESTINATION_VARIABLE}" "${COMPILER}" PARENT_SCOPE)
   break()
  endif()
 endforeach()

 #Emit diagnostic if compiler is not supported
 if(NOT dc_SUPPORTED_COMPILER_DETECTED)
  #Set up diagnostic level
  if(dc_ALLOW_SUPPORTED)
   set(dc_DIAGNOSTIC_LEVEL WARNING)
  else()
   set(dc_DIAGNOSTIC_LEVEL FATAL_ERROR)
  endif()

  #Assemble pretty compiler list
  foreach(COMPILER ${dc_SUPPORTED_COMPILERS})
   string(APPEND dc_PRETTY_COMPILER_STR "\n - ${COMPILER}")
  endforeach()

  #Emit diagnostic
  message(
   ${dc_DIAGNOSTIC_LEVEL}
   "'${${dc_COMPILER_ID}}' is an unsupported compiler. Supported compilers "
   "include: ${dc_PRETTY_COMPILER_STR}"
  )
  unset(dc_PRETTY_COMPILER_STR)
  unset(dc_DIAGNOSTIC_LEVEL)
 endif()
 unset(dc_SUPPORTED_COMPILER_DETECTED)
endfunction()
