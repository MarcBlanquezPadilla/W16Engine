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
#include "resources/ResourceAnimation.h"

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
	checkChangesInterval = 1.0f;

	CreateInternalResources();
	CheckChangesInAssetsFolder();

	checkAssetsModifications = false;
	checkChangesTimer.Start();

	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::AssetsChanged, this);

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

	if (checkChangesTimer.ReadSec() > checkChangesInterval)
	{
		CheckChangesInAssetsFolder();
	}

	if (checkAssetsModifications)
	{
		//CHECK ASSETS FOLDER
		CheckForFilesModifications();
		checkAssetsModifications = false;
	}	

	return ret;
}

bool ModuleResources::CleanUp()
{
	bool ret = true;
	
	Engine::GetInstance().moduleEvents->Unsubscribe(Event::Type::AssetsChanged, this);

	LOG(LogType::LOG_INFO, "Deleting all resources.");

	for (auto& item : resources)
	{
		Resource* res = item.second;

		res->UnloadFromMemory_Internal();

		if (res != nullptr)
		{
			delete res;
			res = nullptr;
		}
	}

	resources.clear();

	return true;

}

bool ModuleResources::CheckChangesInAssetsFolder()
{
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
			LOG(LogType::LOG_INFO, "Asset deleted: UID %u.", uid);
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
		PublishAssetChangedEvent();
	}

	checkChangesTimer.Start();

	return dirtyAssets;
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

