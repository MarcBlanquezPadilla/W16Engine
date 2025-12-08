#pragma once
#include "../Global.h"
#include "../utils/Config.h"
#include "../utils/FileUtils.h"
#include "Importer.h"
#include <string>

bool Importer::Import(const std::string assetPath, const std::string libraryPath, const UID uid, const int type)
{
	this->uid = uid;
	this->type = type;
	this->assetPath = assetPath;
	this->libraryPath = libraryPath;

	return Import_Internal();
}


bool Importer::SaveMeta()
{
	Config meta;

	SaveBasicMeta(meta);

	return meta.Save(GetMetaPath(assetPath).c_str());
}


void Importer::SaveBasicMeta(Config& config)
{
	config.SetUInt("UID", uid);
	config.SetInt("Type", type);
	config.SetUInt("ReferedObjects", 0);
}