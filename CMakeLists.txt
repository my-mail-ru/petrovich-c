project(petrovich C)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -Wmissing-declarations")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Werror")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DNDEBUG")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fvisibility=hidden")

# petrovich-c library
add_library(petrovich SHARED lib/petrovich.c lib/utf8.c)
target_include_directories(petrovich PUBLIC include)
target_link_libraries(petrovich yaml)

# Sample test program
add_executable(petr_test test/test.c)
target_include_directories(petr_test PRIVATE include)
target_link_libraries(petr_test petrovich)