bool ModuleResources::CheckForFilesModifications()
{
	LOG(LogType::LOG_INFO, "Seraching external modifications in assets...");

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
	
	bool somethingModified = false;
	for (std::string path : assetsPaths	)
	{
		UID uid = 0;
		uint32_t fileHash = 0;
		std::string libraryPath;
		Resource::Type type = GetTypeFromExtension(path);

		if (DoesFileHasMeta(path))
		{
			GetMetaInfo(path, uid, fileHash);
			libraryPath = GetLibraryPath(uid);

			//IF FILE MODIFIED
			if (fileHash != GetFileHash(path))
			{
				somethingModified = true;
				return ImportFile(path, libraryPath, uid, type);
			}
		}
	}

	if (somethingModified) 
	{
		LOG(LogType::LOG_INFO, "Some files had changes and reimported.");
		return true;
	}

	LOG(LogType::LOG_INFO, "No changes found.");
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
	auto it = resources.find(parentUID);
	if (it == resources.end()) return;

	Resource* parentResource = it->second;

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

				bool alreadyChild = false;
				for (UID existingChild : parentResource->childs) {
					if (existingChild == childUID) { alreadyChild = true; break; }
				}
				if (!alreadyChild) {
					parentResource->childs.push_back(childUID);
				}

				std::string childLib = GetLibraryPath(childUID);
				int childType = refNode.GetInt("Type");
				std::string childAssetPath = assetPath;
				std::string childName = refNode.GetString("Name");

				CreateResource(childAssetPath, childLib, childUID, (Resource::Type)childType, childName);

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
		LOG(LogType::LOG_ERROR, "Trying to import file but not supported: %s", assetPath.c_str());
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

bool ModuleResources::CreateResource(const std::string& assetPath, const std::string& libraryPath, const UID uid, const Resource::Type type, const std::string& name, const bool internal)
{
	if (resources.find(uid) != resources.end())
	{
		Resource* res = resources[uid];

		res->UnloadFromMemory_Internal();

		if (!res->childs.empty())
		{
			res->childs.clear();
		}

		res->LoadToMemory_Internal();

		LOG(LogType::LOG_INFO, "Reloaded resource %s", assetPath.c_str());
		return true;
	}

	Resource* ret = nullptr;
	switch (type) {
		case Resource::texture: ret = new ResourceTexture(uid); break;
		case Resource::mesh: ret = new ResourceMesh(uid); break;
		case Resource::model: ret = new ResourceModel(uid); break;
		case Resource::scene: ret = new ResourceScene(uid); break;
		case Resource::animation: ret = new ResourceAnimation(uid); break;
	}

	if (ret != nullptr)
	{
		resources[uid] = ret;
		ret->name = name == "" ? assetPath : name;
		ret->assetPath = assetPath;
		ret->libraryPath = libraryPath;
		ret->internalResource = internal;
		LOG(LogType::LOG_INFO, "Created resource for %s", assetPath.c_str());
	}
	else
	{
		LOG(LogType::LOG_ERROR, "Failed in resource creation from %s", assetPath.c_str());
		return false;
	}

	return true;
}

bool ModuleResources::CreateInternalResources()
{
	#pragma region Basics
	std::string cubePath = GetLibraryPath(CUBE);
	std::string pyramidPath = GetLibraryPath(PYRAMID);
	std::string spherePath = GetLibraryPath(SPHERE);

	//CUBE
	if (!DoesFileExist(cubePath))
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Bone> noBones;

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

		importer->Import(cubePath, CUBE, Resource::Type::mesh, vertices, indices, noBones);

		delete importer;	
	}
	CreateResource("Internal resource", cubePath, CUBE, Resource::Type::mesh, "Cube mesh", true);
	

	// PYRAMID
	if (!DoesFileExist(pyramidPath))
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Bone> noBones;
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
		importer->Import(pyramidPath, PYRAMID, Resource::Type::mesh, vertices, indices, noBones);
		delete importer;
	}
	CreateResource("Internal resource", pyramidPath, PYRAMID, Resource::Type::mesh, "Pyramid mesh", true);
	

	//SPHERE
	if (!DoesFileExist(spherePath))
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Bone> noBones;

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

		importer->Import(spherePath, SPHERE, Resource::Type::mesh, vertices, indices, noBones);

		delete importer;
	}
	CreateResource("Internal resource", spherePath, SPHERE, Resource::Type::mesh, "Sphere mesh", true);

	#pragma endregion

	#pragma Icons
	std::string fileIconPath = GetLibraryPath(ICON_FILE);
	std::string folderIconPath = GetLibraryPath(ICON_FOLDER);
	std::string imageIconPath = GetLibraryPath(ICON_IMAGE);
	std::string meshIconPath = GetLibraryPath(ICON_MESH);
	std::string modelIconPath = GetLibraryPath(ICON_MODEL);
	std::string sceneIconPath = GetLibraryPath(ICON_SCENE);
	std::string scriptIconPath = GetLibraryPath(ICON_SCRIPT);
	std::string animationIconPath = GetLibraryPath(ICON_ANIMATION);

	if (!DoesFileExist(fileIconPath))
	{
		ImporterTexture* importer = new ImporterTexture();
		importer->Import("Resources/file.png", fileIconPath, ICON_FILE, Resource::Type::texture);
		delete importer;
	}
	
	if (!DoesFileExist(folderIconPath))
	{
		ImporterTexture* importer = new ImporterTexture();
		importer->Import("Resources/folder.png", folderIconPath, ICON_FOLDER, Resource::Type::texture);
		delete importer;
	}

	if (!DoesFileExist(imageIconPath))
	{
		ImporterTexture* importer = new ImporterTexture();
		importer->Import("Resources/image.png", imageIconPath, ICON_IMAGE, Resource::Type::texture);
		delete importer;
	}

	if (!DoesFileExist(meshIconPath))
	{
		ImporterTexture* importer = new ImporterTexture();
		importer->Import("Resources/mesh.png", meshIconPath, ICON_MESH, Resource::Type::texture);
		delete importer;
	}

	if (!DoesFileExist(modelIconPath))
	{
		ImporterTexture* importer = new ImporterTexture();
		importer->Import("Resources/model.png", modelIconPath, ICON_MODEL, Resource::Type::texture);
		delete importer;
	}

	if (!DoesFileExist(sceneIconPath))
	{
		ImporterTexture* importer = new ImporterTexture();
		importer->Import("Resources/scene.png", sceneIconPath, ICON_SCENE, Resource::Type::texture);
		delete importer;
	}

	if (!DoesFileExist(scriptIconPath))
	{
		ImporterTexture* importer = new ImporterTexture();
		importer->Import("Resources/script.png", scriptIconPath, ICON_SCRIPT, Resource::Type::texture);
		delete importer;
	}

	if (!DoesFileExist(animationIconPath))
	{
		ImporterTexture* importer = new ImporterTexture();
		importer->Import("Resources/animation.png", animationIconPath, ICON_ANIMATION, Resource::Type::texture);
		delete importer;
	}

	CreateResource("Internal resource", fileIconPath, ICON_FILE, Resource::Type::texture, "File icon", true);
	CreateResource("Internal resource", folderIconPath, ICON_FOLDER, Resource::Type::texture, "Folder icon", true);
	CreateResource("Internal resource", imageIconPath, ICON_IMAGE, Resource::Type::texture, "Image icon", true);
	CreateResource("Internal resource", meshIconPath, ICON_MESH, Resource::Type::texture, "Mesh icon", true);
	CreateResource("Internal resource", modelIconPath, ICON_MODEL, Resource::Type::texture, "Model icon", true);
	CreateResource("Internal resource", sceneIconPath, ICON_SCENE, Resource::Type::texture, "Scene icon", true);
	CreateResource("Internal resource", scriptIconPath, ICON_SCRIPT, Resource::Type::texture, "Script icon", true);
	CreateResource("Internal resource", animationIconPath, ICON_ANIMATION, Resource::Type::texture, "Animation icon", true);

	#pragma endregion
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

