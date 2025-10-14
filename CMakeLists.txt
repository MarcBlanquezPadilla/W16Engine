cmake_minimum_required(VERSION 3.10)

project(W16Engine)

find_package(SDL3 REQUIRED)

add_executable(W16Engine 
	source/application.cpp 
	source/Engine.cpp 
	source/Engine.h 
	source/Log.cpp 
	source/Log.h
	source/Timer.cpp
	source/Timer.h
	source/Module.h
	source/Window.cpp
	source/Window.h)

target_link_libraries(W16Engine PRIVATE 
	SDL3::SDL3)