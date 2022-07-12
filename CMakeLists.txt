cmake_minimum_required(VERSION 3.0.0)
project(at VERSION 0.1.0)

add_library(libat INTERFACE)
target_include_directories(libat INTERFACE inc)
target_compile_features(libat INTERFACE cxx_std_11)
target_compile_options(libat INTERFACE "-Wall" "-Wextra" "-Wpedantic")

add_executable(at main.cpp)
target_link_libraries(at PRIVATE libat)