#pragma once
#include "Module.h"
#include <string>

class Loader : public Module
{
public:

	Loader(bool startEnabled);

	virtual ~Loader();

	bool Awake();
	bool Start();

	bool CleanUp();

	void HandleAssetDrop(const std::string& path);

	bool LoadModel(const std::string& filePath);
	bool LoadTexture(const std::string& filePath);
	
	void CreateBasic(int basic);

public:

};