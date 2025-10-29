#include "Mesh.h"
#include "../Log.h"
#include "Component.h"
#include <vector>
#include <assimp/scene.h>
#include "../Engine.h"
#include "../Render.h"

Mesh::Mesh(GameObject* owner, bool enabled) : Component(owner, enabled)
{

}

void Mesh::OnDestroy()
{
	Engine::GetInstance().render->DeleteMeshFromGPU(this->meshData);
}
    
bool Mesh::LoadFromAssimpMesh(aiMesh* assimpMesh)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	// 1. Extraer Vértices (CORREGIDO)
	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.position.x = assimpMesh->mVertices[i].x;
		vertex.position.y = assimpMesh->mVertices[i].y; // <-- Añadido
		vertex.position.z = assimpMesh->mVertices[i].z; // <-- Añadido
		vertices.push_back(vertex);
	}

	// 2. Extraer Índices (CORREGIDO)
	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++) {
		aiFace face = assimpMesh->mFaces[i];
		// Asumimos que Assimp ya trianguló (aiProcess_Triangulate)
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	if (vertices.empty() || indices.empty()) {
		LOG("Error: La malla de Assimp no tiene vértices o índices.");
		return false;
	}
	// ...
	return Engine::GetInstance().render->UploadMeshToGPU(this->meshData, vertices, indices);
}



ComponentType Mesh::GetType()
{
    return ComponentType::Mesh;
}