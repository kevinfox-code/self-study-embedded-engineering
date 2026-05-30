cmake_minimum_required(VERSION 3.10)
project(TestProject)

enable_testing()

add_executable(test_hello test_hello.c)

add_test(NAME HelloWorldTest COMMAND test_hello)