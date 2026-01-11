#include "ResourceTexture.h"
#include "../Engine.h"
#include "../ModuleRender.h" 
#include "../utils/Log.h"
#include <fstream>

ResourceTexture::ResourceTexture(UID uid) : Resource(uid, Resource::Type::texture)
{

}

ResourceTexture::~ResourceTexture()
{
    UnloadFromMemory_Internal();
}

bool ResourceTexture::LoadToMemory_Internal()
{
    std::ifstream file(libraryPath, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        LOG(LogType::LOG_ERROR, "Could not open texture library file: %s", libraryPath.c_str());
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

    if (fileType != Resource::Type::texture)
    {
        LOG(LogType::LOG_ERROR, "Resource Type Mismatch! %s is not a TEXTURE", libraryPath.c_str());
        file.close();
        return false;
    }
    
    int format = 0;
    uint32_t dataSize = 0;

    file.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&format), sizeof(int));
    file.read(reinterpret_cast<char*>(&dataSize), sizeof(uint32_t));

    if (dataSize > 0)
    {
        unsigned char* data = new unsigned char[dataSize];
        file.read(reinterpret_cast<char*>(data), dataSize);

        file.close();

        gpuID = Engine::GetInstance().moduleRender->UploadTextureToGPU(data, width, height);

        delete[] data;

        if (gpuID != 0)
        {
            LOG(LogType::LOG_INFO, "Texture loaded to VRAM: %s (%dx%d)", libraryPath.c_str(), width, height);
            return true;
        }
    }
    else
    {
        file.close();
    }

    return false;
}

bool ResourceTexture::UnloadFromMemory_Internal()
{
    if (gpuID != 0)
    {
        Engine::GetInstance().moduleRender->DeleteTextureFromGPU(gpuID);
        gpuID = 0;
    }

    width = 0;
    height = 0;

    return true;
}