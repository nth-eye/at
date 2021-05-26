cmake_minimum_required(VERSION 3.0.0)
project(at VERSION 0.1.0)

include_directories(inc)
file(GLOB SRC_FILES src/*.c)
add_executable(at main.c ${SRC_FILES}) 

set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Wno-unused-parameter")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
