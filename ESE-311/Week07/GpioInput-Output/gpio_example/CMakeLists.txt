cmake_minimum_required(VERSION 3.20)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_COMPILER_ID GNU)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(gpio-input-output C ASM)

set(TARGET gpio_input_output)
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32U575ZITXQ_FLASH.ld)

set(CPU_FLAGS -mcpu=cortex-m33 -mthumb)

add_subdirectory(CMSIS)

add_executable(${TARGET}
    Src/main.c
    Src/gpio.c
    startup_stm32u575zitxq.s
)

target_include_directories(${TARGET} PRIVATE Inc)

target_link_libraries(${TARGET} PRIVATE cmsis_headers)

target_compile_options(${TARGET} PRIVATE
    ${CPU_FLAGS}
)

target_link_options(${TARGET} PRIVATE
    ${CPU_FLAGS}
    -T${LINKER_SCRIPT}
    --specs=nosys.specs
    -Wl,-Map=${TARGET}.map
    -Wl,--gc-sections
)

set_target_properties(${TARGET} PROPERTIES SUFFIX ".elf")

add_custom_target(flash
    COMMAND openocd
        -f interface/stlink.cfg
        -f target/stm32u5x.cfg
        -c "program ${TARGET}.elf verify reset exit"
    DEPENDS ${TARGET}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Flashing via OpenOCD"
)
