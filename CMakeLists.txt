cmake_minimum_required(VERSION 3.10)

project(W16Engine)

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(DevIL REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ImGuizmo CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)

add_executable(W16Engine
	source/Global.h
	source/application.cpp 
	source/Engine.cpp 
	source/Engine.h 
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
	source/GameObject.cpp
	source/GameObject.h
	source/Loader.cpp
	source/Loader.h
	source/Editor.cpp
	source/Editor.h
	source/Interface.cpp
	source/Interface.h
	source/Event.h
	source/EventListener.h
	source/EventSystem.cpp
	source/EventSystem.h
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
	source/utils/Log.cpp 
	source/utils/Log.h
	source/utils/Timer.cpp
	source/utils/Timer.h
	source/utils/AABB.h
	source/utils/Ray.h
	source/utils/Ray.cpp
	source/utils/Frustum.h
	source/utils/Frustum.cpp
	source/utils/Tree.cpp
	source/utils/Tree.h
	source/geometry/Plane.h
	source/geometry/Plane.cpp
)

target_link_libraries(W16Engine PRIVATE 
	SDL3::SDL3
	glad::glad
	assimp::assimp
	DevIL::IL
	glm::glm
	imgui::imgui
	imguizmo::imguizmo
	pugixml::pugixml
)

install(DIRECTORY Assets
        DESTINATION bin
        COMPONENT Runtime)

install(DIRECTORY Library
        DESTINATION bin
        COMPONENT Runtime)