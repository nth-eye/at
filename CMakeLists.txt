cmake_minimum_required(VERSION 3.0.0)
project(at VERSION 0.1.0)

add_library(at INTERFACE)
target_include_directories(at INTERFACE inc)
target_compile_features(at INTERFACE cxx_std_11)
target_compile_options(at INTERFACE "-Wall" "-Wextra" "-Wpedantic")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_executable(at_exec main.cpp)
    target_link_libraries(at_exec PRIVATE at)
endif()