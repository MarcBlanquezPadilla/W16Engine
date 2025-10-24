cmake_minimum_required(VERSION 3.10)

project(W16Engine)

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(DevIL REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(W16Engine
	source/Global.h
	source/application.cpp 
	source/Engine.cpp 
	source/Engine.h 
	source/Log.cpp 
	source/Log.h
	source/Timer.cpp
	source/Timer.h
	source/Module.h
	source/Input.cpp
	source/Input.h
	source/Window.cpp
	source/Window.h
	source/Vector2D.cpp
	source/Vector2D.h
	source/Render.cpp
	source/Render.h
	source/Camera.cpp
	source/Camera.h
)

target_link_libraries(W16Engine PRIVATE 
	SDL3::SDL3
	glad::glad
	assimp::assimp
	DevIL::IL
	glm::glm
)