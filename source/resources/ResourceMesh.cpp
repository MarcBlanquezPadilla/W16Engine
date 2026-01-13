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
		LOG(LogType::LOG_ERROR, "Could not open mesh library file: %s", libraryPath.c_str());
		return false;
	}

	UID fileUID = 0;
	int fileType = 0;

	// 1. LEER CABECERA
	file.read(reinterpret_cast<char*>(&fileUID), sizeof(UID));
	file.read(reinterpret_cast<char*>(&fileType), sizeof(int));

	if (fileUID != this->uid)
	{
		LOG(LogType::LOG_ERROR, "UID Mismatch in mesh. Expected %u, got %u", this->uid, fileUID);
		file.close(); return false;
	}

	// 2. LEER TAMAÑOS (Ahora son 3)
	// Necesitamos una variable temporal para bones porque no es miembro directo de la clase (es el tamaño del vector)
	uint32_t numBones = 0;

	file.read(reinterpret_cast<char*>(&numVertices), sizeof(uint32_t));
	file.read(reinterpret_cast<char*>(&numIndices), sizeof(uint32_t));
	file.read(reinterpret_cast<char*>(&numBones), sizeof(uint32_t)); // --- NUEVO ---

	if (numVertices == 0)
	{
		LOG(LogType::LOG_ERROR, "Error: Mesh has 0 vertices.");
		file.close(); return false;
	}

	// 3. REDIMENSIONAR VECTORES
	vertices.resize(numVertices);
	indices.resize(numIndices);
	bones.resize(numBones); // --- NUEVO: Asegúrate de tener std::vector<BoneInfo> bones en el .h

	// 4. LEER DATOS MASIVOS (Vértices e Índices)
	// Nota: sizeof(Vertex) ahora incluye automáticamente boneIDs y weights, así que esto leerá todo de golpe.
	file.read(reinterpret_cast<char*>(vertices.data()), numVertices * sizeof(Vertex));
	file.read(reinterpret_cast<char*>(indices.data()), numIndices * sizeof(unsigned int));

	// 5. LEER HUESOS (--- BLOQUE NUEVO ---)
	// No podemos leerlo de golpe (.read total) porque los strings tienen tamaño variable.
	for (unsigned int i = 0; i < numBones; i++)
	{
		// A. Tamaño del nombre
		uint32_t nameSize = 0;
		file.read(reinterpret_cast<char*>(&nameSize), sizeof(uint32_t));

		// B. Caracteres del nombre
		if (nameSize > 0)
		{
			bones[i].name.resize(nameSize);
			file.read(&bones[i].name[0], nameSize);
		}

		// C. Matriz Offset (Bind Pose inversa)
		file.read(reinterpret_cast<char*>(&bones[i].offsetMatrix), sizeof(glm::mat4));
	}

	file.close();

	// 6. CÁLCULOS FINALES (AABB)
	localAABB.SetNegativeInfinity();
	for (const auto& v : vertices)
	{
		localAABB.Enclose(v.position);
	}

	localAABB.min -= glm::vec3(AABB_PADDING);
	localAABB.max += glm::vec3(AABB_PADDING);

	// 7. GENERAR BUFFERS GPU
	// Importante: Asegúrate de que GenerateBuffers() configura los atributos 3 (IDs) y 4 (Pesos)
	if (!GenerateBuffers()) return false;

	// Si usas Stencil o algo extra
	GenerateStencilBuffers();

	LOG(LogType::LOG_INFO, "Mesh loaded: %s (Bones: %d)", libraryPath.c_str(), numBones);
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
	for (const auto& v : vertices)
	{
		Vertex newV;

		newV.position = v.position;
		newV.texCoords = v.texCoords;
		newV.normal = accumulatedNormals[v.position];

		memcpy(newV.boneIDs, v.boneIDs, sizeof(int) * MAX_BONE_INFLUENCE);
		memcpy(newV.weights, v.weights, sizeof(float) * MAX_BONE_INFLUENCE);

		smothedVertices.push_back(newV);
	}

	return Engine::GetInstance().moduleRender->UploadSmoothedMeshToGPU(
		stencilData,
		meshData.EBO,
		smothedVertices
	);
}