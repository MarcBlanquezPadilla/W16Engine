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
#include "components/Mesh.h"
#include "components/Transform.h"
#include "components/Texture.h"
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
	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::FileDropped, this);
	return true;
}


bool ModuleLoader::Start()
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

bool ModuleLoader::CleanUp()
{
	Engine::GetInstance().moduleEvents->UnsubscribeAll(this);
	return true;
}


void ModuleLoader::HandleAssetDrop(const std::string& path)
{
	std::string extension = GetFileExtension(path);

	if (extension == "fbx" || extension == "obj")
	{
		LoadModel(path);
	}
	else if (extension == "png" || extension == "dds" || extension == "jpg" || extension == "tga")
	{
		LoadTextureToGameObject(path, nullptr);
	}
	else
	{
		LOG("Error loading file, incompatible format: %s", extension.c_str());
	}
}

#pragma region Models

bool ModuleLoader::LoadModel(const std::string& filePath)
{
	UID modelUID = Engine::GetInstance().moduleResources->Find(filePath);

	if (modelUID == 0)
	{
		LOG("Error: Scene not found at %s", filePath.c_str());
		return false;
	}

	ResourceModel* modelRes = (ResourceModel*)Engine::GetInstance().moduleResources->RequestResource(modelUID);

	if (modelRes && modelRes->IsLoadedToMemory())
	{
		Config gameObjectNode = modelRes->gameObjectConfig.GetChild("GameObject");

		if (!gameObjectNode.IsValid()) {
			LOG("Error: Still invalid. XML structure is unexpected.");
			return false;
		}

		GameObject* gameObject = new GameObject(true, gameObjectNode.GetString("Name"));
		if (gameObject)
		{
			gameObject->Load(gameObjectNode);
			Engine::GetInstance().moduleScene->AddGameObject(gameObject);
		}

		LOG("Model loaded successfully: %s", filePath.c_str());

		//RELEASE RESOURCE
		Engine::GetInstance().moduleResources->ReleaseResource(modelUID);


		return true;
	}
	return false;
}

#pragma endregion

#pragma region Textures

bool ModuleLoader::LoadTextureToGameObject(const std::string& filePath, GameObject* gameObject)
{
	if (gameObject)
	{
		Texture* texture = (Texture*)gameObject->GetComponent(ComponentType::Texture);

		if (texture == nullptr)
		{
			texture = (Texture*)gameObject->AddComponent(ComponentType::Texture);
		}

		texture->SetResource(Engine::GetInstance().moduleResources->Find(filePath));
	}
	else
	{
		LOG("Object is nullptr");
		return false;
	}

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
			LoadTextureToGameObject(texPath, obj);
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
			LOG("Error converting image to RGBA: %s", path.c_str());
			ilDeleteImages(1, &ilImageID);
			return false;
		}

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);

		LOG("Texture loaded into CPU from: %s (Width: %d, Height: %d)", path.c_str(), width, height);

		if (ilImageID == 0)
		{
			LOG("Error: An attempt was made to upload a texture to the GPU without first loading it to the CPU.");
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
		LOG("Error: Failed loading file from %s.", path.c_str());
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
		Mesh* mesh = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);
		mesh->SetResource(basic);

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
	LOG("Saving scene in: %s", savePath.c_str());

	Config sceneFile;

	Config sceneNode = sceneFile.AddChild("Scene");
	Config gameObjectsList = sceneNode.AddChild("GameObjects");

	//SAVE EACH GAMEOBJECT RECURSIVE
	const std::vector<GameObject*>& gameObjects = Engine::GetInstance().moduleScene->GetGameObjects();

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

	if (!sceneFile.Save(savePath.c_str()))
	{
		LOG("Error saving the scene file.");
		return false;
	}

	LOG("Scene saved successfully.");
	return true;
}

bool ModuleLoader::LoadScene(const std::string& assetPath)
{
	//GET RESOURCE
	UID sceneUID = Engine::GetInstance().moduleResources->Find(assetPath);

	if (sceneUID == 0)
	{
		LOG("Error: Scene not found at %s", assetPath.c_str());
		return false;
	}

	ResourceScene* sceneRes = (ResourceScene*)Engine::GetInstance().moduleResources->RequestResource(sceneUID);

	if (sceneRes && sceneRes->IsLoadedToMemory())
	{

		//SCENE CLEANUP
		Engine::GetInstance().moduleScene->NewScene();

		Config sceneNode = sceneRes->sceneConfig.GetChild("Scene");

		if (!sceneNode.IsValid()) {
			LOG("Error: Still invalid. XML structure is unexpected.");
			return false;
		}

		Config gameObjectsNode = sceneNode.GetChild("GameObjects");
		if (!gameObjectsNode.IsValid())
		{
			LOG("Error loading scene: gameObjects node invalid.");
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
		
		LOG("Scene loaded successfully: %s", assetPath.c_str());

		//RELEASE RESOURCE
		Engine::GetInstance().moduleResources->ReleaseResource(sceneUID);

		return true;
	}

	return false;
}

#pragma endregion

void ModuleLoader::OnEvent(const Event& event)
{
	switch (event.type)
	{
	case Event::Type::FileDropped:
	{
		{
			HandleAssetDrop(event.data.string.filePath);
		}
		break;
	}

	default:
		break;
	}
}