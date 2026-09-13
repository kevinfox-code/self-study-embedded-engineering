# cmake/host.cmake — Host (x86-64/aarch64) build settings.
# Selected automatically when FOC_HOST=1 is set.

set(FOC_HOST 1)

# Use the system C compiler (gcc or clang on the host).
# No cross-compiler prefix needed.

# Strict warning flags for host CI.
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wshadow
    -Wundef
    -pedantic
    -std=c99
)

# Math library required by test/reference and test/sim (float RK4 model).
link_libraries(m)

message(STATUS "FOC host build selected (${CMAKE_C_COMPILER})")
