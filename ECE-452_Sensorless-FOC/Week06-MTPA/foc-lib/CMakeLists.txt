cmake_minimum_required(VERSION 3.18)

# ---------------------------------------------------------------------------
# Project
# ---------------------------------------------------------------------------
project(foc_lib C)

# C99 is the baseline standard for this library.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Build mode selection
# ---------------------------------------------------------------------------
option(FOC_HOST "Build for host (x86-64/aarch64) with tests" OFF)

if(FOC_HOST)
    include(cmake/host.cmake)
else()
    message(STATUS "FOC target (embedded) build — link libfoc.a into your application")
endif()

# ---------------------------------------------------------------------------
# FOC core library — integer-only, no HAL, no RTOS
# ---------------------------------------------------------------------------
set(FOC_CORE_SOURCES
    src/core/motor_math.c
    src/core/motor_math_lut.c
    src/core/motor_filter.c
    src/core/motor_pi.c
    src/core/motor_foc.c
    src/core/motor_observer.c
    src/core/motor_faults.c
    src/core/motor_sm.c
    src/core/motor_ident.c
    src/core/motor_modeler.c
    src/core/motor_tune.c
    src/core/motor_ctrl.c
    src/core/drv8323.c
    src/core/motor_hw_if.c
)

add_library(foc STATIC ${FOC_CORE_SOURCES})

target_include_directories(foc PUBLIC
    include
)

# On host: define FOC_HOST so source can gate HOST-only paths.
# FOC_TESTING enables the foc_sat_event() hook in the foc library TU.
if(FOC_HOST)
    target_compile_definitions(foc PUBLIC FOC_HOST=1 FOC_TESTING=1)
endif()

# ---------------------------------------------------------------------------
# Host-only: test executables
# ---------------------------------------------------------------------------
if(FOC_HOST)

    enable_testing()

    # Shared include path for test support headers.
    set(TEST_INCLUDE_DIRS
        include
        test
        test/sim
        test/reference
    )

    # -----------------------------------------------------------------------
    # Reference / simulation support objects (float permitted here).
    # -----------------------------------------------------------------------
    add_library(foc_ref OBJECT
        test/reference/ref_math.c
        test/reference/ref_observer.c
    )
    target_include_directories(foc_ref PUBLIC ${TEST_INCLUDE_DIRS})
    target_compile_definitions(foc_ref PRIVATE FOC_HOST=1)
    # Reference code uses double — suppress -Wconversion for this target only.
    target_compile_options(foc_ref PRIVATE -Wno-conversion)

    add_library(foc_sim OBJECT
        test/sim/sim_motor_model.c
        test/sim/hw_if_mock.c
        test/sim/replay.c
    )
    target_include_directories(foc_sim PUBLIC ${TEST_INCLUDE_DIRS})
    target_compile_definitions(foc_sim PRIVATE FOC_HOST=1)
    target_compile_options(foc_sim PRIVATE -Wno-conversion)

    # -----------------------------------------------------------------------
    # Helper macro: add a test executable linked against foc + ref + sim.
    # -----------------------------------------------------------------------
    macro(foc_add_test name)
        add_executable(${name} ${ARGN})
        target_include_directories(${name} PRIVATE ${TEST_INCLUDE_DIRS})
        target_compile_definitions(${name} PRIVATE FOC_HOST=1)
        target_link_libraries(${name} PRIVATE foc $<TARGET_OBJECTS:foc_ref> $<TARGET_OBJECTS:foc_sim> m)
        add_test(NAME ${name} COMMAND ${name})
    endmacro()

    # -----------------------------------------------------------------------
    # Unit tests (Phase 1–9)
    # -----------------------------------------------------------------------
    foc_add_test(test_motor_math       test/unit/test_motor_math.c)
    foc_add_test(test_trig             test/unit/test_trig.c)
    foc_add_test(test_motor_filter     test/unit/test_motor_filter.c)
    foc_add_test(test_motor_pi         test/unit/test_motor_pi.c)
    foc_add_test(test_transforms       test/unit/test_transforms.c)
    foc_add_test(test_limits_sat       test/unit/test_limits_sat.c)
    foc_add_test(test_drv8323_regs     test/unit/test_drv8323_regs.c)
    foc_add_test(test_hw_cal           test/unit/test_hw_cal.c)
    foc_add_test(test_modeler_match    test/unit/test_modeler_match.c)

    # -----------------------------------------------------------------------
    # Module tests (Phase 7 onwards)
    # -----------------------------------------------------------------------
    foc_add_test(test_faults           test/module/test_faults.c)

    # -----------------------------------------------------------------------
    # Static-rule checks (run as ctest scripts, not compiled tests).
    # Enforce: no float/double in src/ or include/foc/; no constants.h in core.
    # -----------------------------------------------------------------------
    find_program(GREP_EXECUTABLE grep)
    if(GREP_EXECUTABLE)
        add_test(
            NAME static_check_no_float_in_src
            COMMAND ${CMAKE_COMMAND}
                -DGREP=${GREP_EXECUTABLE}
                -DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}/src/core
                -DINC_DIR=${CMAKE_CURRENT_SOURCE_DIR}/include/foc
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_no_float.cmake
        )
        add_test(
            NAME static_check_no_constants_in_core
            COMMAND ${CMAKE_COMMAND}
                -DGREP=${GREP_EXECUTABLE}
                -DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}/src/core
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_no_constants_h.cmake
        )
    endif()

endif() # FOC_HOST

# ---------------------------------------------------------------------------
# Install rules (for integration into application CMake projects)
# ---------------------------------------------------------------------------
install(TARGETS foc
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/foc
    DESTINATION include
)
