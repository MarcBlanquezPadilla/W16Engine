#pragma once

#include "../Global.h"
#include "Importer.h"
#include <string>
#include <list>

#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>

class GameObject;
class MeshRenderer;

class ImporterModel : public Importer
{
	struct ReferedsData
	{
		int type;
		std::string name = "";
	};

public:

	~ImporterModel() override;
	
	bool Import_Internal() override;

	bool SaveMeta() override;

private:
	bool ProcessNode(aiNode* node, const aiScene* scene, GameObject* targetGameObject);
	bool AddMeshAndTexture(aiMesh* assimpMesh, const aiScene* scene, GameObject* target);
	MeshRenderer* LoadMesh(aiMesh* assimpMesh, GameObject* meshObject);
	bool LoadTexture(aiMaterial* material, const aiScene* scene, MeshRenderer* mesh);

private:
	std::map<UID, ReferedsData> referedUIDs;
	std::map<std::string, UID> UIDsByName;
};