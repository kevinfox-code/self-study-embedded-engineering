# cmake/toolchain-arm-gcc.cmake — ARM Cortex-M33 bare-metal toolchain.
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-gcc.cmake ..

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Toolchain prefix — override with -DARM_TOOLCHAIN_PREFIX=... if needed.
if(NOT ARM_TOOLCHAIN_PREFIX)
    set(ARM_TOOLCHAIN_PREFIX "arm-none-eabi-")
endif()

set(CMAKE_C_COMPILER   ${ARM_TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${ARM_TOOLCHAIN_PREFIX}gcc)
set(CMAKE_AR           ${ARM_TOOLCHAIN_PREFIX}ar)
set(CMAKE_RANLIB       ${ARM_TOOLCHAIN_PREFIX}ranlib)
set(CMAKE_OBJCOPY      ${ARM_TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE         ${ARM_TOOLCHAIN_PREFIX}size)

# Skip compiler ID check (no OS, bare-metal).
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# STM32U5 = Cortex-M33, FPU = FPv5-D16.
# The FOC library itself is integer-only (no FPU); the target flags are here
# so higher-level CMakeLists can link the library into a Cortex-M33 image.
set(CPU_FLAGS
    "-mcpu=cortex-m33"
    "-mthumb"
    "-mfpu=fpv5-sp-d16"
    "-mfloat-abi=hard"
)

string(JOIN " " CPU_FLAGS_STR ${CPU_FLAGS})
set(CMAKE_C_FLAGS_INIT   "${CPU_FLAGS_STR} -std=c99")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CPU_FLAGS_STR} -nostartfiles -Wl,--gc-sections")

# Release flags (size-optimized for flash-constrained target).
set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG   "-Og -g3")

message(STATUS "FOC ARM GCC toolchain: ${CMAKE_C_COMPILER}")
