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
	Engine::GetInstance().moduleScene->AddGameObject(rootGameObject);

	return true;
}

GameObject* ModuleLoader::ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory)
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

		if (!AddMeshAndTextureFromAssimp(nodeGameObject, assimpMesh, scene, modelDirectory))
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

			if (AddMeshAndTextureFromAssimp(meshGameObject, assimpMesh, scene, modelDirectory))
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

bool ModuleLoader::AddMeshAndTextureFromAssimp(GameObject* target, aiMesh* assimpMesh, const aiScene* scene, const std::string& modelDirectory)
{
	if (target)
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
			if (target != nullptr)
			{
				LoadFromAssimpMaterial(material, modelDirectory, target);
			}
		}
		return true;
	}
	return false;
}

bool ModuleLoader::LoadFromAssimpMesh(aiMesh* assimpMesh, Mesh* mesh)
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

void ModuleLoader::CreateBasic(int basic)
{
	switch (basic)
	{
	case EMPTY:
		CreateEmpty();
		break;
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

void ModuleLoader::CreateEmpty()
{
	GameObject* gameObject = new GameObject(true, "Empty");

	if (gameObject)
	{
		Engine::GetInstance().moduleScene->AddGameObject(gameObject);
	}
}

void ModuleLoader::CreateCube()
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
		Engine::GetInstance().moduleScene->AddGameObject(gameObject);
	}
}

void ModuleLoader::CreateSphere()
{
	GameObject* gameObject = new GameObject(true, "Sphere");
	Mesh* mesh = (Mesh*)gameObject->AddComponent(ComponentType::Mesh);
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	const int sectors = 36;
	const int stacks = 18;
	const float radius = 0.5f;

	for (int i = 0; i <= stacks; ++i) {
		float V = (float)i / (float)stacks;
		float phi = V * glm::pi<float>();

		for (int j = 0; j <= sectors; ++j) {
			float U = (float)j / (float)sectors;
			float theta = U * (glm::pi<float>() * 2.0f);

			float x = cos(theta) * sin(phi);
			float y = cos(phi);
			float z = sin(theta) * sin(phi);

			glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));

			vertices.push_back({
				{x * radius, y * radius, z * radius},
				normal,
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
		Engine::GetInstance().moduleScene->AddGameObject(gameObject);
	}
}

void ModuleLoader::CreatePyramid()
{
	GameObject* gameObject = new GameObject(true, "Pyramid");
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