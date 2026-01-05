#pragma once

#include "../Global.h"
#include "Importer.h"
#include "../resources/ResourceAnimation.h"
#include <string>
#include <vector>

struct aiAnimation;

class ImporterAnimation
{
public:
	bool Import(const std::string libraryPath, const UID uid, const aiAnimation* animation);

	bool Save(const std::string libraryPath, const UID uid, const ResourceAnimation& animData);
};