#pragma once
#include "ModuleLoader.h"
#include "ModuleScene.h"
#include "Engine.h"
#include "ModuleRender.h"
#include "ModuleInput.h"
#include "ModuleResources.h"
#include "GameObject.h"
#include "ModuleEvents.h"

#include "components/Component.h"
#include "components/MeshRenderer.h"
#include "components/Transform.h"
#include "geometry/Vertex.h"
#include "utils/Log.h"
#include "utils/FileUtils.h"
#include "Global.h"

#include "resources/ResourceScene.h"
#include "resources/ResourceModel.h"

#include <list>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>
#include <IL/il.h>
#include <IL/ilu.h>

#include "pugixml.hpp"

ModuleLoader::ModuleLoader(bool startEnabled) : Module(startEnabled)
{
	name = "loader";
}

ModuleLoader::~ModuleLoader()
{

}

bool ModuleLoader::Awake()
{
	return true;
}


bool ModuleLoader::Start()
{
	bool ret = true;

	return ret;
}

bool ModuleLoader::CleanUp()
{
	return true;
}

#pragma region Models

bool ModuleLoader::LoadModel(const std::string& filePath)
{
	UID modelUID = Engine::GetInstance().moduleResources->Find(filePath);

	if (modelUID == 0)
	{
		LOG(LogType::LOG_ERROR,"Model not found at %s", filePath.c_str());
		return false;
	}

	ResourceModel* modelRes = (ResourceModel*)Engine::GetInstance().moduleResources->RequestResource(modelUID);

	if (modelRes && modelRes->IsLoadedToMemory())
	{
		Config gameObjectNode = modelRes->gameObjectConfig.GetChild("GameObject");

		if (!gameObjectNode.IsValid()) {
			LOG(LogType::LOG_ERROR, "Error: Still invalid. XML structure is unexpected.");
			return false;
		}

		GameObject* gameObject = new GameObject(true, gameObjectNode.GetString("Name"));
		if (gameObject)
		{
			gameObject->Load(gameObjectNode);
			Engine::GetInstance().moduleScene->AddGameObject(gameObject);
		}

		LOG(LogType::LOG_INFO, "Model loaded successfully: %s", filePath.c_str());

		//RELEASE RESOURCE
		Engine::GetInstance().moduleResources->ReleaseResource(modelUID);

		return true;
	}
	return false;
}

#pragma endregion

#pragma region Textures

bool ModuleLoader::LoadTextureToGameObjects(const std::string& filePath, std::vector<GameObject*> gameObjects)
{
	bool ret = false;
	for (GameObject* gameObject : gameObjects)
	{
		if (gameObject)
		{
			MeshRenderer* texture = (MeshRenderer*)gameObject->GetComponent(ComponentType::MeshRenderer);

			if (texture == nullptr)
			{
				texture = (MeshRenderer*)gameObject->AddComponent(ComponentType::MeshRenderer);
			}

			texture->SetTextureResource(Engine::GetInstance().moduleResources->Find(filePath));
			LOG(LogType::LOG_INFO,"Texture applied to GameObject %s", gameObject->name.c_str());
			ret = true;
		}
		else
		{
			LOG(LogType::LOG_INFO, "Texture not applied because the object was nullptr.");
			continue;
		}
	}
	if (!ret) LOG(LogType::LOG_INFO, "Texture not applied to any object. Select one object before to apply texture.");
	return ret;
}

bool ModuleLoader::LoadFromAssimpMaterial(aiMaterial* material, const std::string& modelDirectory, GameObject* obj)
{
	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString aiPath;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);

		std::string fileName = GetFileName(aiPath.C_Str());

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
			LoadTextureToGameObjects(texPath, {obj});
		}
		else
		{
			LOG(LogType::LOG_ERROR, "Could not find texture '%s' in any location", fileName.c_str());
			return false;
		}
	}
	else
	{
		LOG(LogType::LOG_INFO, "The material does not have a diffuse texture.");
		return false;
	}
}

bool ModuleLoader::LoadTexture(const std::string& path, unsigned int& textureID, int& width, int& height, bool flip)
{
	unsigned int ilImageID = 0;
	ilGenImages(1, &ilImageID);
	ilBindImage(ilImageID);

	if (ilLoadImage(path.c_str()))
	{
		if (flip) iluFlipImage();

		if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
		{
			LOG(LogType::LOG_ERROR, "Error converting image to RGBA: %s", path.c_str());
			ilDeleteImages(1, &ilImageID);
			return false;
		}

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);

		LOG(LogType::LOG_INFO, "Texture loaded into CPU from: %s (Width: %d, Height: %d)", path.c_str(), width, height);

		if (ilImageID == 0)
		{
			LOG(LogType::LOG_ERROR, "An attempt was made to upload a texture to the GPU without first loading it to the CPU.");
			return false;
		}

		ilBindImage(ilImageID);
		unsigned char* data = ilGetData();

		textureID = Engine::GetInstance().moduleRender->UploadTextureToGPU(
			data,
			width,
			height
		);

		ilBindImage(0);

		if (ilImageID != 0)
		{
			ilDeleteImages(1, &ilImageID);
			ilImageID = 0;
		}
	}
	else
	{
		LOG(LogType::LOG_ERROR, "Failed loading file from %s.", path.c_str());
		ilDeleteImages(1, &ilImageID);
		return false;
	}
	return true;
}
#pragma endregion

