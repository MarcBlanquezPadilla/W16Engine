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

Mesh::~Mesh()
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

	if (assimpMesh->HasTextureCoords(0))
	{
		this->hasUVs = true;
		LOG("Cargando malla CON coordenadas UV.");
	}
	else
	{
		this->hasUVs = false;
		LOG("Cargando malla SIN coordenadas UV. Se usarán UVs falsas.");
	}

	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.position.x = assimpMesh->mVertices[i].x;
		vertex.position.y = assimpMesh->mVertices[i].y;
		vertex.position.z = assimpMesh->mVertices[i].z;

		if (this->hasUVs)
		{
			vertex.texCoords.x = assimpMesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = assimpMesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertex.texCoords = glm::vec2(0.0f, 0.0f);
		}
		vertices.push_back(vertex);
	}


	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++) {
		aiFace face = assimpMesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	if (vertices.empty() || indices.empty()) {
		LOG("Error: La malla de Assimp no tiene vértices o índices.");
		return false;
	}

	return Engine::GetInstance().render->UploadMeshToGPU(this->meshData, vertices, indices);
}