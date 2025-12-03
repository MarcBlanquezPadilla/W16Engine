#pragma once

#include "../Global.h"
#include <string>

class Importer
{
public:
	virtual bool Import(const std::string assetPath, const std::string libraryPath, const UID uid, const int type) = 0;
};