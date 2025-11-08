#pragma once
#include "Module.h"
#include <string>

class Mesh;
class Texture;
class GameObject;
struct aiMesh;
struct aiMaterial;
struct aiScene;
struct aiNode;

class Loader : public Module
{
public:

	Loader(bool startEnabled);

	virtual ~Loader();

	bool Awake();
	bool Start();

	bool CleanUp();

	void HandleAssetDrop(const std::string& path);

	//MODELS
	bool LoadModel(const std::string& filePath);
	bool LoadFromAssimpMesh(aiMesh* assimpMesh, Mesh* mesh);
	GameObject* ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory);
	bool ProcessMeshComponents(GameObject* target, aiMesh* assimpMesh, const aiScene* scene, const std::string& modelDirectory);

	//TEXTURES
	bool LoadTexture(const std::string& filePath);
	bool LoadFromAssimpMaterial(aiMaterial* material, const std::string& modelDirectory, Texture* texture);
	
	//BASICS
	void CreateBasic(int basic);

private:
	void CreateCube();
	void CreateSphere();
	void CreatePyramid();
};