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
#include "utils/Log.h"
#include "Global.h"

#include <list>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "SDL3/SDL_filesystem.h"

#include "pugixml.hpp"

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

std::string GetFileName(const std::string& filePath)
{
	size_t pos = filePath.find_last_of("/\\");
	if (std::string::npos == pos) return filePath;
	return filePath.substr(pos + 1);
}

std::string FindFileInDirectory(const std::string& directoryPath, const std::string& fileName)
{
	std::string resultPath = "";

	struct CallbackData {
		std::string* result;
		const std::string* target;
	} data{ &resultPath, &fileName };

	SDL_EnumerateDirectory(
		directoryPath.c_str(),
		[](void* userdata, const char* dirname, const char* fname) -> SDL_EnumerationResult {
			auto* d = static_cast<CallbackData*>(userdata);
			if (fname && *d->target == fname) {
				*d->result = std::string(dirname) + "/" + fname;
				return SDL_ENUM_SUCCESS;
			}
			return SDL_ENUM_CONTINUE;
		},
		&data
	);

	return resultPath;
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

	//std::string modelPath = "Assets/BakerHouse.fbx";

	//LOG("Loading initial model: %s", modelPath.c_str());

	//if (!LoadModel(modelPath))
	//{
	//	LOG("ERROR: Failed to load the initial model. Check if the file exists in the build directory.");
	//	ret = false;
	//}

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

#pragma region Models

bool Loader::LoadModel(const std::string& filePath)
{
	std::string modelDirectory = GetDirectoryFromPath(filePath);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath,
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG("Error loading model with Assimp: %s", importer.GetErrorString());
		return false;
	}

	//SCENE NODES PROCESS
	GameObject* rootGameObject = ProcessNode(scene->mRootNode, scene, modelDirectory);

	if (rootGameObject == nullptr)
	{
		LOG("Failed to process root node for model: %s", filePath.c_str());
		return false;
	}

	//ADD GAMEOBJECT TO SCENE
	Engine::GetInstance().scene->AddGameObject(rootGameObject);

	return true;
}

bool Loader::LoadFromAssimpMesh(aiMesh* assimpMesh, Mesh* mesh)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	//FILL VERTEXS
	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.position.x = assimpMesh->mVertices[i].x;
		vertex.position.y = assimpMesh->mVertices[i].y;
		vertex.position.z = assimpMesh->mVertices[i].z;

		if (assimpMesh->HasNormals()) {
			vertex.normal.x = assimpMesh->mNormals[i].x;
			vertex.normal.y = assimpMesh->mNormals[i].y;
			vertex.normal.z = assimpMesh->mNormals[i].z;
		}
		else {
			vertex.normal = glm::vec3(0.0f);
		}

		if (assimpMesh->HasTextureCoords(0)) {
			vertex.texCoords.x = assimpMesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = assimpMesh->mTextureCoords[0][i].y;
			mesh->hasUVs = true;
		}
		else {
			vertex.texCoords = glm::vec2(0.0f);
			mesh->hasUVs = false;
		}

		vertices.push_back(vertex);
	}

	//FILL INDEX
	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++)
	{
		aiFace face = assimpMesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	mesh->LoadModel(vertices, indices);

	return true;
}

