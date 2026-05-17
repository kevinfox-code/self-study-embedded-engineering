add_executable(test_led_fsm test_led_fsm.c hal_mock.c)
target_link_libraries(test_led_fsm PRIVATE led_fsm)
add_test(NAME led_fsm_tests COMMAND test_led_fsm)
