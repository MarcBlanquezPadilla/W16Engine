#pragma once

#include "../Global.h"
#include <string>

class Config;
class ModuleResources;

class Resource
{
	friend class ModuleResources;

public:
	enum Type {
		texture,
		model,
		mesh,
		scene,
		audio,
		bone,
		animation,
		unknown
	};

public:
	Resource(UID uid, Resource::Type type);
	virtual ~Resource();
	Resource::Type GetType() const { return type; };
	UID GetUID() const { return uid; };
	const char* GetAssetFile() const { return assetPath.c_str(); };
	const char* GetLibraryFile() const { return libraryPath.c_str(); };
	unsigned int GetReferenceCount() const { return referenceCount; };
	bool IsLoadedToMemory()  const { return referenceCount > 0; }
	bool LoadToMemory();
	bool UnloadFromMemory();


protected:

	virtual bool LoadToMemory_Internal() = 0;
	virtual bool UnloadFromMemory_Internal() = 0;

	UID uid = 0;
	std::string assetPath;
	std::string libraryPath;
	Type type = unknown;
	unsigned int referenceCount = 0;
};