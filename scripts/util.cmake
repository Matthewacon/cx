cmake_minimum_required(VERSION 3.19)

include_guard(GLOBAL)

#[[
 TODO Set up global cache variable for prefixing all definitions in this
 library
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
  "Name collision: Function 'is_name_unique' command already defined "
  "elsewhere!"
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
 elseif(inu_CONDITION STREQUAL "CACHE" AND NOT DEFINED CACHE{${inu_NAME}})
  set(inu_UNIQUE TRUE)
 elseif(inu_CONDITION STREQUAL "ENV" AND NOT DEFINED ENV{${inu_NAME}})
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
  "Name collision: Function 'assert_name_unique' command already defined "
  "elsewhere!"
 )
endif()
function(assert_name_unique anu_NAME anu_CONDITION)
 #Help message
 string(
  APPEND anu_HELP_MESSAGE
  "'assert_name_unique' takes the following arguments:"
  "\n - (REQUIRED) <NAME>: The name to check"
  "\n - (REQUIRED) <CONDITION>: The condition to check the name against; one "
  "of [COMMAND, VARIABLE, CACHE, ENV]"
 )

 #Validate name
 string(LENGTH "${anu_NAME}" anu_NAME_LENGTH)
 if(anu_NAME_LENGTH EQUAL 0)
  message("${anu_HELP_MESSAGE}")
  message(
   FATAL_ERROR
   "assert_name_unique: The <NAME> argument must not be empty!"
  )
 endif()
 unset(anu_NAME_LENGTH)

 #Validate condition
 string(LENGTH "${anu_CONDITION}" anu_CONDITION_LENGTH)
 if(anu_CONDITION_LENGTH EQUAL 0)
  message("${anu_HELP_MESSAGE}")
  message(
   FATAL_ERROR
   "assert_name_unique: The <CONDITION> argument must not be empty!"
  )
 endif()
 unset(anu_CONDITION_LENGTH)

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
 Checks if any number of arguments are empty and places result in destination
 variable

 Note: Do not invoke `is_empty` for length checks in this function, to prevent
 infinite loops.
]]
assert_name_unique(
 is_empty
 COMMAND
 MESSAGE "Name collision: Function 'is_empty' is already defined elsewhere!"
)
function(is_empty ie_DESTINATION_VARIABLE)
 #Help message
 string(
  APPEND ie_HELP_MESSAGE
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

 #Validate destination variable
 string(LENGTH "${ie_DESTINATION_VARIABLE}" ie_DESTINATION_VARIABLE_LENGTH)
 if(ie_DESTINATION_VARIABLE_LENGTH EQUAL 0)
  message("${ie_HELP_MESSAGE}")
  message(
   FATAL_ERROR
   "is_empty: The <DESTINATION_VARIABLE> argument must not be empty!"
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
 Generates unique a name to avoid colisions, based on the value of `CONDITION`.
 Defaults to generating random strings of legnth 5, lowercase characters only
 and a search limit of 10000.
]]
assert_name_unique(
 generate_unique_name
 COMMAND
 MESSAGE
  "Name collision: Function 'generate_unique_name' is already defined "
  "elsewhere!"
)
function(generate_unique_name gun_NAME gun_CONDITION gun_DESTINATION_VARIABLE)
 #Help message
 string(
  APPEND gun_HELP_MESSAGE
  "'generate_unique_name' takes the following arguments:"
  "\n - (REQUIRED) <NAME>: The name to check for uniqueness"
  "\n - (REQUIRED) <CONDITION>: The condition to check the name against for "
  "uniqueness. Options are [COMMAND, VARIABLE, CACHE, ENV]"
  "\n - (REQUIRED) <DESTINATION_VARIABLE>: The name of the destination "
  "variable to place the uniquely generated name in"
  "\n - (OPTIONAL) 'LENGTH': The length of the random suffix for generating "
  "unique names. Defaults to 5"
  "\n - (OPTIONAL) 'ALPHABET': The character set to pull from when generating "
  "random suffixes. Must have at least one character. Defaults to "
  "'abcdefghijklmnopqrstuvwxyz'"
  "\n - (OPTIONAL) 'LIMIT': The iteration limit for searching for unique "
  "names. Defaults to 10000. Must be a positive integer greater than 0"
  "\n\nExamples:"
  "\n set(SOME_VARIABLE_NAME)"
  "\n generate_unique_name(SOME_VARIABLE_NAME VARIABLE UNIQUE_VARIABLE_NAME)"
  "\n message(\"\${UNIQUE_VARIABLE_NAME}\") #prints "
  "'SOME_VARIABLE_NAME_XXXXX' where 'XXXXX' are 5 randomly generated "
  "characters"
  "\n ---"
  "\n generate_unique_name(some_function_or_macro COMMAND "
  "UNIQUE_FUNCTION_NAME)"
  "\n message(\"\${UNIQUE_FUNCTION_NAME}\") #prints 'some_function_or_macro' "
  "since no function or macro definition exists with that name"
 )

 #Validate function name
 is_empty(gun_NAME_EMPTY "${gun_NAME}")
 if(gun_NAME_EMPTY)
  message("${gun_HELP_MESSAGE}")
  message(
   FATAL_ERROR
   "generate_unique_name: The <FUNCTION_NAME> argument must not be empty!"
  )
 endif()
 unset(gun_NAME_EMPTY)

 #Validate destination variable name
 is_empty(gun_DESTINATION_VARIABLE_EMPTY "${gun_DESTINATION_VARIABLE}")
 if(gun_DESTINATION_VARIABLE_EMPTY)
  message("${gun_HELP_MESSAGE}")
  message(
   FATAL_ERROR
   "generate_unique_name: The <DESTINATION_VARIABLE> argumnet must not be "
   "empty!"
  )
 endif()
 unset(gun_DESTINATION_VARIABLE_EMPTY)

 #Parse any overrides
 cmake_parse_arguments(
  gun
  ""
  "LENGTH;ALPHABET;LIMIT"
  ""
  ${ARGN}
 )

 #Determine random suffix length
 if(DEFINED gun_LENGTH)
  if(gun_LENGTH LESS 1)
   message("${gun_HELP_MESSAGE}")
   message(
    FATAL_ERROR
    "generate_unique_name: The <LENGTH> argument cannot have a value "
    "less than 1!"
   )
  endif()
 else()
  set(gun_LENGTH 5)
 endif()

 #Determine alphabet for random suffix
 if(NOT DEFINED gun_ALPHABET)
  set(gun_ALPHABET "abcdefghijklmnopqrstuvwxyz")
 else()
  #Validate user-supplied alphabet
  is_empty(gun_ALPHABET_EMPTY "${gun_ALPHABET}")
  if(gun_ALPHABET_EMPTY)
   message("${gun_HELP_MESSAGE}")
   message(
    FATAL_ERROR
    "generate_unique_name: The 'ALPHABET' argument must have at least one "
    "character!"
   )
  endif()
  unset(gun_ALPHABET_EMPTY)
 endif()

 #Determine search limit
 if(DEFINED gun_LIMIT)
  if(gun_LIMIT LESS 1)
   message("${gun_HELP_MESSAGE}")
   message(
    FATAL_ERROR
    "generate_unique_name: The 'LIMIT' argument cannot have a value less than "
    "1!"
   )
  endif()
 else()
  #Use default search limit
  set(gun_LIMIT 10000)
 endif()
 math(EXPR gun_LOOP_UPPER_LIMIT "${gun_LIMIT} + 1")

 #Validate condition
 list(APPEND gun_SUPPORTED_CONDITIONS COMMAND VARIABLE CACHE ENV)
 if(NOT "${gun_CONDITION}" IN_LIST gun_SUPPORTED_CONDITIONS)
  #Emit diagnostic if condition is invalid
  message("${gun_HELP_MESSAGE}")
  message(
   FATAL_ERROR
   "generate_unique_name: The value '${gun_CONDITION}' is not a valid "
   "<CONDITION> argument!"
  )
 endif()
 unset(gun_SUPPORTED_CONDITIONS)

 #Loop until random string is found, or limit is reached
 set(gun_UNIQUE_NAME_FOUND FALSE)
 set(gun_UNIQUE_NAME "${gun_NAME}")
 foreach(COUNT RANGE 1 ${gun_LOOP_UPPER_LIMIT})
  #If unique name found, stop searching
  is_name_unique(
   "${gun_UNIQUE_NAME}"
   ${gun_CONDITION}
   gun_UNIQUE_NAME_FOUND
  )
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
 Generates a prefix based on the current `project()` scope for storing
 project-related information on
]]
assert_name_unique(
 get_project_prefix
 COMMAND
 "Name collision: Function 'get_project_prefix' command already defined "
 "elsewhere!"
)
function(get_project_prefix gpp_DESTINATION_VARIABLE)
 #Help message
 string(
  APPEND gpp_HELP_MESSAGE
  "'get_project_prefix' takes the following arguments:"
  "\n - (REQUIRED) <DESTINATION_VARIABLE>: The name of the variable to place "
  "the project prefix in"
  "\n\nExamples:"
  "\n project(my_project)"
  "\n get_project_prefix(project_prefix)"
  "\n message(\"\${project_prefix}\") #prints 'my_project'"
  "\n ---"
  "\n #Notice there is no `project()` invocation"
  "\n get_project_prefix(project_prefix)"
  "\n message(\"\${project_prefix}\") #prints 'NO_PROJECT'"
 )

 #Validate destination variable name
 is_empty(gpp_DESTINATION_VARIABLE_EMPTY "${gpp_DESTINATION_VARIABLE}")
 if(gpp_DESTINATION_VARIABLE_EMPTY)
  message("${gpp_HELP_MESSAGE}")
  message(
   FATAL_ERROR
   "get_project_prefix: The <DESTINATION_VARIABLE> argument must not be empty!"
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