GameObject* Loader::ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory)
{
	GameObject* nodeGameObject = new GameObject(true, node->mName.C_Str());

	//APPLY NODE TRANSFORMS
	aiVector3D position;
	aiQuaternion rotation;
	aiVector3D scaling;
	node->mTransformation.Decompose(scaling, rotation, position);

	nodeGameObject->transform->SetPosition(glm::vec3(position.x, position.y, position.z));
	nodeGameObject->transform->SetQuaternionRotation(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
	nodeGameObject->transform->SetScale(glm::vec3(scaling.x, scaling.y, scaling.z));

	//PROCESS MESHES
	if (node->mNumMeshes == 1)
	{
		aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[0]];

		if (!ProcessMeshComponents(nodeGameObject, assimpMesh, scene, modelDirectory))
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

			if (ProcessMeshComponents(meshGameObject, assimpMesh, scene, modelDirectory))
			{
				nodeGameObject->AddChild(meshGameObject);
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
		GameObject* childNodeGO = ProcessNode(node->mChildren[i], scene, modelDirectory);
		if (childNodeGO)
		{
			nodeGameObject->AddChild(childNodeGO);
		}
	}

	return nodeGameObject;
}

bool Loader::ProcessMeshComponents(GameObject* target, aiMesh* assimpMesh, const aiScene* scene, const std::string& modelDirectory)
{
	//ADD MESH
	Mesh* meshComp = (Mesh*)target->AddComponent(ComponentType::Mesh);
	if (!meshComp || !LoadFromAssimpMesh(assimpMesh, meshComp))
	{
		LOG("Error loading mesh data for %s.", target->name.c_str());
		return false;
	}

	//ADD TEXTURE
	if (scene->HasMaterials())
	{
		aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
		Texture* texComp = (Texture*)target->AddComponent(ComponentType::Texture);
		if (texComp != nullptr)
		{
			LoadFromAssimpMaterial(material, modelDirectory, texComp);
		}
	}

	return true; // Éxito
}


#pragma endregion

#pragma region Textures

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

bool Loader::LoadFromAssimpMaterial(aiMaterial* material, const std::string& modelDirectory, Texture* texture)
{
	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString aiPath;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);

		std::string fileName = GetFileName(aiPath.C_Str());

		// Intento 1: Ruta tal como viene en el material
		std::string texPath = modelDirectory + aiPath.C_Str();
		if (texture->LoadTexture(texPath))
		{
			return true;
		}

		// Intento 2: Solo el nombre del archivo en el directorio del modelo
		texPath = modelDirectory + fileName;
		if (texture->LoadTexture(texPath))
		{
			return true;
		}

		// Intento 3: Buscar el archivo en el directorio del modelo
		texPath = FindFileInDirectory(modelDirectory, fileName);
		if (!texPath.empty() && texture->LoadTexture(texPath))
		{
			return true;
		}

		LOG("Error: Could not find texture '%s' in any location", fileName.c_str());
		return false;
	}
	else
	{
		LOG("The material does not have a diffuse texture.");
		return false;
	}
}

#pragma endregion

#pragma region Basics

void Loader::CreateBasic(int basic)
{
	switch (basic)
	{
	case CUBE:
		CreateCube();
		break;
	case SPHERE:
		CreateSphere();
		break;
	case PYRAMID:
		CreatePyramid();
		break;
	}
}

void Loader::CreateCube()
{
	GameObject* gameObject = new GameObject(true, "Cube");
	Mesh* mesh = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	//CUBE CONSTRUCTION
	vertices = {

		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)},

		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

		{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

		{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}
	};

	indices = {
		0, 1, 2,  2, 3, 0,
		4, 5, 6,  6, 7, 4,
		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};

	mesh->LoadModel(vertices, indices);

	if (gameObject)
	{
		Engine::GetInstance().scene->AddGameObject(gameObject);
	}
}

void Loader::CreateSphere()
{
	GameObject* gameObject = new GameObject(true, "Cube");
	Mesh* mesh = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	//SPHERE CONSTRUCTION
	const int sectors = 36;
	const int stacks = 18;
	const float radius = 0.5f;

	for (int i = 0; i <= stacks; ++i) {
		float V = (float)i / (float)stacks;
		float phi = V * glm::pi<float>();
		for (int j = 0; j <= sectors; ++j) {
			float U = (float)j / (float)sectors;
			float theta = U * (glm::pi<float>() * 2);
			float x = cos(theta) * sin(phi);
			float y = cos(phi);
			float z = sin(theta) * sin(phi);
			vertices.push_back({
				{x * radius, y * radius, z * radius},
				{x, y, z},
				{U, V}
				});
		}
	}
	for (int i = 0; i < stacks; ++i) {
		for (int j = 0; j < sectors; ++j) {
			int first = (i * (sectors + 1)) + j;
			int second = first + sectors + 1;
			indices.push_back(first);
			indices.push_back(first + 1);
			indices.push_back(second);
			indices.push_back(first + 1);
			indices.push_back(second + 1);
			indices.push_back(second);
		}
	}

	mesh->LoadModel(vertices, indices);

	if (gameObject)
	{
		Engine::GetInstance().scene->AddGameObject(gameObject);
	}
}

