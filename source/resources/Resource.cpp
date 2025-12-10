#include "Resource.h"
#include "../utils/Config.h"

Resource::Resource(UID uid, Resource::Type type) : uid(uid), type(type)
{

}

Resource::~Resource()
{

}

bool Resource::LoadToMemory()
{
    if (!IsLoadedToMemory())
    {
        if (!LoadToMemory_Internal())
        {
            return false;
        }
    }

    referenceCount++;
    return true;
}

bool Resource::UnloadFromMemory()
{
    if (IsLoadedToMemory())
    {
        referenceCount--;

        if (referenceCount == 0)
        {
            UnloadFromMemory_Internal();
        }
        return true;
    }

    return false;
}