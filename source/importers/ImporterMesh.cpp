#include "../Global.h"
#include "../utils/Log.h"

#include "ImporterMesh.h"

#include "../resources/Resource.h"

#include "../geometry/Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <fstream>

bool ImporterMesh::Import(const std::string libraryPath, const UID uid, const int type, aiMesh* assimpMesh)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	//FILL VERTEXS
	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.position.x = assimpMesh->mVertices[i].x;
		vertex.position.y = assimpMesh->mVertices[i].y;
		vertex.position.z = assimpMesh->mVertices[i].z;

		if (assimpMesh->HasNormals()) {
			vertex.normal.x = assimpMesh->mNormals[i].x;
			vertex.normal.y = assimpMesh->mNormals[i].y;
			vertex.normal.z = assimpMesh->mNormals[i].z;
		}
		else {
			vertex.normal = glm::vec3(0.0f);
		}

		if (assimpMesh->HasTextureCoords(0)) {
			vertex.texCoords.x = assimpMesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = assimpMesh->mTextureCoords[0][i].y;
		}
		else {
			vertex.texCoords = glm::vec2(0.0f);
		}

		vertices.push_back(vertex);
	}

	//FILL INDEX
	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++)
	{
		aiFace face = assimpMesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	std::ofstream file(libraryPath, std::ios::out | std::ios::binary);
	if (file.is_open())
	{
		file.write((const char*)&uid, sizeof(UID));
		file.write((const char*)&type, sizeof(int));

		uint32_t num_vertices = vertices.size();
		uint32_t num_indices = indices.size();

		file.write(reinterpret_cast<const char*>(&num_vertices), sizeof(uint32_t));

		file.write(reinterpret_cast<const char*>(&num_indices), sizeof(uint32_t));

		file.write(reinterpret_cast<const char*>(vertices.data()), num_vertices * sizeof(Vertex));

		file.write(reinterpret_cast<const char*>(indices.data()), num_indices * sizeof(unsigned int));

		file.close();
		LOG("Mesh %s saved to Library: %s ", assimpMesh->mName, libraryPath.c_str(), libraryPath);
	}

	return true;
}