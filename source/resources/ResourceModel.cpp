#include "../utils/Log.h"
#include <fstream>
#include "ResourceModel.h"

ResourceModel::ResourceModel(UID uid) : Resource(uid, Resource::Type::model)
{

}

ResourceModel::~ResourceModel()
{
    if (IsLoadedToMemory())
        UnloadFromMemory_Internal();
}

bool ResourceModel::LoadToMemory_Internal()
{
    std::ifstream file(libraryPath, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        LOG(LogType::LOG_ERROR, "Could not open scene library file: %s", libraryPath.c_str());
        return false;
    }

    UID fileUID;
    int fileType;

    file.read(reinterpret_cast<char*>(&fileUID), sizeof(UID));
    file.read(reinterpret_cast<char*>(&fileType), sizeof(int));

    if (fileUID != this->uid)
    {
        LOG(LogType::LOG_ERROR, "UID Mismatch in file %s. Expected %u, got %u", libraryPath.c_str(), this->uid, fileUID);
        file.close();
        return false;
    }

    if (fileType != Resource::Type::model)
    {
        LOG(LogType::LOG_ERROR, "Resource Type Mismatch! %s is not a MODEL", libraryPath.c_str());
        file.close();
        return false;
    }

    uint32_t size = 0;
    file.read((char*)&size, sizeof(uint32_t));

    if (size > 0)
    {
        char* buffer = new char[size + 1];
        file.read(buffer, size);
        buffer[size] = '\0';

        file.close();

        bool parsed = gameObjectConfig.LoadFromBuffer(buffer, size);

        delete[] buffer;

        if (parsed)
        {
            LOG(LogType::LOG_INFO, "Model loaded in memory.");
            return true;
        }
    }

    file.close();
    return false;
}

bool ResourceModel::UnloadFromMemory_Internal()
{
    gameObjectConfig.CleanUp();
    return true;
}