cmake_minimum_required(VERSION 3.18)

project(babylon_example)

add_executable(babylon_example main.c)

target_link_libraries(babylon_example babylon)