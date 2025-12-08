#include "Engine.h"
#include "ModuleResources.h"
#include "ModuleEvents.h"

#include "utils/FileUtils.h"
#include "utils/Log.h"
#include "utils/Config.h"

#include "importers/Importer.h"
#include "importers/ImporterTexture.h"
#include "importers/ImporterScene.h"
#include "importers/ImporterModel.h"

#include "resources/Resource.h"
#include "resources/ResourceTexture.h"
#include "resources/ResourceScene.h"
#include "resources/ResourceModel.h"
#include "resources/ResourceMesh.h"

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

	//CHECK IF REMOVED
	std::vector<UID> uidsToRemove;

	for (auto const& [uid, resource] : resources)
	{
		if (!DoesFileExist(resource->GetAssetFile()))
		{
			uidsToRemove.push_back(uid);
		}
	}

	if (uidsToRemove.size() != 0)
	{
		for (UID uid : uidsToRemove)
		{
			LOG("Asset deleted or moved (Resource removed): UID %u", uid);
		}
		dirtyAssets = true;
	}

	//CHECK IF NOT IMPORTED OR EDITED
	for (std::string assetPath : assetsPaths)
	{
		if (CheckFileLoaded(assetPath))
		{
			dirtyAssets = true;
		}
	}

	if (dirtyAssets) PublishAssetChangedEvent();

	checkAssetsTimer.Start();
	
	return true;
}

bool ModuleResources::CheckFileLoaded(const std::string& assetPath)
{
	UID uid = 0;
	std::string libraryPath;
	Resource::Type type = GetTypeFromExtension(assetPath);

	//IF NEW FILE
	if (!DoesFileHasMeta(assetPath))
	{
		uid = GenerateNewUID();
		libraryPath = GetLibraryPath(uid);
		return ImportFile(assetPath, libraryPath, uid, type);
	}

	GetMetaInfo(assetPath, uid);
	libraryPath = GetLibraryPath(uid);

	//IF LIBRARY MISSING
	if (!DoesFileExist(libraryPath))
	{
		return ImportFile(assetPath, libraryPath, uid, type);
	}

	//IF IMPORTED BUT NOT CREATED
	if (resources.find(uid) == resources.end())
	{
		return CreateResourceWithSubResources(assetPath, libraryPath, uid, type);
	}

	return false;
}

bool ModuleResources::TypeCanHaveSubResources(const Resource::Type type)
{
	if (type == Resource::model) return true;
	else return false;
}

bool ModuleResources::CreateResourceWithSubResources(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type)
{
	bool created = CreateResource(assetPath, libraryPath, uid, type);

	if (created && TypeCanHaveSubResources(type))
	{
		CheckForSubResources(assetPath, uid);
	}

	return created;
}

void ModuleResources::CheckForSubResources(const std::string& assetPath, UID parentUID)
{
	Config meta;
	if (meta.Load((assetPath + ".meta").c_str()))
	{
		unsigned int count = meta.GetUInt("ReferedObjects");

		if (count > 0)
		{
			Config refNode = meta.GetChild("ReferedObject");

			while (refNode.IsValid())
			{
				UID childUID = (UID)refNode.GetUInt("UID");

				if (resources.find(childUID) == resources.end())
				{
					std::string childLib = GetLibraryPath(childUID);
					int childType = refNode.GetInt("UID");
					std::string childAssetPath = refNode.GetString("Path");

					CreateResource(childAssetPath, childLib, childUID, (Resource::Type)childType);

					LOG("Sub-resource registered: UID %u", childUID);
				}

				refNode = refNode.GetNextSibling("ReferedObject");
			}
		}
	}
	meta.CleanUp();
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

	case Resource::model:
		importer = new ImporterModel();
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
		return CreateResourceWithSubResources(assetPath, libraryPath, uid, type);
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
		case Resource::mesh: ret = new ResourceMesh(uid); break;
		case Resource::model: ret = new ResourceModel(uid); break;
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

bool ModuleResources::GetMetaInfo(const std::string& assetPath, UID& uid)
{
	Config meta;
	std::string metaPath = assetPath + ".meta";
	if (meta.Load(metaPath.c_str()))
	{
		uid = (UID)meta.GetUInt("UID");

		return true;
	}
	return false;
}

Resource::Type ModuleResources::GetTypeFromExtension(const std::string& path)
{
	std::string ext = GetFileExtension(path);

	if (ext == "fbx" || ext == "obj" || ext == "dae" || ext == "gltf" || ext == "glb")
	{
		return Resource::Type::model;
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

void ModuleResources::PublishAssetChangedEvent()
{
	Engine::GetInstance().moduleEvents->PublishImmediate(Event::Type::AssetsChanged);
}