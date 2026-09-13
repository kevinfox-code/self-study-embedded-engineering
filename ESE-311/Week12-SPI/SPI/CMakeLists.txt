cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_COMPILER_ID GNU)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(spi VERSION 0.1.0 LANGUAGES C ASM)

# Feature toggles (compile-time)
option(DEBUG_ENABLED "Enable debug output and assertions" OFF)
option(FEATURE_UART_DMA "Enable interrupt-driven UART with DMA/ring buffers (non-blocking API)" OFF)
option(ENABLE_LINT "Enable static analysis with arm-none-eabi-gcc warnings" OFF)

set(TARGET spi)
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32U575ZITXQ_FLASH.ld)

# Build artifact directories
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(HEX_DIR ${CMAKE_BINARY_DIR}/hex)
set(MAP_DIR ${CMAKE_BINARY_DIR}/map)

file(MAKE_DIRECTORY ${BIN_DIR} ${HEX_DIR} ${MAP_DIR})

set(CPU_FLAGS -mcpu=cortex-m33 -mthumb)

# Build-type specific flags (Debug vs Release)
set(CMAKE_C_FLAGS_DEBUG "-g -O0" CACHE STRING "Debug C flags")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "Release C flags")

# Lint/analysis flags
if(ENABLE_LINT)
    set(LINT_FLAGS -Wall -Wextra -Wformat=2 -Wstrict-prototypes -Wwrite-strings -Wno-unused-parameter)
else()
    set(LINT_FLAGS -Wall)
endif()

add_subdirectory(CMSIS)

add_executable(${TARGET}
    Src/main.c
    Src/gpio.c
    Src/uart.c
    Src/adc.c
    Src/tim.c
    Src/spi.c
    Src/adxl345.c
    Src/ringbuffer.c
    Src/systick.c
    Src/debug.c
    Src/sysmem.c
    Src/syscalls.c
    CMSIS/Device/ST/STM32U5xx/Source/Templates/system_stm32u5xx.c
    startup_stm32u575zitxq.s
)

target_include_directories(${TARGET} PRIVATE Inc)

target_link_libraries(${TARGET} PRIVATE cmsis_headers)

target_compile_options(${TARGET} PRIVATE
    ${CPU_FLAGS}
    ${LINT_FLAGS}
)

# Apply feature toggles to compile definitions
if(DEBUG_ENABLED)
    target_compile_definitions(${TARGET} PRIVATE DEBUG_ENABLED=1)
else()
    target_compile_definitions(${TARGET} PRIVATE DEBUG_ENABLED=0)
endif()

if(FEATURE_UART_DMA)
    target_compile_definitions(${TARGET} PRIVATE FEATURE_UART_DMA=1)
else()
    target_compile_definitions(${TARGET} PRIVATE FEATURE_UART_DMA=0)
endif()

target_link_options(${TARGET} PRIVATE
    ${CPU_FLAGS}
    -T${LINKER_SCRIPT}
    --specs=nosys.specs
    -u _printf_float
    -Wl,-Map=${MAP_DIR}/${TARGET}.map
    -Wl,-u,_printf_float
    -Wl,--gc-sections
)

set_target_properties(${TARGET} PROPERTIES SUFFIX ".elf")

# Post-build: copy ELF to bin/, generate .hex and .bin
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET}> ${BIN_DIR}/${TARGET}.elf
    COMMAND arm-none-eabi-objcopy -O ihex $<TARGET_FILE:${TARGET}> ${HEX_DIR}/${TARGET}.hex
    COMMAND arm-none-eabi-objcopy -O binary $<TARGET_FILE:${TARGET}> ${BIN_DIR}/${TARGET}.bin
    COMMENT "Organizing artifacts: bin/, hex/, map/ (${CMAKE_BUILD_TYPE})"
)

add_custom_target(flash
    COMMAND openocd
        -f interface/stlink.cfg
        -f target/stm32u5x.cfg
        -c "program ${BIN_DIR}/${TARGET}.elf verify reset exit"
    DEPENDS ${TARGET}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Flashing via OpenOCD"
)

# Lint target: re-compile with strict warnings for analysis
add_custom_target(lint
    COMMAND ${CMAKE_COMMAND} -B ${CMAKE_BINARY_DIR}/lint -DENABLE_LINT=ON ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/lint
    COMMENT "Running static analysis with strict warnings"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
