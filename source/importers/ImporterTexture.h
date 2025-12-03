#pragma once

#include "../Global.h"
#include "Importer.h"
#include <string>


class ImporterTexture : public Importer
{
public:
	virtual bool Import(const std::string assetPath, const std::string libraryPath, const UID uid, const int type) override;
};