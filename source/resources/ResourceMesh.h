#pragma once
#include "Resource.h"
#include "../geometry/Vertex.h"
#include "../utils/AABB.h"
#include <vector>

// Estructuras para guardar los IDs de OpenGL
struct MeshData
{
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;
};

struct StencilData
{
	unsigned int VAO = 0;
	unsigned int VBO = 0;
};

class ResourceMesh : public Resource
{
public:
	ResourceMesh(UID uid);
	virtual ~ResourceMesh();

	bool LoadToMemory_Internal() override;
	bool UnloadFromMemory_Internal() override;

public:
	MeshData meshData;
	StencilData stencilData;

	uint32_t numVertices = 0;
	uint32_t numIndices = 0;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	AABB localAABB;

	bool hasUVs = false;

private:

	bool GenerateBuffers();
	bool GenerateStencilBuffers();
};