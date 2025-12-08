#pragma once

#include "../Global.h"
#include "Importer.h"
#include <string>


class ImporterTexture : public Importer
{
public:
	virtual bool Import_Internal() override;
};