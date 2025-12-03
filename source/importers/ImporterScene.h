#pragma once

#include "../Global.h"
#include "Importer.h"
#include <string>

class ImporterScene : public Importer
{
public:
	virtual bool Import(const std::string assetPath, const std::string libraryPath, const UID uid, const int type) override;
};