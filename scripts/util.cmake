cmake_minimum_required(VERSION 3.19)

include_guard(GLOBAL)

#[[TODO:
 - Add unique name assertions to all function declarations
 - Add missing help messages to all functions
 - Replace manual length checks with `is_empty` invocations, excluding
   `is_name_unique` and `assert_name_unique` to prevent infinite loops
]]

#[[
 Checks for existing function/macro, varaible, cache variable or environment
 variable, with the same name. Emits a diagnostic if a collision is found.

 Note: Do not invoke `is_empty` or `assert_name_unique` in the body of this
 funciton to prevent infinite loops.
]]
if(COMMAND is_name_unique)
 message(
  FATAL_ERROR
  "Name collision: 'is_name_unique' command already defined elsewhere!"
 )
endif()
function(is_name_unique inu_NAME inu_CONDITION inu_DESTINATION_VARIABLE)
 #Help message
 string(
  APPEND inu_HELP_MESSAGE
  "'is_name_unique' takes the following arguments:"
  "\n - (REQUIRED) <NAME>: The name to check"
  "\n - (REQUIRED) <CONDITION>: The condition to check the name against; one "
  "of [COMMAND, VARIABLE, CACHE, ENV]"
  "\n - (REQUIRED) <DESTINATION_VARIABLE>: The name of the variable to place "
  "the result in"
 )

 #Validate variable name
 string(LENGTH "${inu_NAME}" inu_NAME_LENGTH)
 if(inu_NAME_LENGTH EQUAL 0)
  message("${inu_HELP_MESSAGE}")
  message(
   FATAL_ERROR
   "is_name_unique: The '<NAME>' argument must not be empty!"
  )
 endif()
 unset(inu_NAME_LENGTH)

 #Validate condition
 list(APPEND inu_SUPPORTED_CONDITIONS COMMAND VARIABLE CACHE ENV)
 if(NOT inu_CONDITION IN_LIST inu_SUPPORTED_CONDITIONS)
  message("${inu_HELP_MESSAGE}")
  message("is_name_unique: The condition '${inu_CONDITION}' is invalid.")
 endif()
 unset(inu_SUPPORTED_CONDITIONS)

 #Validate destination variable name
 string(LENGTH "${inu_DESTINATION_VARIABLE}" inu_DESTINATION_VARIABLE_LENGTH)
 if(inu_DESTINATION_VARIABLE_LENGTH EQUAL 0)
  message("${inu_HELP_MESSAGE}")
  message(
   FATAL_ERROR
   "is_name_unique: The '<DESTINATION_VARIABLE>' argument must not be empty!"
  )
 endif()
 unset(inu_DESTINATION_VARIABLE_LENGTH)

 #Perform check
 set(inu_UNIQUE FALSE)
 if(inu_CONDITION STREQUAL "COMMAND" AND NOT COMMAND "${inu_NAME}")
  set(inu_UNIQUE TRUE)
 elseif(inu_CONDITION STREQUAL "VARIABLE" AND NOT DEFINED "${inu_NAME}")
  set(inu_UNIQUE TRUE)
 elseif(inu_CONDITION STREQUAL "CACHE" AND NOT DEFINED CACHE{"${inu_NAME}"})
  set(inu_UNIQUE TRUE)
 elseif(inu_CONDITION STREQUAL "ENV" AND NOT DEFINED ENV{"${inu_NAME}"})
  set(inu_UNIQUE TRUE)
 endif()

 #Set result on destination variable in parent scope
 set("${inu_DESTINATION_VARIABLE}" "${inu_UNIQUE}" PARENT_SCOPE)
 unset(inu_UNIQUE)
endfunction()

#[[
 Same as `is_name_unique`, but emits a diagnostic instead of returning a value

 Note: Do not invoke `is_empty` in the body of this funciton to prevent
 infinite loops.
]]
if(COMMAND assert_name_unique)
 message(
  FATAL_ERROR
  "Name collision: 'assert_name_unique' command already defined elsewhere!"
 )
