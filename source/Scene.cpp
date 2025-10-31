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

	// Convertir a minúsculas
	std::transform(ext.begin(), ext.end(), ext.begin(),
		[](unsigned char c) { return std::tolower(c); });

	return ext;
}

Scene::Scene(bool startEnabled) : Module(startEnabled)
{

}

Scene::~Scene()
{

}

bool Scene::Awake()
{
	bool ret = true;
	selectedGameObject = nullptr;
	return ret;
}

bool Scene::PreUpdate()
{
	bool ret = true;

	return ret;
}

bool Scene::Update(float dt)
{
	bool ret = true;

	for each(GameObject* gameObject in gameObjects)
	{
		gameObject->Update(dt);
	}

	return ret;
}

bool Scene::PostUpdate()
{
	bool ret = true;

	return ret;
}

bool Scene::CleanUp()
{
	bool ret = true;

	return ret;
}

void Scene::HandleAssetDrop(const std::string& path)
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
		LOG("Error cargando archivo, formato incompatible: %s", extension.c_str());
	}
}

bool Scene::LoadModel(const std::string& filePath)
{
	std::string modelDirectory = GetDirectoryFromPath(filePath);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG("Error cargando modelo con Assimp: %s", importer.GetErrorString());
		return false;
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* assimpMesh = scene->mMeshes[i];
		GameObject* gameObject = new GameObject(true, assimpMesh->mName.C_Str());
		Mesh* meshComp = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);
		if (!meshComp || !meshComp->LoadFromAssimpMesh(assimpMesh))
		{
			LOG("Error al cargar la malla, se aborta la creación de este GameObject.");
			delete gameObject;
			continue;
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

		gameObjects.push_back(gameObject);
	}
	return true;
}

bool Scene::LoadTexture(const std::string& filePath)
{
	if (selectedGameObject)
	{
		Texture* texture = (Texture*)selectedGameObject->GetComponent(ComponentType::Texture);

		if (texture == nullptr)
		{
			texture = (Texture*)selectedGameObject->AddComponent(ComponentType::Texture);
		}

		if (texture->LoadTexture(filePath))
		{
			LOG("Textura %s aplicada a GameObject: %s", filePath.c_str(), selectedGameObject->name.c_str());
			return true;
		}
	}
	else
	{
		LOG("Ningun objeto seleccionado.");
		return false;
	}

}

void Scene::CreateBasic(int basic)
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
	if (gameObject) gameObjects.push_back(gameObject);
}