const Resource* ModuleResources::PeekResource(UID uid)
{
	auto it = resources.find(uid);
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
	bool anyUpdated = false;

	for (auto& [id, resource] : resources)
	{
		if (resource->assetPath == oldPath)
		{
			resource->assetPath = newPath;
			anyUpdated = true;
		}
	}

	if (anyUpdated)
	{
		LOG(LogType::LOG_INFO, "Resources moved successfully in memory from %s to %s", oldPath.c_str(), newPath.c_str());
	}
}

void ModuleResources::MoveFolder(const std::string& oldPath, const std::string& newPath)
{
	for (auto& [uid, resource] : resources)
	{
		std::string& resPath = resource->assetPath;
		std::string resourceStartPath = resource->assetPath;
		if (resPath.find(oldPath) == 0)
		{
			if (resPath.length() > oldPath.length() && resPath[oldPath.length()] == '/')
			{
				std::string newResPath = newPath + resPath.substr(oldPath.length());

				resource->assetPath = newResPath;

				LOG(LogType::LOG_INFO, "Resource updated in memory: %s -> %s (UID: %u)", resourceStartPath.c_str(), newResPath.c_str(), uid);
			}
		}
	}
}

void ModuleResources::RemoveResource(UID uid)
{
	auto it = resources.find(uid);

	if (it != resources.end())
	{
		Resource* resource = it->second;

		if (!resource->childs.empty())
		{
			std::vector<UID> childrenToDelete = resource->childs;
			for (UID childUID : childrenToDelete)
			{
				if (childUID != uid) RemoveResource(childUID);
			}
			resource->childs.clear();
		}

		for (auto& [otherID, otherRes] : resources)
		{
			if (otherRes->GetType() == Resource::Type::model)
			{
				ResourceModel* model = (ResourceModel*)otherRes;
			}
		}

		Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::ResourceDestroyed, uid));

		if (!resource->IsInteralResource())
		{
			std::string libPath = resource->GetLibraryFile();

			if (std::remove(libPath.c_str()) == 0)
			{
				LOG(LogType::LOG_INFO, "Deleted Library file: %s", libPath.c_str());
			}
			else
			{
				LOG(LogType::LOG_WARNING, "Warning: Could not delete Library file (or didn't exist): %s", libPath.c_str());
			}
		}

		resource->UnloadFromMemory_Internal();
		delete resource;
		resources.erase(it);

		LOG(LogType::LOG_INFO, "Resource removed/destroyed: UID %u", uid);
	}
}

void ModuleResources::PublishAssetChangedEvent()
{
	Engine::GetInstance().moduleEvents->PublishImmediate(Event::Type::AssetsChanged);
}

void ModuleResources::OnEvent(const Event& event)
{
	switch (event.type)
	{
	case Event::Type::AssetsChanged:
	{
		{
			std::string oldPath = event.data.strings.string1;
			std::string newPath = event.data.strings.string2;
			if (IsFileDirectory(oldPath)) MoveFolder(oldPath, newPath);
			else MoveResource(oldPath, newPath);
		}
		break;
	}

	default:
		break;
	}
}