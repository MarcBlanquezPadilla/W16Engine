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

bool ImporterModel::Import_Internal()
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(assetPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG("Error loading model with Assimp: %s", importer.GetErrorString());
		return false;
	}
	
	referedUIDs.clear();
	UIDsByName.clear();
	if (DoesFileHasMeta(assetPath))
	{
		Config modelConfig;
		if (modelConfig.Load(GetMetaPath(assetPath).c_str()))
		{					
			if (modelConfig.IsValid() && modelConfig.GetUInt("ReferedObjects") > 0)
			{
				Config childNode = modelConfig.GetChild("ReferedObject");
				while (childNode.IsValid())
				{
					std::string name = childNode.GetString("Name");
					UID uid = childNode.GetUInt("UID");

					if (name != "" && uid != 0)
					{
						UIDsByName.emplace(name, uid);
					}
					childNode = childNode.GetNextSibling("ReferedObject");
				}
			}
		}
	}

	GameObject* modelGameObject = new GameObject(true, GetFileName(assetPath));

	if (!modelGameObject || !ProcessNode(scene->mRootNode, scene, modelGameObject))
	{
		LOG("Failed to process root node for model: %s", assetPath.c_str());
		return false;
	}

	//SAVE LIBRARY
	Config gameObjectConfig;
	Config gameObjectNode = gameObjectConfig.AddChild("GameObject");
	modelGameObject->Save(gameObjectNode);

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
		LOG("Model %s imported to Library: %s", GetFileName(assetPath).c_str(), libraryPath.c_str());
	}


	//SAVE META
	SaveMeta();

	modelGameObject->CleanUp();
	delete modelGameObject;
	modelGameObject = nullptr;
	return true;
}

bool ImporterModel::ProcessNode(aiNode* node, const aiScene* scene, GameObject* targetGameObject)
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

		if (!AddMeshAndTexture(assimpMesh, scene, targetGameObject))
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

			if (meshGameObject && AddMeshAndTexture(assimpMesh, scene, meshGameObject))
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
			
		if (ProcessNode(node->mChildren[i], scene, childNodeGO))
		{
			targetGameObject->AddChild(childNodeGO);
		}
	}

	return true;
}

bool ImporterModel::AddMeshAndTexture(aiMesh* assimpMesh, const aiScene* scene, GameObject* target)
{
	if (target)
	{
		//ADD MESH
		LoadMesh(assimpMesh, target);

		//ADD TEXTURE
		if (scene->HasMaterials())
		{
			aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
			LoadTexture(material, target);
		}
		return true;
	}
	return false;
}

bool ImporterModel::LoadMesh(aiMesh* assimpMesh, GameObject* target)
{
	Mesh* meshComp = (Mesh*)target->AddComponent(ComponentType::Mesh);
	if (!meshComp) return false;

	//GET UID
	UID meshUID = 0;
	if (UIDsByName.find(target->name) != UIDsByName.end())
	{
		meshUID = UIDsByName[target->name];
	}
	else meshUID = GenerateNewUID();

	ImporterMesh* importer = new ImporterMesh();

	bool success = importer->Import(GetLibraryPath(meshUID), meshUID, Resource::Type::mesh, assimpMesh);

	delete importer;

	if (success)
	{
		meshComp->SetResource(meshUID);
		ImportMeshData importMeshData;
		importMeshData.name = target->name;
		importMeshData.path = assetPath;
		importMeshData.type = Resource::Type::mesh;
		referedUIDs.emplace(meshUID,importMeshData);
		return true;
	}
	else
	{
		LOG("Error loading mesh data for %s.", target->name.c_str());
		return false;
	}
}

bool ImporterModel::LoadTexture(aiMaterial* material, GameObject* obj)
{
	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString aiPath;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);

		std::string fileName = GetFileName(aiPath.C_Str());

		//SEARCH FILE
		bool foundFile = false;
		std::string modelDirectory = GetDirectoryFromPath(assetPath);

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
					ImportMeshData importMeshData;
					importMeshData.name = fileName;
					importMeshData.type = Resource::Type::texture;
					importMeshData.path = texPath;
					referedUIDs.emplace(textureUID, importMeshData);
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

bool ImporterModel::SaveMeta()
{
	Config meta;

	SaveBasicMeta(meta);

	meta.SetUInt("ReferedObjects", referedUIDs.size());
	for (auto pair : referedUIDs)
	{
		Config refered = meta.AddChild("ReferedObject");
		refered.SetUInt("UID", pair.first);
		refered.SetString("Name", pair.second.name.c_str());
		refered.SetString("Path", pair.second.path.c_str());
		refered.SetInt("Type", pair.second.type);
	}

	return meta.Save(GetMetaPath(assetPath).c_str());
}