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

	// Si ticksPerSecond es 0 (sucede en formatos viejos), ponemos 24 o 30 por defecto
	if (animData.ticksPerSecond == 0) animData.ticksPerSecond = 24.0;

	// 2. RELLENAR CANALES (HUESOS)
	for (unsigned int i = 0; i < assimpAnim->mNumChannels; i++)
	{
		aiNodeAnim* aiChannel = assimpAnim->mChannels[i];
		Channel ourChannel;

		ourChannel.name = aiChannel->mNodeName.C_Str();

		// A. Posiciones
		for (unsigned int k = 0; k < aiChannel->mNumPositionKeys; k++)
		{
			aiVectorKey aiKey = aiChannel->mPositionKeys[k];
			AnimationKey<glm::vec3> key;
			key.time = aiKey.mTime;
			key.value = glm::vec3(aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z);
			ourChannel.positionKeys.push_back(key);
		}

		// B. Rotaciones
		for (unsigned int k = 0; k < aiChannel->mNumRotationKeys; k++)
		{
			aiQuatKey aiKey = aiChannel->mRotationKeys[k];
			AnimationKey<glm::quat> key;
			key.time = aiKey.mTime;

			key.value = glm::quat(aiKey.mValue.w, aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z);

			ourChannel.rotationKeys.push_back(key);
		}

		// C. Escalas
		for (unsigned int k = 0; k < aiChannel->mNumScalingKeys; k++)
		{
			aiVectorKey aiKey = aiChannel->mScalingKeys[k];
			AnimationKey<glm::vec3> key;
			key.time = aiKey.mTime;
			key.value = glm::vec3(aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z);
			ourChannel.scaleKeys.push_back(key);
		}

		animData.channels.push_back(ourChannel);
	}

	// 3. GUARDAR A BINARIO
	return Save(libraryPath, uid, animData);
}

bool ImporterAnimation::Save(const std::string libraryPath, const UID uid, const ResourceAnimation& animData)
{
	std::ofstream file(libraryPath, std::ios::out | std::ios::binary);
	if (file.is_open())
	{
		// A. HEADER (UID + TIPO)
		int type = (int)Resource::Type::animation; // Asegúrate de tener este tipo en tu enum
		file.write((const char*)&uid, sizeof(UID));
		file.write((const char*)&type, sizeof(int));

		// B. DATOS GLOBALES
		file.write((const char*)&animData.duration, sizeof(double));
		file.write((const char*)&animData.ticksPerSecond, sizeof(double));

		// C. CANALES
		uint32_t numChannels = animData.channels.size();
		file.write((const char*)&numChannels, sizeof(uint32_t));

		for (const auto& channel : animData.channels)
		{
			// 1. Nombre del hueso (Tamaño + Caracteres)
			uint32_t nameSize = channel.name.size();
			file.write((const char*)&nameSize, sizeof(uint32_t));
			file.write(channel.name.c_str(), nameSize);

			// 2. Tamaños de las listas de keys
			uint32_t numPos = channel.positionKeys.size();
			uint32_t numRot = channel.rotationKeys.size();
			uint32_t numScl = channel.scaleKeys.size();

			file.write((const char*)&numPos, sizeof(uint32_t));
			file.write((const char*)&numRot, sizeof(uint32_t));
			file.write((const char*)&numScl, sizeof(uint32_t));

			// 3. Escribir los vectores de Keys de golpe (Optimización de memoria)
			// Al ser structs simples (double + float3), podemos escribirlos como bloque de bytes
			if (numPos > 0)
				file.write((const char*)channel.positionKeys.data(), numPos * sizeof(AnimationKey<glm::vec3>));

			if (numRot > 0)
				file.write((const char*)channel.rotationKeys.data(), numRot * sizeof(AnimationKey<glm::quat>));

			if (numScl > 0)
				file.write((const char*)channel.scaleKeys.data(), numScl * sizeof(AnimationKey<glm::vec3>));
		}

		file.close();
		LOG("Animation imported to Library: %s", libraryPath.c_str());
		return true;
	}

	LOG("Error saving animation to library: %s", libraryPath.c_str());
	return false;
}