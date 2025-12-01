#pragma once
#include "Module.h"
#include "resources/Resource.h"
#include <map>
#include <string>

class Resources : public Module
{
public:
    Resources(bool startEnabled);
    virtual ~Resources();

    bool Awake();
    bool CleanUp();


    uint32_t LoadFile(const std::string& assetsPath);
    Resource* RequestResource(uint32_t uid);

    void ReleaseResource(uint32_t uid);

    uint32_t GenerateNewUID();

private:
    //IMPORT
    uint32_t ImportFile(const std::string& assetsPath, uint32_t uuid);

    //META
    uint32_t ReadUIDFromMeta(const std::string& metaPath);

    //LIBRARIES
    Resource* CreateNewResource(const std::string& assetsPath, const std::string& libraryPath, ResourceType type, uint32_t uid = 0);

    ResourceType GetTypeFromExtension(const std::string& path);


    std::string GenerateLibraryPath(uint32_t uid);

private:

    std::map<uint32_t, Resource*> resources;
    std::map<uint32_t, std::string> resourcesPath;
};