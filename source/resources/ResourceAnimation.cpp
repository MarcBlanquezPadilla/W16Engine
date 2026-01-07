#include "ResourceAnimation.h"
#include "../utils/Log.h"
#include <fstream>

ResourceAnimation::ResourceAnimation(UID uid) : Resource(uid, Resource::Type::animation)
{
}

ResourceAnimation::~ResourceAnimation()
{
    UnloadFromMemory_Internal();
}

bool ResourceAnimation::LoadToMemory_Internal()
{
    std::ifstream file(libraryPath, std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        LOG("Error: Could not open animation library file: %s", libraryPath.c_str());
        return false;
    }

    // 1. CABECERA (Igual que antes)
    file.seekg(sizeof(UID) + sizeof(int)); // Ajusta si UID es uint64_t

    // 2. GLOBALES (Igual que antes)
    file.read((char*)&duration, sizeof(double));
    file.read((char*)&ticksPerSecond, sizeof(double));

    // 3. CANALES
    uint32_t numChannels = 0;
    file.read((char*)&numChannels, sizeof(uint32_t));

    channels.clear(); // Buena práctica limpiar antes
    channels.reserve(numChannels);

    for (uint32_t i = 0; i < numChannels; i++)
    {
        Channel channel;

        // A. Nombre (Igual)
        uint32_t nameSize = 0;
        file.read((char*)&nameSize, sizeof(uint32_t));

        if (nameSize > 0)
        {
            channel.name.resize(nameSize);
            file.read(&channel.name[0], nameSize);
        }

        // B. Tamaños (Igual)
        uint32_t numPos = 0;
        uint32_t numRot = 0;
        uint32_t numScl = 0;

        file.read((char*)&numPos, sizeof(uint32_t));
        file.read((char*)&numRot, sizeof(uint32_t));
        file.read((char*)&numScl, sizeof(uint32_t));

        // C. LEER KEYS (¡AQUÍ ESTÁ EL CAMBIO!) 
        // Leemos bloques de glm::vec3 y glm::quat directamente.
        // Ya no leemos el 'double time' porque no existe en el archivo optimizado.

        if (numPos > 0)
        {
            channel.positionKeys.resize(numPos);
            // sizeof(glm::vec3) son 12 bytes (float x, y, z)
            file.read((char*)channel.positionKeys.data(), numPos * sizeof(glm::vec3));
        }

        if (numRot > 0)
        {
            channel.rotationKeys.resize(numRot);
            // sizeof(glm::quat) son 16 bytes (float w, x, y, z)
            file.read((char*)channel.rotationKeys.data(), numRot * sizeof(glm::quat));
        }

        if (numScl > 0)
        {
            channel.scaleKeys.resize(numScl);
            // sizeof(glm::vec3) son 12 bytes
            file.read((char*)channel.scaleKeys.data(), numScl * sizeof(glm::vec3));
        }

        channels.push_back(channel);
    }

    file.close();
    return true;
}

// LIBERAR RAM
bool ResourceAnimation::UnloadFromMemory_Internal()
{
    channels.clear();
    return true;
}