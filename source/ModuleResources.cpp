#include "ModuleResources.h"

#include "utils/FileUtils.h"
#include "utils/Log.h"
#include "utils/Config.h"

#include "importers/Importer.h"
#include "importers/ImporterTexture.h"

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
	UID uid = 0;
	std::string libraryPath;
	int64_t lastModificationTime;

	//IF NEW FILE
	if (!DoesFileHasMeta(assetPath))
	{
		uid = GenerateNewUID();
		libraryPath = GetLibraryPath(uid);
		return ImportFile(assetPath, libraryPath, uid);
	}

	GetMetaInfo(assetPath, uid, lastModificationTime);
	libraryPath = GetLibraryPath(uid);

	//IF LIBRARY MISSING
	if (!DoesFileExist(libraryPath))
	{
		return ImportFile(assetPath, libraryPath, uid);
	}

	//IF FILE MODIFICATED
	int64_t assetTime = GetLastModificationTime(assetPath);
	int64_t metaTime = lastModificationTime;
	if (assetTime > metaTime)
	{
		return ImportFile(assetPath, libraryPath,uid) != 0;
	}

	//IF NOT LOADED
	if (resources.find(uid) == resources.end())
	{
		return LoadFile(assetPath, libraryPath, uid);
	}

	return false;
}

bool ModuleResources::ImportFile(const std::string& assetPath, const std::string& libraryPath, const UID uid)
{
	const Resource::Type type = GetTypeFromExtension(assetPath);

	Importer* importer = nullptr;
	bool succes = false;

	switch (type)
	{
	case Resource::texture:
		importer = new ImporterTexture();
		break;

	case Resource::unknown:
		//LOG("File not supported: %s", assetPath.c_str());
		break;
	}

	if (importer)
	{
		succes = importer->Import(assetPath, libraryPath, uid, type);
		delete importer;
	}

	if (succes)
	{
		LOG("Asset imported: %s", assetPath.c_str());
		if(SaveMeta(assetPath, uid)) LOG("Failed saving meta of: %s", assetPath.c_str());
		LoadFile(assetPath, libraryPath, uid);
		return true;
	}
	
	LOG("Failed on asset import: %s", assetPath.c_str());
	return false;
}

bool ModuleResources::LoadFile(const std::string& assetPath, const std::string& libraryPath, const UID uid)
{

	return true;
}

bool ModuleResources::GetMetaInfo(const std::string& assetPath, UID& uid, int64_t& lastModificationTime)
{
	Config meta;
	std::string metaPath = assetPath + ".meta";
	if (meta.Load(metaPath.c_str()))
	{
		uid = (UID)meta.GetUInt("UID");
		lastModificationTime = meta.GetInt64("ModificationTime");

		return true;
	}
	return false;
}

bool ModuleResources::SaveMeta(const std::string& assetPath, UID uid)
{
	Config meta;

	meta.SetUInt("UID", uid);

	int64_t lastModificationTime = GetLastModificationTime(assetPath);
	meta.SetInt64("ModificationTime", lastModificationTime);

	return meta.Save(GetMetaPath(assetPath).c_str());
}

UID ModuleResources::GenerateNewUID()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
	return dis(gen);
}

Resource::Type ModuleResources::GetTypeFromExtension(const std::string& path)
{
	std::string ext = GetFileExtension(path);

	if (ext == "fbx" || ext == "obj" || ext == "dae" || ext == "gltf" || ext == "glb")
	{
		return Resource::Type::mesh;
	}

	if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "tga" || ext == "dds" || ext == "bmp" || ext == "tif")
	{
		return Resource::Type::texture;
	}

	if (ext == "wscene")
	{
		return Resource::Type::scene;
	}

	return  Resource::Type::unknown;
}