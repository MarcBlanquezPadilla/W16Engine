#pragma once

#include "../Global.h"
#include "ResourceUser.h"
#include <string>
#include <vector>

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
		animation,
		unknown
	};

public:
	Resource(UID uid, Resource::Type type);
	virtual ~Resource();
	Resource::Type GetType() const { return type; };
	UID GetUID() const { return uid; };
	const char* GetName() const { return name.c_str(); };
	const char* GetAssetFile() const { return assetPath.c_str(); };
	const char* GetLibraryFile() const { return libraryPath.c_str(); };
	unsigned int GetReferenceCount() const { return referenceCount; };
	bool IsLoadedToMemory()  const { return referenceCount > 0; }
	bool LoadToMemory();
	bool UnloadFromMemory();

	bool IsInteralResource()  const { return internalResource; };

	//USERS
	void AddReference(ResourceUser* user);
	void RemoveReference(ResourceUser* user);
	void NotifyUsers(UID newUID = 0);

protected:

	virtual bool LoadToMemory_Internal() = 0;
	virtual bool UnloadFromMemory_Internal() = 0;

	UID uid = 0;
	std::string name;
	std::string assetPath;
	std::string libraryPath;
	Type type = unknown;
	unsigned int referenceCount = 0;

	bool internalResource = false;

	std::vector<UID> childs;
	std::vector<ResourceUser*> users;
};