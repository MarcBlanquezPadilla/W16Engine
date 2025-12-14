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
#include "importers/ImporterMesh.h"

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

	CreateInternalResources();
	
	CheckChangesInAssets();
	updateAssets = false;

	return true;
}


bool ModuleResources::Start()
{
	bool ret = true;

	return ret;
}
bool ModuleResources::Update()
{
	bool ret = true;

	if (updateAssets)
	{
		//CHECK ASSETS FOLDER
		CheckChangesInAssets();
		updateAssets = false;
	}	

	return ret;
}

bool ModuleResources::CleanUp()
{
	bool ret = true;
	
	return true;
}

bool ModuleResources::CheckChangesInAssets()
{
	LOG("SEARCHING FOR CHANGES IN ASSETS...");

	std::vector<std::string> allPaths = GetListDirectoryContents("Assets", true);
	std::vector<std::string> assetsPaths;
	assetsPaths.clear();

	//SEPARATE ASSETS FROM OTHER FILES
	for (std::string path : allPaths)
	{
		if (IsFileDirectory(path)) continue;
		if (GetTypeFromExtension(path) == Resource::Type::unknown) continue;

		assetsPaths.push_back(path);
	}

	bool dirtyAssets = false;

	//CHECK IF REMOVED
	std::vector<UID> uidsToRemove;

	for (auto const& [uid, resource] : resources)
	{
		if (!resource->IsInteralResource() && !DoesFileExist(resource->GetAssetFile()))
		{
			uidsToRemove.push_back(uid);
		}
	}

	if (uidsToRemove.size() != 0)
	{
		for (UID uid : uidsToRemove)
		{
			RemoveResource(uid);
			LOG("Asset deleted: UID %u", uid);
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

	if (dirtyAssets)
	{
		LOG("FOUND CHANGES AND APPLIED TO FOLDER.");
		PublishAssetChangedEvent();
	}
	else
	{
		LOG("NO CHANGES FOUND.");
	}
	
	return true;
}

bool ModuleResources::CheckFileLoaded(const std::string& assetPath)
{
	UID uid = 0;
	uint32_t fileHash = 0;
	std::string libraryPath;
	Resource::Type type = GetTypeFromExtension(assetPath);

	//IF NEW FILE
	if (!DoesFileHasMeta(assetPath))
	{
		uid = GenerateNewUID();
		libraryPath = GetLibraryPath(uid);
		return ImportFile(assetPath, libraryPath, uid, type);
	}

	GetMetaInfo(assetPath, uid, fileHash);
	libraryPath = GetLibraryPath(uid);

	//IF FILE MODIFIED
	if (fileHash != GetFileHash(assetPath))
	{
		return ImportFile(assetPath, libraryPath, uid, type);
	}

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
					int childType = refNode.GetInt("Type");
					std::string childAssetPath = refNode.GetString("Path");

					CreateResource(childAssetPath, childLib, childUID, (Resource::Type)childType);
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
		resources[uid]->UnloadFromMemory_Internal();
		resources[uid]->LoadToMemory_Internal();
		LOG("Reloaded resource %s", assetPath.c_str());
		return true;
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
		LOG("Created resource for %s", assetPath.c_str());
	}
	else
	{
		LOG("Failed in resource creation from %s", assetPath.c_str());
		return false;
	}

	return true;
}


bool ModuleResources::CreateInternalResources()
{
	std::string cubePath = GetLibraryPath(CUBE);
	std::string pyramidPath = GetLibraryPath(PYRAMID);
	std::string spherePath = GetLibraryPath(SPHERE);

	//CUBE
	if (!DoesFileExist(cubePath))
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		vertices = {

			{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
			{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
			{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
			{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

			{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
			{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)},
			{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)},
			{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)},

			{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
			{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
			{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
			{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

			{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
			{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
			{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
			{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

			{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
			{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
			{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
			{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
			{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
			{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
			{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
			{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}
		};

		indices = {
			0, 1, 2,  2, 3, 0,
			4, 5, 6,  6, 7, 4,
			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20
		};

		ImporterMesh* importer = new ImporterMesh();

		importer->Import(cubePath, CUBE, Resource::Type::mesh, vertices, indices);

		delete importer;	
	}
	CreateResource("Internal resource", cubePath, CUBE, Resource::Type::mesh);
	

	// PYRAMID
	if (!DoesFileExist(pyramidPath))
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		glm::vec3 apex = glm::vec3(0.0f, 0.5f, 0.0f);

		vertices = {
		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f)), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f)), glm::vec2(1.0f, 0.0f)},
		{apex, glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f)), glm::vec2(0.5f, 1.0f)},

		{glm::vec3(0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(0.5f, 0.5f, 0.0f)), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(0.5f, 0.5f, 0.0f)), glm::vec2(1.0f, 0.0f)},
		{apex, glm::normalize(glm::vec3(0.5f, 0.5f, 0.0f)), glm::vec2(0.5f, 1.0f)},

		{glm::vec3(0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, -0.5f)), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(0.0f, 0.5f, -0.5f)), glm::vec2(1.0f, 0.0f)},
		{apex, glm::normalize(glm::vec3(0.0f, 0.5f, -0.5f)), glm::vec2(0.5f, 1.0f)},

		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::normalize(glm::vec3(-0.5f, 0.5f, 0.0f)), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::normalize(glm::vec3(-0.5f, 0.5f, 0.0f)), glm::vec2(1.0f, 0.0f)},
		{apex, glm::normalize(glm::vec3(-0.5f, 0.5f, 0.0f)), glm::vec2(0.5f, 1.0f)},
		};

		indices = {
			0, 1, 3,  1, 2, 3,
			4, 5, 6,
			7, 8, 9,
			10, 11, 12,
			13, 14, 15
		};

		ImporterMesh* importer = new ImporterMesh();
		importer->Import(pyramidPath, PYRAMID, Resource::Type::mesh, vertices, indices);
		delete importer;
	}
	CreateResource("Internal resource", pyramidPath, PYRAMID, Resource::Type::mesh);
	

	//SPHERE
	if (!DoesFileExist(spherePath))
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		const int sectors = 36;
		const int stacks = 18;
		const float radius = 0.5f;

		for (int i = 0; i <= stacks; ++i) {
			float V = (float)i / (float)stacks;
			float phi = V * glm::pi<float>();

			for (int j = 0; j <= sectors; ++j) {
				float U = (float)j / (float)sectors;
				float theta = U * (glm::pi<float>() * 2.0f);

				float x = cos(theta) * sin(phi);
				float y = cos(phi);
				float z = sin(theta) * sin(phi);

				glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));

				vertices.push_back({
					{x * radius, y * radius, z * radius},
					normal,
					{U, V}
					});
			}
		}

		for (int i = 0; i < stacks; ++i) {
			for (int j = 0; j < sectors; ++j) {
				int first = (i * (sectors + 1)) + j;
				int second = first + sectors + 1;


				indices.push_back(first);
				indices.push_back(first + 1);
				indices.push_back(second);

				indices.push_back(first + 1);
				indices.push_back(second + 1);
				indices.push_back(second);
			}
		}

		ImporterMesh* importer = new ImporterMesh();

		importer->Import(spherePath, SPHERE, Resource::Type::mesh, vertices, indices);

		delete importer;
	}
	CreateResource("Internal resource", spherePath, SPHERE, Resource::Type::mesh);

	Resource* cube = RequestResource(CUBE);
	Resource* sphere = RequestResource(SPHERE);
	Resource* pyramid = RequestResource(PYRAMID);

	if (cube) cube->internalResource = true;
	if (sphere) sphere->internalResource = true;
	if (pyramid) pyramid->internalResource = true;

	return true;
}

UID ModuleResources::Find(const std::string& assetPath)
{
	UID uid = 0;

	if (GetMetaUID(assetPath, uid))
	{
		return uid; 
	}

	return 0;
}

bool ModuleResources::GetMetaUID(const std::string& assetPath, UID& uid)
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

bool ModuleResources::GetMetaInfo(const std::string& assetPath, UID& uid, uint32_t& fileHash)
{
	Config meta;
	std::string metaPath = assetPath + ".meta";
	if (meta.Load(metaPath.c_str()))
	{
		uid = (UID)meta.GetUInt("UID");
		fileHash = meta.GetUInt("FileHash");

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

void ModuleResources::MoveResource(const std::string& oldPath, const std::string& newPath)
{
	UID uid = 0;

	for (auto& [id, resource] : resources)
	{
		if (resource->assetPath == oldPath)
		{
			uid = id;
			break;
		}
	}

	if (uid == 0)
	{
		uid = Find(newPath);
	}

	if (uid != 0)
	{
		Resource* res = RequestResource(uid);
		if (res)
		{
			res->assetPath = newPath;
			LOG("Resource updated in memory: %s -> %s (UID: %u)", oldPath.c_str(), newPath.c_str(), uid);
		}

		ReleaseResource(uid);
	}
	else
	{
		LOG("Error: Could not find UID for moved resource. Old: %s", oldPath.c_str());
	}
}

void ModuleResources::RemoveResource(UID uid)
{
	auto it = resources.find(uid);

	if (it != resources.end())
	{
		Resource* resource = it->second;

		resource->UnloadFromMemory_Internal();

		delete resource;

		resources.erase(it);

		LOG("Resource removed/destroyed: UID %u", uid);
	}
}

void ModuleResources::PublishAssetChangedEvent()
{
	Engine::GetInstance().moduleEvents->PublishImmediate(Event::Type::AssetsChanged);
}