#pragma region Basics

void ModuleLoader::LoadBasic(int basic)
{
	if (Engine::GetInstance().moduleResources->RequestResource(basic))
	{
		GameObject* gameObject = new GameObject(true, "Basic");
		MeshRenderer* mesh = (MeshRenderer*)gameObject->AddComponent(ComponentType::MeshRenderer);
		mesh->SetMeshResource(basic);

		if (gameObject)
		{
			Engine::GetInstance().moduleScene->AddGameObject(gameObject);
		}
	}
}

void ModuleLoader::LoadEmpty()
{
	GameObject* gameObject = new GameObject(true, "Empty");

	if (gameObject)
	{
		Engine::GetInstance().moduleScene->AddGameObject(gameObject);
	}
}


#pragma endregion

#pragma region Load&Save

bool ModuleLoader::SaveScene(const std::string& savePath)
{
	//SAVE SCENE
	LOG(LogType::LOG_INFO, "Saving scene in: %s", savePath.c_str());

	if (DoesFileExist(savePath)) 
	{
		LOG(LogType::LOG_ERROR, "Saving scene failed, file already exist");
		return false;
	}

	Config sceneConfig;

	if (!SaveSceneToMemory(sceneConfig))
	{
		LOG(LogType::LOG_ERROR, "Failed parsing scene to XML");
	}

	if (!sceneConfig.Save(savePath.c_str()))
	{
		LOG(LogType::LOG_ERROR, "Error saving the scene file.");
		return false;
	}

	LOG(LogType::LOG_INFO, "Scene saved successfully.");
	return true;
}

bool ModuleLoader::SaveSceneToMemory(Config& sceneConfig)
{
	Config sceneNode = sceneConfig.AddChild("Scene");
	Config gameObjectsList = sceneNode.AddChild("GameObjects");

	//SAVE EACH GAMEOBJECT RECURSIVE
	const std::vector<GameObject*>& gameObjects = Engine::GetInstance().moduleScene->GetRootGameObjects();

	if (gameObjects.size() > 0)
	{
		for (GameObject* gameObject : gameObjects)
		{
			if (gameObject->parent == nullptr)
			{
				Config goNode = gameObjectsList.AddChild("GameObject");

				gameObject->Save(goNode);
			}
		}
	}
	return true;
}

bool ModuleLoader::CleanAndLoadScene(const std::string& assetPath)
{
	Engine::GetInstance().moduleScene->NewScene();
	return LoadScene(assetPath);
}

bool ModuleLoader::LoadScene(const std::string& assetPath)
{
	//GET RESOURCE
	UID sceneUID = Engine::GetInstance().moduleResources->Find(assetPath);

	if (sceneUID == 0)
	{
		LOG(LogType::LOG_ERROR, "Scene not found at %s", assetPath.c_str());
		return false;
	}

	//LOAD RESOURCE
	ResourceScene* sceneRes = (ResourceScene*)Engine::GetInstance().moduleResources->RequestResource(sceneUID);

	if (sceneRes && sceneRes->IsLoadedToMemory())
	{
		LoadSceneFromMemory(sceneRes->sceneConfig);

		//RELEASE RESOURCE
		Engine::GetInstance().moduleResources->ReleaseResource(sceneUID);

		return true;
	}

	return false;
}

bool ModuleLoader::LoadSceneFromMemory(Config& sceneConfig)
{
	Config sceneNode = sceneConfig.GetChild("Scene");

	if (!sceneNode.IsValid()) {
		LOG(LogType::LOG_ERROR, "Failed loading scene: XML structure is unexpected. Scene node invalid");
		return false;
	}

	Config gameObjectsNode = sceneNode.GetChild("GameObjects");
	if (!gameObjectsNode.IsValid())
	{
		LOG(LogType::LOG_ERROR, "Failed loading scene: XML structure is unexpected. GameObjects node invalid");
		return false;
	}

	Config gameObjectNode = gameObjectsNode.GetChild("GameObject");

	while (gameObjectNode.IsValid())
	{
		GameObject* gameObject = new GameObject(true, gameObjectNode.GetString("Name"));
		if (gameObject)
		{
			gameObject->Load(gameObjectNode);
			Engine::GetInstance().moduleScene->AddGameObject(gameObject);
		}
		gameObjectNode = gameObjectNode.GetNextSibling("GameObject");
	}

	for (auto pair : Engine::GetInstance().moduleScene->GetAllGameObjects())
	{
		pair.second->SolveReferences();
	}

	return true;
}

#pragma endregion
