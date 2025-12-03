#pragma once
#include <cstdint>

using UID = uint32_t;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_SCALE 1

#define VERSION "1.0"
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

#define EMPTY 0
#define CUBE 1
#define PYRAMID 2
#define SPHERE 3

#define DEBUG_R 0.0f
#define DEBUG_G 1.0f
#define DEBUG_B 0.0f
#define DEBUG_A 1.0f

#define DEBUG_COLOR 0.0f, 1.0f, 0.0f, 1.0f
#define GRID_COLOR 0.7f,0.7f,0.7f,0.2f
#define STENCIL_COLOR 0.0f,1.0f,1.0f,1.0f
#define CAMERA_COLOR 1.0f,1.0f,1.0f,1.0f
