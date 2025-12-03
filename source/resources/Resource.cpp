#include "Resource.h"
#include "../utils/Config.h"

Resource::Resource(UID uid, Resource::Type type)
{

}

Resource::~Resource()
{

}

bool Resource::LoadInMemory()
{
	return true;
}

void Resource::SaveBasicData(Config& config)
{
	config.SetUInt("UID", uid);
	config.SetInt("type", type);
	config.SetString("assetPath", assetPath);
	config.SetString("libraryPath", libraryPath);
}
