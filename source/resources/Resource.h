#pragma once
#include <string>
#include "../Global.h"

enum class ResourceType 
{
    UNKNOWN,
    TEXTURE,
    MESH,
    MODEL,
    SCENE
};

class Resource
{
    friend class ModuleResources;

public:
    Resource(uint32_t uid, ResourceType type);
    virtual ~Resource();

    ResourceType GetType() const { return type; }
    uint32_t GetUID() const { return uid; }
    const char* GetAssetsFile() const { return assetsFile.c_str(); }
    const char* GetLibraryFile() const { return libraryFile.c_str(); }
    void SetAssetFile(std::string assetPath) { assetsFile = assetPath; };
    void SetLibraryFile(std::string libraryPath) { libraryFile = libraryPath; }

    bool IsLoadedToMemory() const { return referenceCount > 0; }

    void LoadToMemory();
    void UnloadFromMemory();

    virtual bool LoadInMemory() = 0;
    virtual bool UnloadFromMemory_Internal() = 0;

protected:
    uint32_t uid = 0;
    std::string assetsFile;
    std::string libraryFile;
    ResourceType type = ResourceType::UNKNOWN;

    unsigned int referenceCount = 0;
};