cmake_minimum_required(VERSION 3.10)

project(W16Engine)

find_package(fmt CONFIG REQUIRED)

add_executable(W16Engine application.cpp)

target_link_libraries(W16Engine PRIVATE fmt::fmt)