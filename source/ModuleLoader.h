
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

class ModuleLoader : public Module, public EventListener
{
public:

	ModuleLoader(bool startEnabled);

	virtual ~ModuleLoader();

	bool Awake() override;
	bool Start() override;

	bool CleanUp() override;

	void HandleAssetDrop(const std::string& path);

	//MODELS
	bool LoadModel(const std::string& filePath);

	//TEXTURES
	bool LoadTextureToGameObject(const std::string& filePath, GameObject* gameObject);
	bool LoadFromAssimpMaterial(aiMaterial* material, const std::string& modelDirectory, GameObject* obj);
	bool LoadTexture(const std::string& path, unsigned int& textureID, int& width, int& height, bool flip = false);
	
	//BASICS
	void LoadBasic(int basic);
	void LoadEmpty();

	//LOAD & SAVE
	bool SaveScene(const std::string& assetPath);
	bool LoadScene(const std::string& assetPath);

	//EVENTS
	void OnEvent(const Event& event) override;
};