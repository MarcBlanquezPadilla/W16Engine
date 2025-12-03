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

	UID Find(const char* file_in_assets) const;
	bool ImportFile(const char* new_file_in_assets, const UID uid);
	bool LoadFile(const char* new_file_in_assets, const UID uid);
	UID GenerateNewUID();
	const Resource* RequestResource(UID uid) const;
	Resource* RequestResource(UID uid);
	void ReleaseResource(UID uid);
private:
	Resource* CreateNewResource(const char* assetsFile, Resource::Type type);
	bool CheckChangesInAssets();

	UID GetUIDFromMeta(const std::string& assetPath);
		
private:
	std::map<UID, Resource*> resources;

	float timeToCheckAssets;
	Timer checkAssetsTimer;
};