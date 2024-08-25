cmake_minimum_required(VERSION 3.18)

project(example_cpp)

add_executable(example_cpp main.cpp)

target_link_libraries(example_cpp babylon)

project(example_c)

add_executable(example_c main.c)

target_link_libraries(example_c babylon)