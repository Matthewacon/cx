cmake_minimum_required(VERSION 3.19)

include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/util.cmake)

#[[TODO:
 - Set up global cache variable for prefixing all definitions in this library
 - Add unique name assertions to all function declarations
 - Remove internal `print_help` variants in favour of manual `message`
   invocations
 - Add help messages to all functions
]]

#[[
 Generates a unique prefix for storing infromation related to compiler flags,
 source flags, linker flags, etc...
]]
function(get_project_compiler_details_prefix gpcdp_DESTINATION_VARIABLE)
 #Get project prefix
 get_project_prefix("${gpcdp_DESTINATION_VARIABLE}")

 #Create unique variable name for storing compiler related information
 string(APPEND "${gpcdp_DESTINATION_VARIABLE}" "_COMPILER_DETAILS")

 #Set destination variable in parent scope
 set(
  "${gpcdp_DESTINATION_VARIABLE}" "${${gpcdp_DESTINATION_VARIABLE}}"
  PARENT_SCOPE
 )
endfunction()

#[[
 Detect the compiler and validate it against a user-supplied list of supported
 compilers. Stores list of supported compilers for the current project, as well
 as the detected compiler, for later use.
]]
function(detect_compiler dc_DESTINATION_VARIABLE)
 #Get compiler details variable
 get_project_compiler_details_prefix(dc_COMPILER_DETAILS_PREFIX)

 #Help message utility function
 function(dc_print_help dc_ph_LEVEL)
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
  dc_print_help(
   FATAL_ERROR
   "detect_compiler: The 'COMPILER_ID' argument must be provided!"
  )
 endif()

 if(NOT DEFINED "${dc_COMPILER_ID}")
  dc_print_help(
   FATAL_ERROR
   "detect_compiler: The provided compiler ID variable, '${dc_COMPILER}', is "
   "not set! Are you sure that you provided the correct compiler ID variable?"
  )
 endif()

 #Validate `dc_SUPPORTED_COMPILERS`
 if(NOT DEFINED dc_SUPPORTED_COMPILERS)
  dc_print_help(
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

 #Store supported compiler list, flags and other information
 set(
  "${dc_COMPILER_DETAILS_PREFIX}_ALLOW_UNSUPPORTED" "${dc_ALLOW_UNSUPPORTED}"
  PARENT_SCOPE
 )
 set(
  "${dc_COMPILER_DETAILS_PREFIX}_DETECTED_COMPILER_ID"
  "${dc_DESTINATION_VARIABLE}"
  PARENT_SCOPE
 )
 set(
  "${dc_COMPILER_DETAILS_PREFIX}_SUPPORTED_COMPILERS"
  "${dc_SUPPORTED_COMPILERS}"
  PARENT_SCOPE
 )

 #Emit diagnostic if compiler is not supported
 if(NOT dc_SUPPORTED_COMPILER_DETECTED)
  #Set up diagnostic level
  if(dc_ALLOW_UNSUPPORTED)
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

#[[
 TODO Retrieves the list of supported compilers and places it in the destination
 variable
]]
function(get_supported_compilers gsc_DESTINATION_VARIABLE)
endfunction()

#Checks whether a given compiler ID is supported
function(is_compiler_supported ics_COMPILER ics_DESTINATION_VARIABLE)
 #Get compiler details prefix
 get_project_compiler_details_prefix(ics_COMPILER_DETAILS_PREFIX)

 #Help message utility function
 function(ics_print_help ics_ph_LEVEL)
  #Print help
  message(
   "'is_compiler_supported' takes the following arguments:"
   "\n - (REQUIRED) <COMPILER_ID>: The name of the compiler to check"
   "\n - (REQUIRED) <DESTINATION_VARIABLE>: The name of the destination "
   "variable, to place the result in"
   "\n\nExample:"
   "\n detect_compiler("
   "\n  MY_DETECTED_COMPILER_ID_RESULT"
   "\n  COMPILER_ID CMAKE_C_COMPILER_ID"
   "\n  SUPPORTED_COMPILERS GNU MSVC"
   "\n )"
   "\n is_compiler_supported(GNU GNU_SUPPORTED)"
   "\n message(\${GNU_SUPPORTED}) #prints 'TRUE'"
   "\n is_compiler_supported(Clang Clang_SUPPORTED)"
   "\n message(\${Clang_SUPPORTED}) #prints 'FALSE'"
  )

  #Print diagnostic
  list(LENGTH ARGN ics_ph_DYNAMIC_ARGUMENT_LENGTH)
  if(ics_ph_DYNAMIC_ARGUMENT_LENGTH GREATER 0)
   message(${ics_ph_LEVEL} ${ARGN})
  else()
   message(
    ${ics_ph_LEVEL}
    "Illegal arguments supplied to 'is_compiler_supported!'"
   )
  endif()
  unset(ics_ph_DYNAMIC_ARGUMENT_LENGTH)
 endfunction()


 #Validate compiler name
 is_empty(ics_COMPILER_EMPTY "${ics_COMPILER}")
 if(ics_COMPILER_EMPTY)
  ics_print_help(
   FATAL_ERROR
   "is_compiler_supported: The 'COMPILER' argument cannot be empty!"
  )
 endif()
 unset(ics_COMPILER_EMPTY)

 #Validate destination variable name
 is_empty(ics_DESTINATION_VARIABLE_EMPTY "${ics_DESTINATION_VARIABLE}")
 if(ics_DESTINATION_VARIABLE_EMPTY)
  ics_print_help(
   FATAL_ERROR
   "is_compiler_supported: The 'DESTINATION_VARIABLE' argument cannot be "
   "empty!"
  )
 endif()
 unset(ics_DESTINATION_VARIABLE_EMPTY)

 #Check if compiler is supported
 set(
  ics_SUPPORTED_COMPILER_LIST_VARIABLE
  "${ics_COMPILER_DETAILS_PREFIX}_SUPPORTED_COMPILERS"
 )
 if(ics_COMPILER IN_LIST "${ics_SUPPORTED_COMPILER_LIST_VARIABLE}")
  set(ics_COMPILER_SUPPORTED TRUE)
 else()
  set(ics_COMPILER_SUPPORTED FALSE)
 endif()
 unset(ics_SUPPORTED_COMPILER_LIST_VARIABLE)

 #Set result on destination variable in parent scope
 set("${ics_DESTINATION_VARIABLE}" "${ics_COMPILER_SUPPORTED}" PARENT_SCOPE)
 unset(ics_COMPILER_SUPPORTED)
endfunction()

#TODO Adds a compiler-specific formatter for defines and source flags
function(add_compiler_define_formatter acdf_COMPILER acdf_FORMATTER_FUNCTION)
 #TODO Validate compiler name

endfunction()

#[[
 TODO Gets the name of the compiler-specific define formatter function and
 places it in the destination variable
]]
function(get_compiler_define_formatter gcdf_COMPILER)
endfunction()

#[[
 TODO Removes the association between a compiler and its define formatter
 function. Useful for users that may want to override the default formatters.
]]
function(remove_compiler_define_formatter fcdf_COMPILER)
endfunction()

#TODO Add compiler-specific source define for c-preprocessor
function(add_cc_define)
endfunction()

#TODO Add compiler or linker flags, segmented by compiler
function(add_cc_or_ld_argument)
endfunction()
