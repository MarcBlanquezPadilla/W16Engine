#include "ImporterScene.h"
#include "../utils/Log.h"
#include "../ModuleResources.h" 
#include <fstream>
#include <sstream>

bool ImporterScene::Import(const std::string assetPath, const std::string libraryPath, const UID uid, const int type)
{
    std::ifstream importFile(assetPath);
    if (!importFile.is_open())
    {
        LOG("Error loading scene: %s", assetPath.c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << importFile.rdbuf();
    std::string xmlContent = buffer.str();
    importFile.close();

    std::ofstream file(libraryPath, std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        file.write((const char*)&uid, sizeof(UID));
        file.write((const char*)&type, sizeof(int));

        uint32_t size = (uint32_t)xmlContent.size();
        file.write((const char*)&size, sizeof(uint32_t));

        file.write(xmlContent.c_str(), size);

        file.close();
        LOG("Scene imported to Library: %s", libraryPath.c_str());
    }
    else
    {
        LOG("Error importing scene binary: %s", libraryPath.c_str());
        return false;
    }

    return true;
}