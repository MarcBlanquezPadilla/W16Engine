#pragma once

#include "../Global.h"
#include "../geometry/Vertex.h"
#include "../geometry/Bone.h"
#include "Importer.h"
#include <string>
#include <vector>

class aiMesh;


class ImporterMesh
{
public:
	bool Import(const std::string libraryPath, const UID uid, const int type, aiMesh* mesh);
    bool Import(const std::string libraryPath, const UID uid, const int type, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Bone>&bones);
};