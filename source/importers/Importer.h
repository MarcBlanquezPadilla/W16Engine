#pragma once
#include "../Global.h"
#include <string>
#include <list>

class Config;

class Importer
{
public:
	virtual bool Import(const std::string assetPath, const std::string libraryPath, const UID uid, const int type) = 0;

	virtual bool SaveMeta(const std::string assetPath, const UID uid, const int type, const std::list<UID> referedIDs);

protected:
	void SaveBasicMeta(Config& config, const UID uid, const int type, const std::list<UID> referedIDs = {});

};