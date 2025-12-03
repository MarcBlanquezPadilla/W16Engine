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
	bool ImportFile(const std::string& assetPath, const UID uid);
	bool LoadFile(const std::string& assetPath, const UID uid);
	UID GenerateNewUID();
	const Resource* RequestResource(UID uid) const;
	Resource* RequestResource(UID uid);
	void ReleaseResource(UID uid);
private:
	Resource* CreateNewResource(const std::string& assetPath, Resource::Type type);
	
	bool CheckChangesInAssets();
	bool CheckFileLoaded(const std::string& assetPath);

	UID GetUIDFromMeta(const std::string& assetPath);
		
private:
	std::map<UID, Resource*> resources;

	float timeToCheckAssets;
	Timer checkAssetsTimer;
};