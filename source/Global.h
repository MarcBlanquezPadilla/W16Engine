#pragma once
#include <cstdint>
#include <random>

using UID = uint32_t;

static UID GenerateNewUID()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
	return dis(gen);
}

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_SCALE 1

#define VERSION "2.0"
#define NAME "W16 Motor"
#define DEVELOPER_1 "Marc Blanquez"
#define DEVELOPER_2 "Arnau Balasch"
#define DEVELOPER_3 "Marti Mira"
#define LIBRARY_1 "SDL 3.0"
#define LIBRARY_2 "GLAD (OpenGL 4.6)"
#define LIBRARY_3 "Assimp (3D Model Loading)"
#define LIBRARY_4 "DevIL (Image Loading)"
#define LIBRARY_5 "ImGui (Editor UI)"
#define LIBRARY_6 "GLM (Math)"

#define CUBE 1
#define PYRAMID 2
#define SPHERE 3
#define ICON_FILE 4
#define ICON_FOLDER 5
#define ICON_IMAGE 6
#define ICON_MESH 7
#define ICON_MODEL 8
#define ICON_SCENE 9
#define ICON_SCRIPT 10
#define ICON_ANIMATION 11

#define GAMEOBJECTS_DRAG "OBJECTS_DRAG"
#define ASSETS_DRAG "ASSETS_DRAG"
#define	RESOURCE_DRAG "RESOURCE_DRAG"

#define DEBUG_R 0.0f
#define DEBUG_G 1.0f
#define DEBUG_B 0.0f
#define DEBUG_A 1.0f

#define DEBUG_COLOR 0.0f, 1.0f, 0.0f, 1.0f
#define GRID_COLOR 0.7f,0.7f,0.7f,0.2f
#define STENCIL_COLOR 0.0f,1.0f,1.0f,1.0f
#define CAMERA_COLOR 1.0f,1.0f,1.0f,1.0f

#define MAX_BONE_INFLUENCE 4
