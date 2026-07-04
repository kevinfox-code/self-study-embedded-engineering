# cmake/check_no_constants_h.cmake
# Static rule: no #include "constants.h" in src/core (Layer A/B files).
# constants.h is a Layer C/D HAL bridge — must not appear in core.

cmake_minimum_required(VERSION 3.18)

file(GLOB_RECURSE C_FILES "${SRC_DIR}/*.c" "${SRC_DIR}/*.h")
foreach(f ${C_FILES})
    file(READ "${f}" contents)
    string(FIND "${contents}" "constants.h" pos)
    if(NOT pos EQUAL -1)
        message(FATAL_ERROR
            "static_check: #include \"constants.h\" found in Layer A/B file: ${f}\n"
            "  constants.h is a Layer C/D HAL bridge and must not appear in src/core.")
    endif()
endforeach()

message(STATUS "static_check_no_constants_h: PASS")
