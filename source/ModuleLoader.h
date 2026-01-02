
#pragma once
#include "Module.h"
#include <string>
#include <vector>

class Mesh;
class Texture;
class GameObject;
class Config;
struct aiMesh;
struct aiMaterial;
struct aiScene;
struct aiNode;

class ModuleLoader : public Module
{
public:

	ModuleLoader(bool startEnabled);

	~ModuleLoader() override;

	bool Awake() override;
	bool Start() override;

	bool CleanUp() override;

	//MODELS
	bool LoadModel(const std::string& filePath);

	//TEXTURES
	bool LoadTextureToGameObjects(const std::string& filePath, std::vector<GameObject*> gameObject);
	bool LoadFromAssimpMaterial(aiMaterial* material, const std::string& modelDirectory, GameObject* obj);
	bool LoadTexture(const std::string& path, unsigned int& textureID, int& width, int& height, bool flip = false);
	
	//BASICS
	void LoadBasic(int basic);
	void LoadEmpty();

	//LOAD & SAVE
	bool SaveScene(const std::string& assetPath);
	bool LoadScene(const std::string& assetPath);
	bool CleanAndLoadScene(const std::string& assetPath);
	bool SaveSceneToMemory(Config& sceneConfig);
	bool LoadSceneFromMemory(Config& sceneConfig);

};