#include "Scene.h"
#include "GameObject.h"
#include "components/Component.h"
#include "components/Mesh.h"
#include "components/Transform.h"
#include "components/Texture.h"
#include "Log.h"

#include<list>
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
		GameObject* newGO = new GameObject(true);

		Mesh* meshComp = (Mesh*)newGO->AddComponent(ComponentType::Mesh);
		if (!meshComp || !meshComp->LoadFromAssimpMesh(assimpMesh))
		{
			LOG("Error al cargar la malla, se aborta la creación de este GameObject.");
			delete newGO;
			continue;
		}

		if (scene->HasMaterials())
		{
			aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
			Texture* texComp = (Texture*)newGO->AddComponent(ComponentType::Texture);
			if (texComp != nullptr)
			{
				texComp->LoadFromAssimpMaterial(material, modelDirectory);
			}
		}

		gameObjects.push_back(newGO);
	}
	return true;
}

