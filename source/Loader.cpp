#pragma once
#include "Loader.h"
#include "Scene.h"
#include "Engine.h"
#include "Input.h"
#include "GameObject.h"

#include "components/Component.h"
#include "components/Mesh.h"
#include "components/Transform.h"
#include "components/Texture.h"
#include "Log.h"
#include "Global.h"

#include <list>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

std::string GetDirectoryFromPath(const std::string& filePath)
{
	size_t pos = filePath.find_last_of("\\/");
	return (std::string::npos == pos) ? "" : filePath.substr(0, pos + 1);
}

std::string GetFileExtension(const std::string& filePath)
{
	size_t pos = filePath.find_last_of(".");
	if (std::string::npos == pos) return "";

	std::string ext = filePath.substr(pos + 1);

	std::transform(ext.begin(), ext.end(), ext.begin(),
		[](unsigned char c) { return std::tolower(c); });

	return ext;
}

Loader::Loader(bool startEnabled) : Module(startEnabled)
{
	name = "loader";
}

Loader::~Loader()
{

}

bool Loader::Awake()
{
	return true;
}


bool Loader::Start()
{
	bool ret = true;

	std::string modelPath = "Assets/BakerHouse.fbx";

	LOG("Loading initial model: %s", modelPath.c_str());

	if (!LoadModel(modelPath))
	{
		LOG("ERROR: Failed to load the initial model. Check if the file exists in the build directory.");
		ret = false;
	}

	return ret;
}

bool Loader::CleanUp()
{
	return true;
}


void Loader::HandleAssetDrop(const std::string& path)
{
	std::string extension = GetFileExtension(path);

	if (extension == "fbx" || extension == "obj")
	{
		LoadModel(path);
	}
	else if (extension == "png" || extension == "dds" || extension == "jpg" || extension == "tga")
	{
		LoadTexture(path);
	}
	else
	{
		LOG("Error loading file, incompatible format: %s", extension.c_str());
	}
}

bool Loader::LoadModel(const std::string& filePath)
{
	std::string modelDirectory = GetDirectoryFromPath(filePath);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG("Error loading model with Assimp: %s", importer.GetErrorString());
		return false;
	}

	GameObject* gameObject = new GameObject(true, scene->mRootNode->mName.C_Str());
	if (scene->mNumMeshes > 1)
	{
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* assimpMesh = scene->mMeshes[i];
			GameObject* child = new GameObject(true, assimpMesh->mName.C_Str());
			Mesh* meshComp = (Mesh*)child->AddComponent(ComponentType::Mesh);
			if (!meshComp || !meshComp->LoadFromAssimpMesh(assimpMesh))
			{
				LOG("Error loading mesh, creation of this GameObject is aborted.");
				delete gameObject;
				continue;
			}

			if (scene->HasMaterials())
			{
				aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
				Texture* texComp = (Texture*)child->AddComponent(ComponentType::Texture);
				if (texComp != nullptr)
				{
					texComp->LoadFromAssimpMaterial(material, modelDirectory);
				}
			}

			gameObject->AddChild(child);
			Engine::GetInstance().scene->SetSelectedGameObject(child);
		}
	}
	else
	{
		aiMesh* assimpMesh = scene->mMeshes[0];
		gameObject->name = assimpMesh->mName.C_Str();
		Mesh* meshComp = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);
		if (!meshComp || !meshComp->LoadFromAssimpMesh(assimpMesh))
		{
			LOG("Error loading mesh, creation of this GameObject is aborted.");
			delete gameObject;
		}

		if (scene->HasMaterials())
		{
			aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
			Texture* texComp = (Texture*)gameObject->AddComponent(ComponentType::Texture);
			if (texComp != nullptr)
			{
				texComp->LoadFromAssimpMaterial(material, modelDirectory);
			}
		}
	}
	Engine::GetInstance().scene->AddGameObject(gameObject);

	return true;
}

bool Loader::LoadTexture(const std::string& filePath)
{
	GameObject* selectedGameObject = Engine::GetInstance().scene->GetSelectedGameObject();
	if (selectedGameObject)
	{
		Texture* texture = (Texture*)selectedGameObject->GetComponent(ComponentType::Texture);

		if (texture == nullptr)
		{
			texture = (Texture*)selectedGameObject->AddComponent(ComponentType::Texture);
		}

		if (texture->LoadTexture(filePath))
		{
			LOG("Texture %s applied to GameObject: %s", filePath.c_str(), selectedGameObject->name.c_str());
			return true;
		}
	}
	else
	{
		LOG("No object selected.");
		return false;
	}

}

void Loader::CreateBasic(int basic)
{
	GameObject* gameObject = nullptr;
	Mesh* mesh = nullptr;

	switch (basic)
	{
	case CUBE:
		gameObject = new GameObject(true, "Cube");
		mesh = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);
		mesh->LoadCube();
		break;
	case SPHERE:
		gameObject = new GameObject(true, "Sphere");
		mesh = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);
		mesh->LoadSphere();
		break;
	case PYRAMID:
		gameObject = new GameObject(true, "Pyramid");
		mesh = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);
		mesh->LoadPyramid();
		break;
	}

	if (gameObject)
	{
		Engine::GetInstance().scene->AddGameObject(gameObject);
	}
}