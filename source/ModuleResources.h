#pragma once

#include "Global.h"

#include "Module.h"

#include "resources/Resource.h"

#include "utils/Timer.h"

#include <map>

class ModuleResources : public Module
{
public:
	ModuleResources(bool startEnabled);

	virtual ~ModuleResources();

	bool Awake() override;
	bool Start() override;
	bool Update(float dt) override;

	bool CleanUp() override;

	UID Find(const std::string& assetPath) const;
	
	bool ImportFile(const std::string& assetPath, const std::string& libraryPath, const UID uid);
	bool LoadFile(const std::string& assetPath, const std::string& libraryPath, const UID uid);
	
	UID GenerateNewUID();
	
	const Resource* RequestResource(UID uid) const;
	Resource* RequestResource(UID uid);
	
	void ReleaseResource(UID uid);

private:
	bool CheckChangesInAssets();
	bool CheckFileLoaded(const std::string& assetPath);

	Resource* CreateNewResource(const std::string& assetPath, Resource::Type type);
	
	bool GetMetaInfo(const std::string& metaPath, UID& uid, int64_t& lastModificationTime);

	Resource::Type GetTypeFromExtension(const std::string& path);

	bool SaveMeta(const std::string& assetPath, UID uid);
		
private:
	std::map<UID, Resource*> resources;

	float timeToCheckAssets;
	Timer checkAssetsTimer;
};