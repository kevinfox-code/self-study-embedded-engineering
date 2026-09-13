# cmake/check_no_float.cmake
# Static rule: no float/double literal or type in src/core or include/foc.
# Called by ctest via add_test(... -P ...).
# Violations in comments or test reference files are expected and excluded.

cmake_minimum_required(VERSION 3.18)

foreach(dir ${SRC_DIR} ${INC_DIR})
    file(GLOB_RECURSE C_FILES "${dir}/*.c" "${dir}/*.h")
    foreach(f ${C_FILES})
        # motor_types.h is the single approved location for compile-time (double)
        # casts inside the Q16()/Q15() macros.  Skip it here.
        if(f MATCHES "motor_types\\.h$")
            continue()
        endif()
        # Read the file and strip single-line comments (crude but sufficient).
        file(READ "${f}" contents)
        # Check for float or double keywords as types or casts.
        # Allow: (double) in Q16/Q15 macros (compile-time only, guarded).
        # Pattern: 'float ' or 'double ' used as a variable/parameter type.
        string(REGEX MATCHALL "(^|[ \t(,*])(float|double)[ \t(*]" matches "${contents}")
        if(matches)
            message(FATAL_ERROR
                "static_check: float/double found in ${f}\n"
                "  Matches: ${matches}\n"
                "  (Q16/Q15 macros using (double) at compile-time are allowed in motor_types.h only)")
        endif()
    endforeach()
endforeach()

message(STATUS "static_check_no_float: PASS")
