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
find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(W16_ENGINE_SOURCES
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
	source/ModulePhysics.h
	source/ModulePhysics.cpp
	source/ModuleResources.cpp
	source/ModuleResources.h
	source/ModuleTime.cpp
	source/ModuleTime.h
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
	source/components/MeshRenderer.cpp
	source/components/MeshRenderer.h
	source/components/SkinnedMeshRenderer.cpp
	source/components/SkinnedMeshRenderer.h
	source/components/Transform.cpp
	source/components/Transform.h
	source/components/Camera.cpp
	source/components/Camera.h
	source/components/Animation.cpp
	source/components/Animation.h
	source/components/Rigidbody.h
	source/components/Rigidbody.cpp
	source/components/Collider.h
	source/components/Collider.cpp
	source/components/BoxCollider.h
	source/components/BoxCollider.cpp
	source/components/SphereCollider.h
	source/components/SphereCollider.cpp
	source/components/CapsuleCollider.h
	source/components/CapsuleCollider.cpp
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
	source/windows/ToolbarWindow.cpp
	source/windows/ToolbarWindow.h
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
	source/utils/Time.h
	source/geometry/Plane.h
	source/geometry/Plane.cpp
	source/geometry/Vertex.h
	source/geometry/Bone.h
	source/resources/Resource.h
	source/resources/Resource.cpp
	source/resources/ResourceUser.h
	source/resources/ResourceTexture.h
	source/resources/ResourceTexture.cpp
	source/resources/ResourceScene.h
	source/resources/ResourceScene.cpp
	source/resources/ResourceModel.h
	source/resources/ResourceModel.cpp
	source/resources/ResourceMesh.h
	source/resources/ResourceMesh.cpp
	source/resources/ResourceAnimation.h
	source/resources/ResourceAnimation.cpp
	source/importers/Importer.h
	source/importers/Importer.cpp
	source/importers/ImporterTexture.h
	source/importers/ImporterTexture.cpp
	source/importers/ImporterScene.h
	source/importers/ImporterScene.cpp
	source/importers/ImporterMesh.h
	source/importers/ImporterMesh.cpp
	source/importers/ImporterAnimation.h
	source/importers/ImporterAnimation.cpp
	source/importers/ImporterModel.h
	source/importers/ImporterModel.cpp
)

add_executable(W16Engine ${W16_ENGINE_SOURCES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/source" PREFIX "Source Files" FILES ${W16_ENGINE_SOURCES})

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
	unofficial::omniverse-physx-sdk::sdk
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