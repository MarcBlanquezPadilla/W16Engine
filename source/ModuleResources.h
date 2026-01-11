#pragma once

#include "Global.h"

#include "Module.h"
#include "EventListener.h"

#include "resources/Resource.h"

#include "utils/Timer.h"

#include <map>

class ModuleResources : public Module, public EventListener
{
public:
	ModuleResources(bool startEnabled);

	~ModuleResources() override;

	bool Awake() override;
	bool Start() override;
	bool Update() override;
	bool CleanUp() override;

	//CHECKERS
	bool CheckChangesInAssetsFolder();
	bool CheckForFilesModifications();
	bool CheckFileLoaded(const std::string& assetPath);

	//RESURCES
	UID Find(const std::string& assetPath);
	const Resource* PeekResource(UID uid);
	Resource* RequestResource(UID uid);
	void ReleaseResource(UID uid);
	void RemoveResource(UID uid);
	void MoveResource(const std::string& oldPath, const std::string& newPath);
	void MoveFolder(const std::string& oldPath, const std::string& newPath);
	Resource::Type GetTypeFromExtension(const std::string& path);

	//EVENTS
	void OnEvent(const Event& event) override;
	void PublishAssetChangedEvent();

private:
	
	//IMPORT
	bool ImportFile(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type);

	//CHECKERS
	
	void CheckForSubResources(const std::string& assetPath, UID parentUID);
	
	//GETTERS
	bool GetMetaInfo(const std::string& assetPath, UID& uid, uint32_t& fileHash);
	bool GetMetaUID(const std::string& assetPath, UID& uid);

	
	//CREATE RESOURCES
	bool CreateResourceWithSubResources(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type);
	bool CreateResource(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type, const std::string& name = "", const bool internal = false);
	bool CreateInternalResources();
	bool TypeCanHaveSubResources(const Resource::Type type);
		
private:
	std::map<UID, Resource*> resources;

	float checkChangesInterval;
	Timer checkChangesTimer;

	bool checkAssetsModifications;
};