#pragma once

#include "../Global.h"
#include <string>

class Config;

class Resource
{
public:
	enum Type {
		texture,
		mesh,
		audio,
		scene,
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

	void SaveBasicData(Config& config);
	virtual void Save(Config& config) const {};
	virtual void Load(const Config& config) {};
	virtual bool LoadInMemory() = 0;

protected:
	UID uid = 0;
	std::string assetPath;
	std::string libraryPath;
	Type type = unknown;
	unsigned int referenceCount = 0;
};