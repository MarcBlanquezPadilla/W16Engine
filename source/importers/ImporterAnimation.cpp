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

	// Si ticksPerSecond es 0, ponemos 24 o 30 por defecto
	if (animData.ticksPerSecond == 0) animData.ticksPerSecond = 24.0;

	// --- SEGURIDAD DE BAKED (Opcional pero recomendada) ---
	// Si has delegado la responsabilidad al artista, aquí podrías avisar si viene mal.
	// Si dura 100 ticks y solo hay 2 keys... huele a que no está baked.
	 if (assimpAnim->mChannels[0]->mNumPositionKeys < (unsigned int)animData.duration) {
	    LOG(LogType::LOG_WARNING, "Animation %d seems NOT baked. Runtime glitches expected.", uid);
	 }

	// 2. RELLENAR CANALES (HUESOS)
	for (unsigned int i = 0; i < assimpAnim->mNumChannels; i++)
	{
		aiNodeAnim* aiChannel = assimpAnim->mChannels[i];
		Channel ourChannel;

		ourChannel.name = aiChannel->mNodeName.C_Str();

		// A. Posiciones
		// IMPORTANTE: Reservamos memoria antes para evitar copias internas
		ourChannel.positionKeys.reserve(aiChannel->mNumPositionKeys);
		for (unsigned int k = 0; k < aiChannel->mNumPositionKeys; k++)
		{
			aiVectorKey aiKey = aiChannel->mPositionKeys[k];

			// CAMBIO: Ya no usamos 'key.time'. Solo guardamos el valor.
			// Asumimos que Assimp las trae ordenadas temporalmente (casi siempre es así).
			ourChannel.positionKeys.push_back(glm::vec3(aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z));
		}

		// B. Rotaciones
		ourChannel.rotationKeys.reserve(aiChannel->mNumRotationKeys);
		for (unsigned int k = 0; k < aiChannel->mNumRotationKeys; k++)
		{
			aiQuatKey aiKey = aiChannel->mRotationKeys[k];

			// CAMBIO: Solo valor (w, x, y, z)
			ourChannel.rotationKeys.push_back(glm::quat(aiKey.mValue.w, aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z));
		}

		// C. Escalas
		ourChannel.scaleKeys.reserve(aiChannel->mNumScalingKeys);
		for (unsigned int k = 0; k < aiChannel->mNumScalingKeys; k++)
		{
			aiVectorKey aiKey = aiChannel->mScalingKeys[k];

			// CAMBIO: Solo valor
			ourChannel.scaleKeys.push_back(glm::vec3(aiKey.mValue.x, aiKey.mValue.y, aiKey.mValue.z));
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
		int type = (int)Resource::Type::animation;
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
			// 1. Nombre del hueso
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

			// 3. ESCRITURA OPTIMIZADA (Sin AnimationKey wrapper)

			// Posiciones: 12 bytes por key (float * 3)
			if (numPos > 0)
				file.write((const char*)channel.positionKeys.data(), numPos * sizeof(glm::vec3));

			// Rotaciones: 16 bytes por key (float * 4)
			if (numRot > 0)
				file.write((const char*)channel.rotationKeys.data(), numRot * sizeof(glm::quat));

			// Escalas: 12 bytes por key (float * 3)
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