endif()
function(assert_name_unique anu_NAME anu_CONDITION)
 #[[
  Note: No need to check conditions since they will be checked by
  `is_name_unique`
 ]]

 #Parse message, if any
 cmake_parse_arguments(
  anu
  ""
  ""
  "MESSAGE"
  ${ARGN}
 )

 #Set up diagnostic message
 if(DEFINED anu_MESSAGE)
  #Use user-supplied message
  foreach(LINE ${anu_MESSAGE})
   string(APPEND anu_DIAGNOSTIC "${LINE}")
  endforeach()
 else()
  #Use default message
  set(anu_DIAGNOSTIC "'${anu_NAME}' is not a unique ${anu_CONDITION}!")
 endif()

 #Check for uniqueness and emit diagnostic if not unique
 is_name_unique("${anu_NAME}" "${anu_CONDITION}" anu_UNIQUE)
 if(NOT anu_UNIQUE)
  message(
   FATAL_ERROR
   "assert_name_unique: ${anu_DIAGNOSTIC}"
  )
 endif()
 unset(anu_UNIQUE)
endfunction()

#[[
 Generates unique a name to avoid colisions, based on the value of `CONDITION`.
 Defaults to generating random strings of legnth 5, lowercase characters only
 and a search limit of 10000.
]]
assert_name_unique(
 generate_unique_name
 COMMAND
 MESSAGE "Name collision: 'generate_unique_name' is already defined elsewhere!"
)
function(generate_unique_name gun_NAME gun_DESTINATION_VARIABLE)
 #Validate function name
 string(LENGTH "${gun_NAME}" gun_NAME_LENGTH)
 if(gun_NAME_LENGTH EQUAL 0)
  message(
   FATAL_ERROR
   "generate_unique_name: The '<FUNCTION_NAME>' argument cannot be "
   "empty!"
  )
 endif()
 unset(gun_NAME_LENGTH)

 #Validate destination variable name
 string(LENGTH "${gun_DESTINATION_VARIABLE}" gun_DESTINATION_VARIABLE_LENGTH)
 if(gun_DESTINATION_VARIABLE_LENGTH EQUAL 0)
  message(
   FATAL_ERROR
   "generate_unique_name: The '<DESTINATION_VARIABLE>' argumnet "
   "cannot be empty!"
  )
 endif()
 unset(gun_DESTINATION_VARIABLE_LENGTH)

 #Parse any overrides
 cmake_parse_arguments(
  gun
  ""
  "LENGTH;ALPHABET;LIMIT;CONDITION"
  ""
  ${ARGN}
 )

 #Determine random suffix length
 if(DEFINED gun_LENGTH)
  if(gun_LENGTH LESS 1)
   message(
    FATAL_ERROR
    "generate_unique_name: The 'LENGTH' argument cannot have a value "
    "less than 1!"
   )
  endif()
 else()
  set(gun_LENGTH 5)
 endif()

 #Determine alphabet for random suffix
 if(NOT DEFINED gun_ALPHABET)
  set(gun_ALPHABET "abcdefghijklmnopqrstuvwxyz")
 endif()

 #Determine search limit
 if(DEFINED gun_LIMIT)
  if(gun_LIMIT LESS 1)
   message(
    FATAL_ERROR
    "generate_unique_name: The 'LIMIT' argument cannot have a value "
    "less than 1!"
   )
  endif()
 else()
  set(gun_LIMIT 10000)
 endif()
 math(EXPR gun_LOOP_UPPER_LIMIT "${gun_LIMIT} + 1")

 #Validate condition
 list(APPEND gun_SUPPORTED_CONDITIONS COMMAND VARIABLE CACHE ENV)
 if(NOT DEFINED gun_CONDITION)
  set(gun_CONDITION_INVALID TRUE)
 elseif(NOT "${gun_CONDITION}" IN_LIST gun_SUPPORTED_CONDITIONS)
  set(gun_CONDITION_INVALID TRUE)
 endif()
 unset(gun_SUPPORTED_CONDITIONS)

 #Emit diagnostic if condition is invalid
 if(gun_CONDITION_INVALID)
  message(
   FATAL_ERROR
   "generate_unique_name: The 'CONDITION' argument must be provided! Valid "
   "values are: [COMMAND, VARIABLE, CACHE, ENV]."
  )
 endif()
 unset(gun_CONDITION_INVALID)

 #TODO Replace with function from above
 #Define function to check for unique name
 function(
  __generate_unique_name__condition
  gunc_NAME
  gunc_DESTINATION_VARIABLE
 )
  set(gunc_NAME_IS_UNIQUE FALSE)
  if(gun_CONDITION STREQUAL "COMMAND" AND NOT COMMAND "${gunc_NAME}")
   #Handle command conditions
   set(gunc_NAME_IS_UNIQUE TRUE)
  elseif(gun_CONDITION STREQUAL "VARIABLE" AND NOT DEFINED "${gunc_NAME}")
   #Handle variable conditions
   set(gunc_NAME_IS_UNIQUE TRUE)
  elseif(gun_CONDITION STREQUAL "CACHE" AND NOT DEFINED CACHE{"${gunc_NAME}"})
   #Handle cache variable conditions
   set(gunc_NAME_IS_UNIQUE TRUE)
  elseif(gun_CONDITION STREQUAL "ENV" AND NOT DEFINED ENV{"${gun_NAME}"})
   #Handle environment variable conditions
   set(gunc_NAME_IS_UNIQUE TRUE)
  endif()

  #Propagate result to parent scope
  set("${gunc_DESTINATION_VARIABLE}" "${gunc_NAME_IS_UNIQUE}" PARENT_SCOPE)
 endfunction()

 #Loop until random string is found, or limit is reached
 set(gun_UNIQUE_NAME_FOUND FALSE)
 set(gun_UNIQUE_NAME "${gun_NAME}")
 foreach(COUNT RANGE 1 ${gun_LOOP_UPPER_LIMIT})
  #If unique name found, stop searching
  __generate_unique_name__condition("${gun_UNIQUE_NAME}" gun_UNIQUE_NAME_FOUND)
  if(gun_UNIQUE_NAME_FOUND)
   break()
  endif()

  #Generate new function name
  string(
   RANDOM
   LENGTH ${gun_LENGTH}
   ALPHABET "${gun_ALPHABET}"
   gun_RANDOM_SUFFIX
  )
  set(gun_UNIQUE_NAME "${gun_NAME}_${gun_RANDOM_SUFFIX}")
 endforeach()
 unset(gun_LOOP_UPPER_LIMIT)
 unset(gun_RANDOM_SUFFIX)

 #If no unique function name found, emit diagnostic
 if(NOT gun_UNIQUE_NAME_FOUND)
  message(
   FATAL_ERROR
   "generate_unique_name: Was unable to find unique name for '${gun_NAME}', "
   "after ${gun_LIMIT} iterations. Try increasing the limit by providing the "
   "`LIMIT <positive integer>` argument. You may also try increasing the "
   "character pool by specifying an extended alphabet through the "
   "`ALPHABET <character string>` argument."
  )
 endif()
 unset(gun_UNIQUE_NAME_FOUND)

 #Set function name on destination variable in parent scope
 set("${gun_DESTINATION_VARIABLE}" "${gun_UNIQUE_NAME}" PARENT_SCOPE)
 unset(gun_UNIQUE_NAME)
