add_executable(led_sandbox main.c hal_host.c)
target_link_libraries(led_sandbox PRIVATE led_fsm)
