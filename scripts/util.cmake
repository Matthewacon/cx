cmake_minimum_required(VERSION 3.19)

function(is_empty ie_DESTINATION_VARIABLE)
 #Help message utility function
 function(print_help ie_ph_LEVEL)
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

 #Validate destination variable
 string(LENGTH "${ie_DESTINATION_VARIABLE}" ie_DESTINATION_VARIABLE_LENGTH)
 if(ie_DESTINATION_VARIABLE_LENGTH EQUAL 0)
  print_help(
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
