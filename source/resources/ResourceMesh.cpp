#include "ResourceMesh.h"
#include "../Engine.h"
#include "../ModuleRender.h" 
#include "../utils/Log.h"
#include <fstream>
#include <algorithm>
#include <map>
#include <glm/glm.hpp>

struct Vec3Comparator {
	bool operator()(const glm::vec3& a, const glm::vec3& b) const {
		if (a.x != b.x) return a.x < b.x;
		if (a.y != b.y) return a.y < b.y;
		return a.z < b.z;
	}
};

ResourceMesh::ResourceMesh(UID uid) : Resource(uid, Resource::Type::mesh)
{
}

ResourceMesh::~ResourceMesh()
{
	if (IsLoadedToMemory())
		UnloadFromMemory_Internal();
}

bool ResourceMesh::LoadToMemory_Internal()
{
	std::ifstream file(libraryPath, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		LOG("Error: Could not open mesh library file: %s", libraryPath.c_str());
		return false;
	}

	UID fileUID = 0;
	int fileType = 0;

	file.read(reinterpret_cast<char*>(&fileUID), sizeof(UID));
	file.read(reinterpret_cast<char*>(&fileType), sizeof(int));

	if (fileUID != this->uid)
	{
		LOG("Error: UID Mismatch in mesh. Expected %u, got %u", this->uid, fileUID);
		file.close(); return false;
	}

	file.read(reinterpret_cast<char*>(&numVertices), sizeof(uint32_t));
	file.read(reinterpret_cast<char*>(&numIndices), sizeof(uint32_t));

	if (numVertices == 0)
	{
		LOG("Error: Mesh has 0 vertices.");
		file.close(); return false;
	}

	vertices.resize(numVertices);
	indices.resize(numIndices);

	file.read(reinterpret_cast<char*>(vertices.data()), numVertices * sizeof(Vertex));
	file.read(reinterpret_cast<char*>(indices.data()), numIndices * sizeof(unsigned int));

	file.close();

	localAABB.SetNegativeInfinity();
	for (const auto& v : vertices)
	{
		localAABB.Enclose(v.position);
	}

	if (!GenerateBuffers()) return false;

	GenerateStencilBuffers();

	LOG("Mesh loaded: %s", libraryPath.c_str());
	return true;
}

bool ResourceMesh::UnloadFromMemory_Internal()
{
	if (meshData.VAO != 0) Engine::GetInstance().moduleRender->DeleteMeshFromGPU(meshData);
	if (stencilData.VAO != 0) Engine::GetInstance().moduleRender->DeleteSmoothedMeshFromGPU(stencilData);

	vertices.clear();
	indices.clear();

	meshData = MeshData();
	stencilData = StencilData();
	numVertices = 0;
	numIndices = 0;

	return true;
}

bool ResourceMesh::GenerateBuffers()
{
	if (vertices.empty()) return false;

	bool success = Engine::GetInstance().moduleRender->UploadMeshToGPU(meshData, vertices, indices);

	if (success) hasUVs = true;
	return success;
}

bool ResourceMesh::GenerateStencilBuffers()
{
	std::vector<Vertex> smothedVertices;
	std::map<glm::vec3, glm::vec3, Vec3Comparator> accumulatedNormals;

	for (const auto& v : vertices) {
		accumulatedNormals[v.position] += v.normal;
	}
	for (auto& pair : accumulatedNormals) {
		pair.second = glm::normalize(pair.second);
	}
	smothedVertices.reserve(vertices.size());
	for (const auto& v : vertices) {
		smothedVertices.push_back({
			v.position,
			accumulatedNormals[v.position],
			v.texCoords
			});
	}

	return Engine::GetInstance().moduleRender->UploadSmoothedMeshToGPU(
		stencilData,
		meshData.EBO,
		smothedVertices
	);
}