#pragma once

#include "../Global.h"
#include "Importer.h"
#include <string>

class aiMesh;

class ImporterMesh
{
public:
	virtual bool Import(const std::string libraryPath, const UID uid, const int type, aiMesh* mesh);
};