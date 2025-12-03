#include "ModuleResources.h"

#include "utils/FileUtils.h"
#include "utils/Log.h"
#include "utils/Config.h"

#include <vector>
#include <string>
#include <random>



ModuleResources::ModuleResources(bool startEnabled) : Module(startEnabled)
{
	name = "resources";
}

ModuleResources::~ModuleResources()
{

}

bool ModuleResources::Awake()
{
	bool ret = true;

	timeToCheckAssets = 1.0f;
	CheckChangesInAssets();

	return true;
}


bool ModuleResources::Start()
{
	bool ret = true;

	return ret;
}
bool ModuleResources::Update(float dt)
{
	bool ret = true;

	if (checkAssetsTimer.ReadSec() > timeToCheckAssets)
		CheckChangesInAssets();

	return ret;
}

bool ModuleResources::CleanUp()
{
	bool ret = true;
	
	return true;
}

bool ModuleResources::CheckChangesInAssets()
{
	std::vector<std::string> allPaths = GetListDirectoryContents("Assets", true);
	std::vector<std::string> assetsPaths;
	assetsPaths.clear();

	for (std::string path : allPaths)
	{
		if (IsFileDirectory(path)) continue;
		if (GetFileExtension(path) == "meta") continue;

		assetsPaths.push_back(path);
	}

	bool dirtyAssets = false;

	for (std::string assetPath : assetsPaths)
	{
		if (CheckFileLoaded(assetPath))
		{
			dirtyAssets = true;
		}
	}

	if (dirtyAssets) LOG("dirty");

	checkAssetsTimer.Start();
	
	return true;
}

bool ModuleResources::CheckFileLoaded(const std::string& assetPath)
{
	UID uid = DoesFileHasMeta(assetPath) ? GetUIDFromMeta(GetMetaPath(assetPath)) : 0;

	//IF NEW FILE
	if (uid == 0)
	{
		uid = GenerateNewUID();
		return ImportFile(assetPath, uid);
	}

	//IF NEEDS REIMPORT
	if (!DoesFileExist(GetLibraryPath(uid)))
	{
		return ImportFile(assetPath, uid);
	}

	//IF NEEDS LOAD
	if (resources.find(uid) == resources.end())
	{
		return LoadFile(assetPath, uid);
	}

	return false;
}

bool ModuleResources::ImportFile(const std::string& assetPath, const UID uid)
{
	
	return true;
}

bool ModuleResources::LoadFile(const std::string& assetPath, const UID uid)
{

	return true;
}

UID ModuleResources::GetUIDFromMeta(const std::string& metaPath)
{
	Config meta;

	if (meta.Load(metaPath.c_str()))
	{
		UID uid = (UID)meta.GetUInt("UID", 0);

		return uid;
	}

	return 0;
}


UID ModuleResources::GenerateNewUID()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
	return dis(gen);
}