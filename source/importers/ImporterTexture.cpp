#include "../Global.h"
#include "../utils/Log.h"
#include <string>
#include "ImporterTexture.h"
#include "../resources/Resource.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <fstream>

bool ImporterTexture::Import_Internal()
{
	unsigned int ilImageID = 0;
	ilGenImages(1, &ilImageID);
	ilBindImage(ilImageID);

	if (!ilLoadImage(assetPath.c_str()))
	{
		LOG(LogType::LOG_ERROR, "Failed loading file from %s.", assetPath.c_str());
		ilDeleteImages(1, &ilImageID);
		return false;
	}

	//if (flip) iluFlipImage();

	if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
	{
		LOG(LogType::LOG_ERROR, "Failed converting image to RGBA: %s", assetPath.c_str());
		ilDeleteImages(1, &ilImageID);
		return false;
	}

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	uint32_t width = ilGetInteger(IL_IMAGE_WIDTH);
	uint32_t height = ilGetInteger(IL_IMAGE_HEIGHT);
	uint32_t dataSize = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
	int format = ilGetInteger(IL_IMAGE_FORMAT);

    std::ofstream file(libraryPath, std::ios::out | std::ios::binary);
    if (file.is_open())
    {
		file.write((const char*)&uid, sizeof(UID));
		file.write((const char*)&type, sizeof(int));

        file.write((const char*)&width, sizeof(uint32_t));
        file.write((const char*)&height, sizeof(uint32_t));
        file.write((const char*)&format, sizeof(int));
        file.write((const char*)&dataSize, sizeof(uint32_t));

        file.write((const char*)ilGetData(), dataSize);

        file.close();
        LOG(LogType::LOG_INFO, "Texture saved to Library: %s (%dx%d)", libraryPath.c_str(), width, height);
    }
    else
    {
        LOG(LogType::LOG_ERROR, "Error importing texture binary: %s", libraryPath.c_str());
        ilDeleteImages(1, &ilImageID);
        return false;
    }
	
	SaveMeta();

    ilDeleteImages(1, &ilImageID);

	return true;
}