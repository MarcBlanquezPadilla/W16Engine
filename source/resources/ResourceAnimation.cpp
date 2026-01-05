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
    // libraryPath viene de la clase padre Resource
    std::ifstream file(libraryPath, std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        LOG("Error: Could not open animation library file: %s", libraryPath.c_str());
        return false;
    }

    // 1. LEER CABECERA (Saltamos UID y Tipo porque ya los sabemos, pero hay que avanzar el cursor)
    // El Importer guardó: UID (4 u 8 bytes) + Type (4 bytes)
    // Si tu UID es 'unsigned int' son 4 bytes, si es 'unsigned long long' son 8. 
    // Asumiré que UID es 4 bytes (uint32) por seguridad, ajusta si es uint64.
    file.seekg(sizeof(UID) + sizeof(int));

    // 2. LEER DATOS GLOBALES
    file.read((char*)&duration, sizeof(double));
    file.read((char*)&ticksPerSecond, sizeof(double));

    // 3. LEER CANALES
    uint32_t numChannels = 0;
    file.read((char*)&numChannels, sizeof(uint32_t));

    channels.reserve(numChannels);

    for (uint32_t i = 0; i < numChannels; i++)
    {
        Channel channel;

        // A. Nombre
        uint32_t nameSize = 0;
        file.read((char*)&nameSize, sizeof(uint32_t));

        if (nameSize > 0)
        {
            channel.name.resize(nameSize);
            file.read(&channel.name[0], nameSize);
        }

        // B. Tamaños de keys
        uint32_t numPos = 0;
        uint32_t numRot = 0;
        uint32_t numScl = 0;

        file.read((char*)&numPos, sizeof(uint32_t));
        file.read((char*)&numRot, sizeof(uint32_t));
        file.read((char*)&numScl, sizeof(uint32_t));

        // C. Leer Keys (Lectura en bloque rapida)
        if (numPos > 0)
        {
            channel.positionKeys.resize(numPos);
            file.read((char*)channel.positionKeys.data(), numPos * sizeof(AnimationKey<glm::vec3>));
        }

        if (numRot > 0)
        {
            channel.rotationKeys.resize(numRot);
            file.read((char*)channel.rotationKeys.data(), numRot * sizeof(AnimationKey<glm::quat>));
        }

        if (numScl > 0)
        {
            channel.scaleKeys.resize(numScl);
            file.read((char*)channel.scaleKeys.data(), numScl * sizeof(AnimationKey<glm::vec3>));
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