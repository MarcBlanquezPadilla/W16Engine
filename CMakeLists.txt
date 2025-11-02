cmake_minimum_required(VERSION 3.10)

project(W16Engine)

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(DevIL REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

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
	source/Scene.cpp
	source/Scene.h
	source/Editor.cpp
	source/Editor.h
	source/GameObject.cpp
	source/GameObject.h
	source/Loader.cpp
	source/Loader.h
	source/components/Component.h
	source/components/Mesh.cpp
	source/components/Mesh.h
	source/components/Transform.cpp
	source/components/Transform.h
	source/components/Texture.cpp
	source/components/Texture.h
	source/windows/UIWindow.h
	source/windows/ConfigWindow.cpp
	source/windows/ConfigWindow.h
	source/windows/ConsoleWindow.cpp
	source/windows/ConsoleWindow.h
	source/windows/AboutWindow.cpp
	source/windows/AboutWindow.h
	source/windows/HierarchyWindow.cpp
	source/windows/HierarchyWindow.h
	source/windows/InspectorWindow.cpp
	source/windows/InspectorWindow.h
)

target_link_libraries(W16Engine PRIVATE 
	SDL3::SDL3
	glad::glad
	assimp::assimp
	DevIL::IL
	glm::glm
	imgui::imgui
)

install(DIRECTORY Assets
        DESTINATION bin
        COMPONENT Runtime)