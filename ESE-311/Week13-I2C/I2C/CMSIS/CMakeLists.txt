cmake_minimum_required(VERSION 3.16)
project(CMSIS LANGUAGES C)

add_library(cmsis_headers INTERFACE)

target_include_directories(cmsis_headers INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/Include
    ${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32U5xx/Include
)
