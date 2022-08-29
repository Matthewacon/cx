#[[
 This file contains all of the configurations for unit and integration tests,
 as well as benchmarks.
]]

include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/flags.cmake)

#[[Unit tests]]
if(CX_UNIT_TESTS)
 message(WARNING "Unit tests are unimplemented!")
endif()
mark_build_flag_as_processed(CX_UNIT_TESTS)

#[[Integration tests]]
if(CX_INTEGRATION_TESTS)
 message(WARNING "Integration tests are unimplemented!")
endif()
mark_build_flag_as_processed(CX_INTEGRATION_TESTS)

#[[Benchmarks]]
if(CX_BENCHMARKS)
 message(WARNING "Benchmarks are unimplemented!")
endif()
mark_build_flag_as_processed(CX_BENCHMARKS)
