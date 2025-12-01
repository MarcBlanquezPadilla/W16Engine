#include "Resource.h"
#include "../utils/Log.h"

Resource::Resource(uint32_t uid, ResourceType type)
    : uid(uid), type(type), referenceCount(0)
{
}

Resource::~Resource()
{
}

void Resource::LoadToMemory()
{
    if (referenceCount == 0)
    {
        if (LoadInMemory())
        {
            LOG("Resource %d loaded successfully", uid);
        }
        else
        {
            LOG("Error loading resource %d", uid);
        }
    }

    referenceCount++;
}

void Resource::UnloadFromMemory()
{
    referenceCount--;

    if (referenceCount == 0)
    {
        UnloadFromMemory_Internal();
        LOG("Resource %d unloaded", uid);
    }
}