endfunction()

#[[
 Checks if any number of arguments are empty and places result in destination
 variable
]]
function(is_empty ie_DESTINATION_VARIABLE)
 #Help message utility function
 function(__is_empty__print_help ie_ph_LEVEL)
  #Print help
  message(
   "'is_empty' takes the following arguments:"
   "\n - (REQUIRED) <DESTINATION_VARIABLE>: The name of the destination "
   "variable."
   "\n - (OPTIONAL) <STRINGS>...: Zero or more strings to check."
   "\n"
   "\nExamples:"
   "\n is_empty(MAYBE_EMPTY)"
   "\n message(\${MAYBE_EMPTY}) #prints 'TRUE'"
   "\n ---"
   "\n is_empty(MAYBE_EMPTY 123 abc)"
   "\n message(\${MAYBE_EMPTY}) #prints 'FALSE'"
  )

  #Print diagnostic
  list(LENGTH ARGN ie_ph_DYNAMIC_ARGUMENT_LENGTH)
  if(ie_ph_DYNAMIC_ARGUMENT_LENGTH GREATER 0)
   message(${ie_ph_LEVEL} ${ARGN})
  else()
   message(${ie_ph_LEVEL} "Illegal arguments supplied to 'is_empty'!")
  endif()
  unset(ie_ph_DYNAMIC_ARGUMENT_LENGTH)
 endfunction()
 unset(ie_ph_FUNCTION_GUARD_VARIABLE)

 #Validate destination variable
 string(LENGTH "${ie_DESTINATION_VARIABLE}" ie_DESTINATION_VARIABLE_LENGTH)
 if(ie_DESTINATION_VARIABLE_LENGTH EQUAL 0)
  __is_empty__print_help(
   FATAL_ERROR
   "is_empty: Destination variable name cannot be empty!"
  )
 endif()
 unset(ie_DESTINATION_VARIABLE_LENGTH)

 #Check if remaining arguments are empty
 string(LENGTH "${ARGN}" ie_ARGN_LENGTH)
 if(ie_ARGN_LENGTH EQUAL 0)
  set(ie_ARGN_EMPTY TRUE)
 else()
  set(ie_ARGN_EMPTY FALSE)
 endif()

 #Set destination variable in parent scope
 set("${ie_DESTINATION_VARIABLE}" "${ie_ARGN_EMPTY}" PARENT_SCOPE)
 unset(ie_ARGN_EMPTY)
 unset(ie_ARGN_LENGTH)