void Loader::CreatePyramid()
{
	GameObject* gameObject = new GameObject(true, "Cube");
	Mesh* mesh = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	glm::vec3 apex = glm::vec3(0.0f, 0.5f, 0.0f);

	//PYRAMID CONSTRUCTION
	vertices = {
		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f)), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f)), glm::vec2(1.0f, 0.0f)},
		{apex, glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f)), glm::vec2(0.5f, 1.0f)},

		{glm::vec3(0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(0.5f, 0.5f, 0.0f)), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(0.5f, 0.5f, 0.0f)), glm::vec2(1.0f, 0.0f)},
		{apex, glm::normalize(glm::vec3(0.5f, 0.5f, 0.0f)), glm::vec2(0.5f, 1.0f)},

		{glm::vec3(0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, -0.5f)), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, -0.5f)), glm::vec2(1.0f, 0.0f)},
		{apex, glm::normalize(glm::vec3(0.0f, 0.5f, -0.5f)), glm::vec2(0.5f, 1.0f)},

		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(-0.5f, 0.5f, 0.0f)), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(-0.5f, 0.5f, 0.0f)), glm::vec2(1.0f, 0.0f)},
		{apex, glm::normalize(glm::vec3(-0.5f, 0.5f, 0.0f)), glm::vec2(0.5f, 1.0f)},
	};

	indices = {
		0, 1, 3,  1, 2, 3,
		4, 5, 6,
		7, 8, 9,
		10, 11, 12,
		13, 14, 15
	};

	mesh->LoadModel(vertices, indices);

	if (gameObject)
	{
		Engine::GetInstance().scene->AddGameObject(gameObject);
	}
}

#pragma endregion

#pragma region Load&Save

bool Loader::SaveScene()
{
	std::string savePath = "Assets/Scenes/scene.W16Scene";
	LOG("Saving scene in: %s", savePath.c_str());

	pugi::xml_document doc;

	pugi::xml_node sceneNode = doc.append_child("Scene");

	pugi::xml_node gameObjectsNode = sceneNode.append_child("GameObjects");

	for (GameObject* gameObject : Engine::GetInstance().scene->GetGameObjects())
	{
		pugi::xml_node currentGameObjectNode = gameObjectsNode.append_child("GameObject");
		gameObject->Save(currentGameObjectNode);
	}

	if (!doc.save_file(savePath.c_str()))
	{
		LOG("Error saving the scene file.");
		return false;
	}
	return true;
}

bool Loader::LoadScene()
{
	std::string loadPath = "Assets/Scenes/scene.W16Scene";
	LOG("Loading scene with path: %s", loadPath.c_str());

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(loadPath.c_str());

	if (!result)
	{
		LOG("Error loading scene file '%s': %s", loadPath.c_str(), result.description());
		return false;
	}

	pugi::xml_node sceneNode = doc.child("Scene");
	if (!sceneNode)
	{
		LOG("Error: <Scene> node not found in %s", loadPath.c_str());
		return false;
	}

	pugi::xml_node gameObjectsNode = sceneNode.child("GameObjects");
	if (!gameObjectsNode)
	{
		LOG("Error: <GameObjects> node not found in %s", loadPath.c_str());
		return false;
	}

	//Engine::GetInstance().scene->ClearGameObjects(); // O algo similar

	for (pugi::xml_node gameObjectNode = gameObjectsNode.child("GameObject"); gameObjectNode; gameObjectNode = gameObjectNode.next_sibling("GameObject"))
	{
		GameObject* gameObject = new GameObject(true, gameObjectNode.attribute("Name").as_string());

		if (!gameObject)
		{
			LOG("Error: Could not create new GameObject while loading scene.");
			continue;
		}
		gameObject->Load(gameObjectNode);
		Engine::GetInstance().scene->AddGameObject(gameObject);
	}

	LOG("Scene loaded successfully.");
	return true;
}

#pragma endregion