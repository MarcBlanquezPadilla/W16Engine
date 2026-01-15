#include "ImporterAnimation.h"
#include "../Global.h"
#include "../utils/Log.h"
#include "../resources/Resource.h"
#include "../resources/ResourceAnimation.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <assimp/anim.h>
#include <fstream>

bool ImporterAnimation::Import(const std::string libraryPath, const UID uid, const aiAnimation* assimpAnim)
{
	ResourceAnimation animData(uid);

	animData.duration = assimpAnim->mDuration;
	animData.ticksPerSecond = assimpAnim->mTicksPerSecond;

	if (animData.ticksPerSecond == 0) animData.ticksPerSecond = 24.0;

	if (assimpAnim->mChannels[0]->mNumPositionKeys < (unsigned int)animData.duration) {
	LOG(LogType::LOG_WARNING, "Animation %s seems NOT baked. Runtime glitches expected.", assimpAnim->mName.C_Str());
	}

	for (unsigned int i = 0; i < assimpAnim->mNumChannels; i++)
	{
		aiNodeAnim* aiChannel = assimpAnim->mChannels[i];
		Channel ourChannel;

		ourChannel.name = aiChannel->mNodeName.C_Str();

		ourChannel.positionKeys.reserve(aiChannel->mNumPositionKeys);
		for (unsigned int k = 0; k < aiChannel->mNumPositionKeys; k++)
		{
			aiVectorKey aiKey = aiChannel->mPositionKeys[k];

			ourChannel.positionKeys.push_back(glm::vec3(aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z));
		}

		ourChannel.rotationKeys.reserve(aiChannel->mNumRotationKeys);
		for (unsigned int k = 0; k < aiChannel->mNumRotationKeys; k++)
		{
			aiQuatKey aiKey = aiChannel->mRotationKeys[k];

			ourChannel.rotationKeys.push_back(glm::quat(aiKey.mValue.w, aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z));
		}

		ourChannel.scaleKeys.reserve(aiChannel->mNumScalingKeys);
		for (unsigned int k = 0; k < aiChannel->mNumScalingKeys; k++)
		{
			aiVectorKey aiKey = aiChannel->mScalingKeys[k];

			ourChannel.scaleKeys.push_back(glm::vec3(aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z));
		}

		animData.channels.push_back(ourChannel);
	}

	return Save(libraryPath, uid, animData);
}

bool ImporterAnimation::Save(const std::string libraryPath, const UID uid, const ResourceAnimation& animData)
{
	std::ofstream file(libraryPath, std::ios::out | std::ios::binary);
	if (file.is_open())
	{
		int type = (int)Resource::Type::animation;
		file.write((const char*)&uid, sizeof(UID));
		file.write((const char*)&type, sizeof(int));

		file.write((const char*)&animData.duration, sizeof(double));
		file.write((const char*)&animData.ticksPerSecond, sizeof(double));

		uint32_t numChannels = animData.channels.size();
		file.write((const char*)&numChannels, sizeof(uint32_t));

		for (const auto& channel : animData.channels)
		{
			uint32_t nameSize = channel.name.size();
			file.write((const char*)&nameSize, sizeof(uint32_t));
			file.write(channel.name.c_str(), nameSize);

			uint32_t numPos = channel.positionKeys.size();
			uint32_t numRot = channel.rotationKeys.size();
			uint32_t numScl = channel.scaleKeys.size();

			file.write((const char*)&numPos, sizeof(uint32_t));
			file.write((const char*)&numRot, sizeof(uint32_t));
			file.write((const char*)&numScl, sizeof(uint32_t));

			if (numPos > 0)
				file.write((const char*)channel.positionKeys.data(), numPos * sizeof(glm::vec3));

			if (numRot > 0)
				file.write((const char*)channel.rotationKeys.data(), numRot * sizeof(glm::quat));

			if (numScl > 0)
				file.write((const char*)channel.scaleKeys.data(), numScl * sizeof(glm::vec3));
		}

		file.close();
		LOG(LogType::LOG_INFO, "Animation imported to Library (BAKED format): %s", libraryPath.c_str());
		return true;
	}

	LOG(LogType::LOG_ERROR, "Failed saving animation to library: %s", libraryPath.c_str());
	return false;
}