endfunction()

#[[
 Generates a prefix based on the current `project()` scope for storing
 project-related information on
]]
function(get_project_prefix gpp_DESTINATION_VARIABLE)
 #Validate destination variable name
 is_empty(gpp_DESTINATION_VARIABLE_EMPTY "${gpp_DESTINATION_VARIABLE}")
 if(gpp_DESTINATION_VARIABLE_EMPTY)
  message(
   FATAL_ERROR
   "get_project_prefix: Destination variable cannot be empty!"
  )
 endif()
 unset(gpp_DESTINATION_VARIABLE_EMPTY)

 #Determine project prefix
 if(NOT DEFINED CMAKE_PROJECT_NAME)
  #Use `NO_PROJECT` if not in project scope
  set(gpp_PROJECT_NAME "NO_PROJECT")

  #[[
   Emit warning if invoked in a non-`project()` scope
   Note: Do not spam with warnings for every invocation, just on first
  ]]
  set(gpp_DIAGNOSTIC_VARIABLE "${gpp_PROJECT_NAME}_DIAGNOSTIC")
  if(NOT DEFINED "${gpp_DIAGNOSTIC_VARIABLE}")
   message(
    WARNING
    "get_project_prefix: Not in a `project()` scope, using 'NO_PROJECT' as "
    "prefix!"
   )
   set("${gpp_DIAGNOSTIC_VARIABLE}" "" CACHE INTERNAL "")
  endif()
  unset(gpp_DIAGNOSTIC_VARIABLE)
 else()
  #Use `${CMAKE_PROJECT_NAME}` if invoked in `project()` scope
  set(gpp_PROJECT_NAME "${CMAKE_PROJECT_NAME}")
 endif()

 #TODO Prefix with the name of this CMake library, once separated from cx
 #Standardize project name
 string(TOUPPER "${gpp_PROJECT_NAME}" "${gpp_DESTINATION_VARIABLE}")

 #Set destination variable in parent scope
 set(
  "${gpp_DESTINATION_VARIABLE}" "${${gpp_DESTINATION_VARIABLE}}"
  PARENT_SCOPE
 )
endfunction()
