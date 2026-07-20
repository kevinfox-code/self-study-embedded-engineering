# App/CMakeLists.txt — standalone FOC application build (NOT added from the
# root CMakeLists; the root bare-metal project is untouched).
#
# Compose recipe (after generating the CubeMX profile into
# Profiles/NucleoU575/ per foc-lib/docs/cubemx_setup_guide.md):
#
#   cmake -S App -B App/build -DAPP_HAVE_CUBEMX_PROFILE=ON
#
# (Profiles/ lives at the workspace root, a sibling of App/ and foc-lib/,
# so it stays independent of which App-like tree consumes it.)
#   cmake --build App/build
#
# Today (no profile yet):
#
#   cmake -S App -B App/build && cmake --build App/build
#
# builds `app_stub_check` — a host compile of the portable App sources —
# plus libfoc.a, proving the scaffold and the constants.h bridge compile.

cmake_minimum_required(VERSION 3.20)

option(APP_HAVE_CUBEMX_PROFILE "Build the firmware against a generated CubeMX profile" OFF)

if(APP_HAVE_CUBEMX_PROFILE)
    # Cross build — mirror the root project's toolchain conventions.
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

project(foc_app VERSION 0.1.0 LANGUAGES C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BOARD_VARIANT BOARD_NUCLEO_U575)
set(PROFILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Profiles/NucleoU575)
set(FOC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../foc-lib)
set(FOC_PORT_DIR ${FOC_LIB_DIR}/src/port/stm32u5)

# Portable core library (integer-only, no HAL) — target `foc`.
add_subdirectory(${FOC_LIB_DIR} foc-lib)

# Hand-written, board-agnostic application sources.
set(APP_PORTABLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/SystemInit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Entry/CoreAppMain.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Tasks/app_tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/RTOS/rtos_hooks.c
)

# foc-lib port layer: compiled by the application (it resolves
# constants.h from App/ via the include path below).
set(FOC_PORT_SOURCES
    ${FOC_PORT_DIR}/board_support.c
    ${FOC_PORT_DIR}/motor_hw_if.c
    ${FOC_PORT_DIR}/drv8323_transport.c
    ${FOC_PORT_DIR}/isr_motor.c
    ${FOC_PORT_DIR}/freertos_tasks.c
)

set(APP_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}            # constants.h, SystemInit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BSP
    ${CMAKE_CURRENT_SOURCE_DIR}/Entry
    ${CMAKE_CURRENT_SOURCE_DIR}/Tasks
    ${FOC_PORT_DIR}                        # board_support.h
)

if(APP_HAVE_CUBEMX_PROFILE)
    # CubeMX-generated sources (fill in after Generate Code; never
    # hand-edit them outside USER CODE markers).
    file(GLOB APP_PROFILE_SOURCES
        ${PROFILE_DIR}/Core/Src/*.c
        # TODO after generation, add as produced by CubeMX:
        #   ${PROFILE_DIR}/Drivers/STM32U5xx_HAL_Driver/Src/*.c
        #   ${PROFILE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/...
        #   ${PROFILE_DIR}/startup_stm32u575zitxq.s
    )

    set(CPU_FLAGS -mcpu=cortex-m33 -mthumb)

    add_executable(foc_app
        ${APP_PORTABLE_SOURCES}
        ${FOC_PORT_SOURCES}
        ${APP_PROFILE_SOURCES}
    )
    target_include_directories(foc_app PRIVATE
        ${APP_INCLUDE_DIRS}
        ${PROFILE_DIR}/Core/Inc
        # TODO after generation: HAL/CMSIS/FreeRTOS include dirs from the profile.
    )
    target_compile_definitions(foc_app PRIVATE
        ${BOARD_VARIANT}
        APP_HAVE_CUBEMX_PROFILE
    )
    target_compile_options(foc_app PRIVATE ${CPU_FLAGS} -Wall -Wextra)
    target_link_libraries(foc_app PRIVATE foc)
    target_link_options(foc_app PRIVATE
        ${CPU_FLAGS}
        # TODO after generation: -T${PROFILE_DIR}/<linker>.ld
        --specs=nosys.specs
        -Wl,--gc-sections
        -Wl,-Map=${CMAKE_BINARY_DIR}/foc_app.map
    )
    set_target_properties(foc_app PROPERTIES SUFFIX ".elf")
else()
    message(STATUS "APP_HAVE_CUBEMX_PROFILE=OFF: building host stub check only. "
                   "Generate the CubeMX profile into ${PROFILE_DIR} and reconfigure "
                   "with -DAPP_HAVE_CUBEMX_PROFILE=ON to build the firmware.")

    # Host compile of the portable App sources — the scaffold's CI check.
    add_library(app_stub_check OBJECT ${APP_PORTABLE_SOURCES})
    target_include_directories(app_stub_check PRIVATE ${APP_INCLUDE_DIRS})
    target_compile_definitions(app_stub_check PRIVATE ${BOARD_VARIANT})
    target_compile_options(app_stub_check PRIVATE -Wall -Wextra)
    target_link_libraries(app_stub_check PRIVATE foc)
endif()
