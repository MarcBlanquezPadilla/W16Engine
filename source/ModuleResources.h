#pragma once

#include "Global.h"

#include "Module.h"

#include "resources/Resource.h"

#include <map>

class ModuleResources : public Module
{
public:
	ModuleResources(bool startEnabled);

	virtual ~ModuleResources();

	bool Awake() override;
	bool Start() override;
	bool Update() override;
	bool CleanUp() override;

	//CHECKERS
	bool CheckChangesInAssets();
	bool CheckFileLoaded(const std::string& assetPath);

	//RESURCES
	UID Find(const std::string& assetPath);
	const Resource* RequestResource(UID uid) const;
	Resource* RequestResource(UID uid);
	void ReleaseResource(UID uid);
	void RemoveResource(UID uid);
	void MoveResource(const std::string& oldPath, const std::string& newPath);

	//EVENTS
	void PublishAssetChangedEvent();

private:
	
	//IMPORT
	bool ImportFile(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type);

	//CHECKERS
	
	void CheckForSubResources(const std::string& assetPath, UID parentUID);
	
	//GETTERS
	bool GetMetaInfo(const std::string& assetPath, UID& uid, uint32_t& fileHash);
	bool GetMetaUID(const std::string& assetPath, UID& uid);
	Resource::Type GetTypeFromExtension(const std::string& path);
	
	//CREATE RESOURCES
	bool CreateResourceWithSubResources(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type);
	bool CreateResource(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type);
	bool CreateInternalResources();
	bool TypeCanHaveSubResources(const Resource::Type type);
		
private:
	std::map<UID, Resource*> resources;

	bool updateAssets;
};