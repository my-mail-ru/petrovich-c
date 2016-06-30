project(petrovich C)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-Werror")
add_executable(petr_test petrovich.c test.c)
target_link_libraries(petr_test yaml)
