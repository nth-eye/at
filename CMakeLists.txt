cmake_minimum_required(VERSION 3.0.0)
project(at VERSION 0.1.0)

include(CTest)
enable_testing()

file(GLOB SRC_FILES src/*.c)

include_directories(inc)

add_executable(at main.c ${SRC_FILES}) 

set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic-errors -Wno-unused-parameter")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
