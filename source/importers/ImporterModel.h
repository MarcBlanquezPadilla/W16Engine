#pragma once

#include "../Global.h"
#include "Importer.h"
#include <string>
#include <list>

#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>

class GameObject;


class ImporterModel : public Importer
{
	struct ImportMeshData
	{
		int type;
		std::string name = "";
		std::string path = "";
	};

public:
	bool Import_Internal() override;

	bool SaveMeta() override;

private:
	bool ProcessNode(aiNode* node, const aiScene* scene, GameObject* targetGameObject);
	bool AddMeshAndTexture(aiMesh* assimpMesh, const aiScene* scene, GameObject* target);
	bool LoadMesh(aiMesh* assimpMesh, GameObject* mesh);
	bool LoadTexture(aiMaterial* material, GameObject* obj);

private:
	std::map<UID,ImportMeshData> referedUIDs;
	std::map<std::string, UID> UIDsByName;
};