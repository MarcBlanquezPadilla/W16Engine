cmake_minimum_required(VERSION 3.10)

project(W16Motor)

find_package(fmt CONFIG REQUIRED)

add_executable(W16Motor W16Motor.cpp)

target_link_libraries(W16Motor PRIVATE fmt::fmt)