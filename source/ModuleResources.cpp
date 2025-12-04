#include "Engine.h"
#include "ModuleResources.h"
#include "ModuleEvents.h"

#include "utils/FileUtils.h"
#include "utils/Log.h"
#include "utils/Config.h"

#include "importers/Importer.h"
#include "importers/ImporterTexture.h"
#include "importers/ImporterScene.h"

#include "resources/Resource.h"
#include "resources/ResourceTexture.h"
#include "resources/ResourceScene.h"

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

	//SEPARATE ASSETS FROM OTHER FILES
	for (std::string path : allPaths)
	{
		if (IsFileDirectory(path)) continue;
		if (GetFileExtension(path) == "meta") continue;

		assetsPaths.push_back(path);
	}

	bool dirtyAssets = false;

	//CHECK ASSETS
	for (std::string assetPath : assetsPaths)
	{
		if (CheckFileLoaded(assetPath))
		{
			dirtyAssets = true;
		}
	}

	if (dirtyAssets) Engine::GetInstance().moduleEvents->PublishImmediate(Event::Type::AssetsChanged);

	checkAssetsTimer.Start();
	
	return true;
}

bool ModuleResources::CheckFileLoaded(const std::string& assetPath)
{
	UID uid = 0;
	std::string libraryPath;
	int64_t lastModificationTime;
	Resource::Type type = GetTypeFromExtension(assetPath);

	//IF NEW FILE
	if (!DoesFileHasMeta(assetPath))
	{
		uid = GenerateNewUID();
		libraryPath = GetLibraryPath(uid);
		return ImportFile(assetPath, libraryPath, uid, type);
	}

	GetMetaInfo(assetPath, uid, lastModificationTime);
	libraryPath = GetLibraryPath(uid);

	//IF LIBRARY MISSING
	if (!DoesFileExist(libraryPath))
	{
		return ImportFile(assetPath, libraryPath, uid, type);
	}

	//IF FILE MODIFICATED
	int64_t assetTime = GetLastModificationTime(assetPath);
	int64_t metaTime = lastModificationTime;
	if (assetTime > metaTime)
	{
		return ImportFile(assetPath, libraryPath, uid, type);
	}

	//IF IMPORTED BUT NOT CREATED
	if (resources.find(uid) == resources.end())
	{
		return CreateResource(assetPath, libraryPath, uid, type);
	}

	return false;
}

bool ModuleResources::ImportFile(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type)
{
	Importer* importer = nullptr;
	bool success = false;

	switch (type)
	{
	case Resource::texture:
		importer = new ImporterTexture();
		break;

	case Resource::scene:
		importer = new ImporterScene();
		break;

	case Resource::unknown:
		LOG("Trying to import file but not supported: %s", assetPath.c_str());
		break;
	}

	if (importer)
	{
		success = importer->Import(assetPath, libraryPath, uid, type);
		delete importer;
	}

	if (success)
	{
		if (!SaveMeta(assetPath, uid)) LOG("Failed saving meta");

		if (resources.find(uid) != resources.end())
		{
			Resource* res = resources[uid];

			if (res->IsLoadedToMemory())
			{
				res->UnloadFromMemory_Internal();
				res->LoadToMemory_Internal();
			}

			LOG("Resource re-imported and reloaded: %s", assetPath.c_str());
			return true;
		}
		else
		{
			return CreateResource(assetPath, libraryPath, uid, type);
		}
	}

	return false;
}

bool ModuleResources::CreateResource(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type)
{	
	if (resources.find(uid) != resources.end())
	{
		return false;
	}

	Resource* ret = nullptr;
	switch (type) {
		case Resource::texture: ret = new ResourceTexture(uid); break;
		//case Resource::mesh: ret = (Resource*) new ResourceMesh(uid); break;
		case Resource::scene: ret = new ResourceScene(uid); break;
		//case Resource::bone: ret = (Resource*) new ResourceBone(uid); break;
		//case Resource::animation: ret = (Resource*) new ResourceAnimation(uid); break;
	}
	if (ret != nullptr)
	{
		resources[uid] = ret;
		ret->assetPath = assetPath;
		ret->libraryPath = libraryPath;
	}
	else
	{
		LOG("Failed in resource creation from %s", assetPath.c_str());
		return false;
	}

	return true;
}


UID ModuleResources::Find(const std::string& assetPath)
{
	UID uid = 0;

	if (GetMetaInfo(assetPath, uid))
	{
		return uid; 
	}

	return 0;
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

bool ModuleResources::GetMetaInfo(const std::string& assetPath, UID& uid)
{
	int64_t ignoredTime;
	return GetMetaInfo(assetPath, uid, ignoredTime);
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

Resource* ModuleResources::RequestResource(UID uid)
{
	auto it = resources.find(uid);

	if (it != resources.end())
	{
		Resource* res = it->second;

		res->LoadToMemory();

		return res;
	}

	return nullptr;
}

const Resource* ModuleResources::RequestResource(UID uid) const
{
	std::map<UID, Resource*>::const_iterator it = resources.find(uid);

	if (it != resources.end())
	{
		return it->second;
	}

	return nullptr;
}

void ModuleResources::ReleaseResource(UID uid)
{
	auto it = resources.find(uid);

	if (it != resources.end())
	{
		Resource* res = it->second;

		res->UnloadFromMemory();
	}
}