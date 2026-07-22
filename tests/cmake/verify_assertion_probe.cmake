if(NOT DEFINED PROBE)
    message(FATAL_ERROR "PROBE is required")
endif()

execute_process(
    COMMAND "${PROBE}"
    RESULT_VARIABLE probe_result
    OUTPUT_VARIABLE probe_stdout
    ERROR_VARIABLE probe_stderr
)

if("${probe_result}" STREQUAL "0")
    message(FATAL_ERROR "Assertion probe unexpectedly succeeded")
endif()
if(NOT "${probe_stderr}" MATCHES "Assertion failed: false")
    message(FATAL_ERROR "Assertion probe did not report the expression")
endif()
if(NOT "${probe_stderr}" MATCHES "M2 assertion probe")
    message(FATAL_ERROR "Assertion probe did not report the message")
endif()
