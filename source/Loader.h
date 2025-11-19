#pragma once
#include "Module.h"
#include "EventListener.h"
#include <string>

class Mesh;
class Texture;
class GameObject;
struct aiMesh;
struct aiMaterial;
struct aiScene;
struct aiNode;

class Loader : public Module, public EventListener
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
	bool AddMeshAndTextureFromAssimp(GameObject* target, aiMesh* assimpMesh, const aiScene* scene, const std::string& modelDirectory);

	//TEXTURES
	bool LoadTexture(const std::string& filePath);
	bool LoadFromAssimpMaterial(aiMaterial* material, const std::string& modelDirectory, Texture* texture);
	
	//BASICS
	void CreateBasic(int basic);

	//LOAD & SAVE
	bool SaveScene();
	bool LoadScene();

	//EVENTS
	void OnEvent(const Event& event) override;

private:
	void CreateCube();
	void CreateSphere();
	void CreatePyramid();
};