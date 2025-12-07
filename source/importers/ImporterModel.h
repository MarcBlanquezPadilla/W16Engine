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
public:
	bool Import(const std::string assetPath, const std::string libraryPath, const UID uid, const int type) override;

	bool SaveMeta(const std::string assetPath, const UID uid, const int type, const std::list<UID> referedIDs) override;

private:
	bool ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory, GameObject* targetGameObject);
	bool AddMeshAndTexture(aiMesh* assimpMesh, const aiScene* scene, const std::string& modelDirectory, GameObject* target);
	bool LoadMesh(aiMesh* assimpMesh, GameObject* mesh);
	bool LoadTexture(aiMaterial* material, const std::string& modelDirectory, GameObject* obj);

private:
	std::list<UID> referedUIDs;
};