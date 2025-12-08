#pragma once

#include "../Global.h"
#include "Importer.h"
#include <string>

class ImporterScene : public Importer
{
public:
	virtual bool Import_Internal() override;
};