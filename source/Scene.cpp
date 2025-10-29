#include<list>
#include "Scene.h"
#include "GameObject.h"
#include "components/Component.h"
#include "components/Mesh.h"
#include "components/Transform.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Log.h"


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

bool Scene::LoadModel(const std::string& filePath)
{
	for (auto go : gameObjects) delete go;
	gameObjects.clear();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
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

		if (meshComp->LoadFromAssimpMesh(assimpMesh))
		{
			gameObjects.push_back(newGO);
		}
		else
		{
			delete newGO;
		}
	}
	return true;
}