cmake_minimum_required(VERSION 3.10)

project(W16Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
	source/ModuleInput.cpp
	source/ModuleInput.h
	source/ModuleWindow.cpp
	source/ModuleWindow.h
	source/ModuleLoader.cpp
	source/ModuleLoader.h
	source/ModuleEditor.cpp
	source/ModuleEditor.h
	source/ModuleScene.cpp
	source/ModuleScene.h
	source/ModuleEvents.cpp
	source/ModuleEvents.h
	source/ModuleResources.cpp
	source/ModuleResources.h
	source/Vector2D.cpp
	source/Vector2D.h
	source/ModuleRender.cpp
	source/ModuleRender.h
	source/EditorCamera.cpp
	source/EditorCamera.h
	source/CameraLens.cpp
	source/CameraLens.h
	source/GameObject.cpp
	source/GameObject.h
	source/Interface.cpp
	source/Interface.h
	source/Event.h
	source/EventListener.h
	source/components/Component.h
	source/components/Mesh.cpp
	source/components/Mesh.h
	source/components/Transform.cpp
	source/components/Transform.h
	source/components/Texture.cpp
	source/components/Texture.h
	source/components/Camera.cpp
	source/components/Camera.h
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
	source/windows/ProjectWindow.cpp
	source/windows/ProjectWindow.h
	source/windows/SceneWindow.cpp
	source/windows/SceneWindow.h
	source/windows/GameWindow.cpp
	source/windows/GameWindow.h
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
	source/utils/FileUtils.cpp
	source/utils/FileUtils.h
	source/utils/Config.cpp
	source/utils/Config.h
	source/geometry/Plane.h
	source/geometry/Plane.cpp
	source/resources/Resource.h
	source/resources/Resource.cpp
	source/importers/Importer.h
	source/importers/ImporterTexture.h
	source/importers/ImporterTexture.cpp
)

target_link_libraries(W16Engine PRIVATE 
	SDL3::SDL3
	glad::glad
	assimp::assimp
	DevIL::IL
	DevIL::ILU
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

install(DIRECTORY Resources
        DESTINATION bin
        COMPONENT Runtime)