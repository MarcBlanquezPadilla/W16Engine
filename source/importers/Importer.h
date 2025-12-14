#pragma once
#include "../Global.h"
#include <string>
#include <list>

class Config;

class Importer
{

public:
    Importer() {};

    virtual ~Importer() {};
    bool Import(const std::string assetPath, const std::string libraryPath, const UID uid, const int type);

protected:

    virtual bool Import_Internal() = 0;
    virtual bool SaveMeta();
	void SaveBasicMeta(Config& config);

protected: 

    UID uid;
    int type;
    std::string assetPath;
    std::string libraryPath;
};