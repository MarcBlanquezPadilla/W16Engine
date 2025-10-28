#include<list>
#include "Scene.h"
#include "GameObject.h"
#include "components/Component.h"
#include "components/Mesh.h"
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
	// 1. Borramos los modelos viejos (¡esto es temporal!)
	for (auto go : gameObjects) delete go;
	gameObjects.clear();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG("Error cargando modelo con Assimp: %s", importer.GetErrorString());
		return false;
	}

	// 2. Recorremos las mallas de Assimp
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* assimpMesh = scene->mMeshes[i];

		// 3. Creamos un GameObject por cada malla
		GameObject* newGO = new GameObject(true);

		// 4. Le añadimos el Componente Mesh
		Mesh* meshComp = (Mesh*)newGO->AddComponent(ComponentType::Mesh);

		// 5. Le decimos al componente que se cargue
		if (meshComp->LoadFromAssimpMesh(assimpMesh))
		{
			// 6. Si todo va bien, lo añadimos a la escena
			gameObjects.push_back(newGO);
		}
		else
		{
			// Si falla, borramos el GO
			delete newGO;
		}
	}
	return true;
}