#include "../Global.h"
#include "../Engine.h"
#include "../ModuleResources.h"

#include "ImporterModel.h"
#include "ImporterMesh.h"
#include "ImporterTexture.h"

#include "../GameObject.h"
#include "../components/Transform.h"
#include "../components/Mesh.h"
#include "../components/Texture.h"

#include "../resources/Resource.h"

#include "../utils/Log.h"
#include "../utils/FileUtils.h"
#include "../utils/Config.h"

#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <sstream>

bool ImporterModel::Import(const std::string assetPath, const std::string libraryPath, const UID uid, const int type)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(assetPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG("Error loading model with Assimp: %s", importer.GetErrorString());
		return false;
	}
	
	referedUIDs.clear();
	GameObject* modelGameObject = new GameObject(true, GetFileName(assetPath));

	if (!modelGameObject || !ProcessNode(scene->mRootNode, scene, assetPath, modelGameObject))
	{
		LOG("Failed to process root node for model: %s", assetPath.c_str());
		return false;
	}

	//SAVE LIBRARY
	Config gameObjectConfig;
	modelGameObject->Save(gameObjectConfig);

	std::string xmlBuffer;
	gameObjectConfig.SaveToString(xmlBuffer);

	std::ofstream file(libraryPath, std::ios::out | std::ios::binary);
	if (file.is_open())
	{
		file.write((const char*)&uid, sizeof(UID));
		file.write((const char*)&type, sizeof(int));

		uint32_t size = (uint32_t)xmlBuffer.size();
		file.write((const char*)&size, sizeof(uint32_t));
		file.write(xmlBuffer.c_str(), size);
		file.close();
		LOG("Model imported to Library: %s", libraryPath.c_str());
	}


	//SAVE META
	referedUIDs.sort();
	referedUIDs.unique();
	SaveMeta(assetPath, uid, type, referedUIDs);

	modelGameObject->CleanUp();
	delete modelGameObject;
	modelGameObject = nullptr;
	return true;
}

bool ImporterModel::ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory, GameObject* targetGameObject)
{
	//APPLY NODE TRANSFORMS
	aiVector3D position;
	aiQuaternion rotation;
	aiVector3D scaling;
	node->mTransformation.Decompose(scaling, rotation, position);

	targetGameObject->transform->SetPosition(glm::vec3(position.x, position.y, position.z));
	targetGameObject->transform->SetQuaternionRotation(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
	targetGameObject->transform->SetScale(glm::vec3(scaling.x, scaling.y, scaling.z));

	//PROCESS MESHES
	if (node->mNumMeshes == 1)
	{
		aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[0]];

		if (!AddMeshAndTexture(assimpMesh, scene, modelDirectory, targetGameObject))
		{
			LOG("Error processing mesh for node %s. Node will be empty.", node->mName.C_Str());
		}
	}
	else if (node->mNumMeshes > 0)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];
			GameObject* meshGameObject = new GameObject(true, assimpMesh->mName.C_Str());

			if (meshGameObject && AddMeshAndTexture(assimpMesh, scene, modelDirectory, meshGameObject))
			{
				targetGameObject->AddChild(meshGameObject);
			}
			else
			{
				LOG("Error processing mesh %s, skipping.", assimpMesh->mName.C_Str());
				delete meshGameObject;
			}
		}
	}

	//RECURSIVE CHILDS CREATION
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		GameObject* childNodeGO = new GameObject(true, node->mChildren[i]->mName.C_Str());
			
		if (ProcessNode(node->mChildren[i], scene, modelDirectory, childNodeGO))
		{
			targetGameObject->AddChild(childNodeGO);
		}
	}

	return true;
}

bool ImporterModel::AddMeshAndTexture(aiMesh* assimpMesh, const aiScene* scene, const std::string& modelDirectory, GameObject* target)
{
	if (target)
	{
		//ADD MESH
		LoadMesh(assimpMesh, target);

		//ADD TEXTURE
		if (scene->HasMaterials())
		{
			aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
			LoadTexture(material, GetDirectoryFromPath(modelDirectory), target);
		}
		return true;
	}
	return false;
}

bool ImporterModel::LoadMesh(aiMesh* assimpMesh, GameObject* target)
{
	Mesh* meshComp = (Mesh*)target->AddComponent(ComponentType::Mesh);
	if (!meshComp) return false;

	UID meshUID = GenerateNewUID();

	ImporterMesh* importer = new ImporterMesh();

	bool success = importer->Import(GetLibraryPath(meshUID), meshUID, Resource::Type::mesh, assimpMesh);

	delete importer;

	if (success)
	{
		//meshComp->SetResource(meshUID);
		referedUIDs.push_back(meshUID);
		return true;
	}
	else
	{
		LOG("Error loading mesh data for %s.", target->name.c_str());
		return false;
	}
}

bool ImporterModel::LoadTexture(aiMaterial* material, const std::string& modelDirectory, GameObject* obj)
{
	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString aiPath;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);

		std::string fileName = GetFileName(aiPath.C_Str());

		//SEARCH FILE
		bool foundFile = false;

		std::string texPath = modelDirectory + aiPath.C_Str();
		if (DoesFileExist(texPath))
		{
			foundFile = true;
		}

		texPath = modelDirectory + fileName;
		if (!foundFile && DoesFileExist(texPath))
		{
			foundFile = true;
		}

		texPath = FindFileInDirectory(modelDirectory, fileName);
		if (!foundFile && DoesFileExist(texPath))
		{
			foundFile = true;
		}


		if (foundFile)
		{
			//LOAD
			Texture* texture = (Texture*)obj->AddComponent(ComponentType::Texture);
			if (texture)
			{
				//BE SURE IMPORTED
				Engine::GetInstance().moduleResources->CheckFileLoaded(texPath);
				Engine::GetInstance().moduleResources->PublishAssetChangedEvent();

				//LOAD TO COMPONENT
				UID textureUID = Engine::GetInstance().moduleResources->Find(texPath);
				if (textureUID != 0)
				{
					texture->SetResource(textureUID);
					referedUIDs.push_back(textureUID);
				}
				else
				{
					LOG("Error: Failed loading and attaching texture %s to component.", aiPath.C_Str());
				}
			}
			else
			{
				LOG("Error: Creation of texture component failed on %s gameObject.", obj->name);
				return false;
			}
		}
		else
		{
			LOG("Error: Could not find texture '%s' in any location", fileName.c_str());
			return false;
		}
	}
	else
	{
		LOG("The material does not have a diffuse texture.");
		return false;
	}
}

bool ImporterModel::SaveMeta(const std::string assetPath, const UID uid, const int type, const std::list<UID> referedIDs)
{
	Config meta;

	SaveBasicMeta(meta, uid, type, referedIDs);

	return meta.Save(GetMetaPath(assetPath).c_str());
}