cmake_minimum_required(VERSION 3.16)
project(LedStateMachine C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# FSM logic — no HAL implementation; each executable supplies its own.
add_library(led_fsm STATIC src/led_fsm.c)
target_include_directories(led_fsm PUBLIC src)

add_subdirectory(sandbox)

enable_testing()
add_subdirectory(test)
