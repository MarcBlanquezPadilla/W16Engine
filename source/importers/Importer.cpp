#pragma once
#include "../Global.h"
#include "../utils/Config.h"
#include "../utils/FileUtils.h"
#include "Importer.h"
#include <string>


bool Importer::SaveMeta(const std::string assetPath, const UID uid, const int type, const std::list<UID> referedIDs)
{
	Config meta;

	SaveBasicMeta(meta, uid, type, referedIDs);

	return meta.Save(GetMetaPath(assetPath).c_str());
}


void Importer::SaveBasicMeta(Config& config, const UID uid, const int type, const std::list<UID> referedIDs)
{
	config.SetUInt("UID", uid);
	config.SetUInt("Type", type);
	config.SetUInt("ReferedObjects", referedIDs.size());
	for (UID referedUID : referedIDs)
	{
		config.AddChild("ReferedObject").SetUInt("UID", referedUID);
	}
}