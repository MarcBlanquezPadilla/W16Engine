#include "Resources.h"
#include "Engine.h"
#include "utils/Log.h"
#include "utils/FileUtils.h"

#include "importers/Importer.h"
#include "importers/MeshImporter.h"
#include "importers/ModelImporter.h"
#include "utils/FileUtils.h"

// Includes de tus recursos específicos (los crearemos luego)
// #include "ResourceMesh.h"
// #include "ResourceTexture.h"

#include <random>
#include <fstream>

Resources::Resources(bool startEnabled) : Module(startEnabled)
{
    name = "Resources";
}

Resources::~Resources()
{

}

bool Resources::Awake()
{
    std::vector<std::string> assetsPaths = GetListDirectoryContents("Assets", true);

    for (std::string assetPath : assetsPaths)
    {
        if (!IsFileDirectory(assetPath))
        {
            if (GetFileExtension(assetPath) == ".meta") continue;
            LoadFile(assetPath);
        }
    }

    return true;
}

bool Resources::CleanUp()
{
    for (auto& pair : resources)
    {
        delete pair.second;
    }
    resources.clear();
    return true;
}

uint32_t Resources::LoadFile(const std::string& assetPath)
{
    std::string metaPath = assetPath + ".meta";

    if (DoesFileExist(metaPath)) {

        uint32_t uid = ReadUIDFromMeta(metaPath);

        if (resources.count(uid) > 0)
        {
            // TODO: Comprobar si el Timestamp en el meta es más viejo que el asset real
            // Si es más viejo, llamar a ImportFile(assetPath, uid) para re-importar.
            return uid;
        }

        std::string libPath = GenerateLibraryPath(uid);

        if (DoesFileExist(libPath)) {
            
            ResourceType type = GetTypeFromExtension(assetPath);
            Resource* res = CreateNewResource(assetPath, libPath, type, uid);

            if (res)
            {
                resources[uid] = res;
                LOG("Resource loaded from library: %s (UID: %d)", assetPath.c_str(), uid);
                return uid;
            }
        }
        else
        {
            LOG("Library file missing for %s. Re-importing...", assetPath.c_str());
            return ImportFile(assetPath, uid);
        }
    }
    else
    {
        LOG("New asset detected: %s. Importing...", assetPath.c_str());
        return ImportFile(assetPath, 0);
    }
   
    return 0;
}

uint32_t Resources::ImportFile(const std::string& assetPath, uint32_t uuid)
{
    uint32_t uid = uuid;
    if (uid == 0) uid = GenerateNewUID();

    std::string libraryPath = GenerateLibraryPath(uid);

    ResourceType type = GetTypeFromExtension(assetPath);

    for (auto const& [uid, res] : resources)
    {
        if (res->GetAssetsFile() == assetPath)
            return uid;
    }

    if (type == ResourceType::UNKNOWN)
    {
        LOG("Error: Type of file isn't supported: %s", assetPath.c_str());
        return 0;
    }

   

    Importer* importer = nullptr;

    switch (type) {
        case ResourceType::TEXTURE:
            break;

        case ResourceType::MODEL:
            importer = new ModelImporter();
            break;
    }

    bool success = false;
    if (importer != nullptr) success = importer->Import(assetPath, libraryPath, uid);

    delete importer;

    if (success)
    {
        if (resources.find(uid) == resources.end())
        {
            Resource* res = CreateNewResource(assetPath, libraryPath, type, uid);
            if (res) resources[uid] = res;
        }

        LOG("Imported successfully: %s -> Library: %s", assetPath.c_str(), libraryPath.c_str());
        return uid;
    }

    return 0;
}

Resource* Resources::RequestResource(uint32_t uid)
{
    // Buscar en el mapa
    auto it = resources.find(uid);

    if (it != resources.end())
    {
        Resource* res = it->second;

        // ¡CLAVE! Llamamos a LoadToMemory
        // Esto incrementa el contador y carga los datos si es la primera vez
        res->LoadToMemory();

        return res;
    }

    return nullptr;
}

void Resources::ReleaseResource(uint32_t uid)
{
    auto it = resources.find(uid);

    if (it != resources.end())
    {
        Resource* res = it->second;

        // ¡CLAVE! Decrementa contador y descarga si llega a 0
        res->UnloadFromMemory();
    }
}

Resource* Resources::CreateNewResource(const std::string& assetsPath, const std::string& libraryPath, ResourceType type, uint32_t uid)
{
    Resource* ret = nullptr;

    switch (type) {
    case ResourceType::TEXTURE:
        // ret = new ResourceTexture(uid);
        break;
    case ResourceType::MODEL:
        // ret = new ResourceMesh(uid); 
        break;
    }

    if (ret)
    {
        ret->SetAssetFile(assetsPath);
        ret->SetLibraryFile(libraryPath);
    }

    return ret;
}

uint32_t Resources::GenerateNewUID()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
    return dis(gen);
}

ResourceType Resources::GetTypeFromExtension(const std::string& path)
{
    std::string ext = GetFileExtension(path); // Tu función helper existente

    if (ext == "png" || ext == "jpg" || ext == "dds" || ext == "tga") return ResourceType::TEXTURE;
    if (ext == "fbx" || ext == "obj") return ResourceType::MODEL; // O MESH

    return ResourceType::UNKNOWN;
}

uint32_t Resources::ReadUIDFromMeta(const std::string& metaPath)
{
    std::ifstream file(metaPath, std::ios::binary);

    if (!file.is_open())
    {
        return 0;
    }

    uint32_t uid = 0;

    file.read(reinterpret_cast<char*>(&uid), sizeof(uint32_t));
    file.close();

    return uid;
}

std::string Resources::GenerateLibraryPath(uint32_t uid)
{
    std::string uidStr = std::to_string(uid);

    std::string folder = (uidStr.length() >= 2) ? uidStr.substr(0, 2) : "00";

    std::string directoryPath = "Library/" + folder;

    CreateDirectory(directoryPath);

    return directoryPath + "/" + uidStr + ".bin";
}