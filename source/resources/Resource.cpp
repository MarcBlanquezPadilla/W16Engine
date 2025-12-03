#include "Resource.h"
#include "../utils/Config.h"

Resource::Resource(UID uid, Resource::Type type) : uid(uid), type(type)
{

}

Resource::~Resource()
{

}

void Resource::SaveBasicData(Config& config)
{
	config.SetUInt("UID", uid);
	config.SetInt("type", type);
	config.SetString("assetPath", assetPath);
	config.SetString("libraryPath", libraryPath);
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