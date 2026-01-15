#include "../Global.h"
#include "../Engine.h"
#include "../ModuleResources.h"

#include "ImporterModel.h"
#include "ImporterMesh.h"
#include "ImporterTexture.h"
#include "ImporterAnimation.h"

#include "../GameObject.h"
#include "../components/Transform.h"
#include "../components/Mesh.h"
#include "../components/Texture.h"

#include "../resources/Resource.h"

#include "../utils/Log.h"
#include "../utils/FileUtils.h"
#include "../utils/Config.h"

#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <sstream>

ImporterModel::~ImporterModel()
{

}

bool ImporterModel::Import_Internal()
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(assetPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GlobalScale);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG(LogType::LOG_ERROR, "Failed loading model with Assimp: %s", importer.GetErrorString());
		return false;
	}
	
	referedUIDs.clear();
	UIDsByName.clear();
	if (DoesFileHasMeta(assetPath))
	{
		Config modelConfig;
		if (modelConfig.Load(GetMetaPath(assetPath).c_str()))
		{					
			if (modelConfig.IsValid() && modelConfig.GetUInt("ReferedObjects") > 0)
			{
				Config childNode = modelConfig.GetChild("ReferedObject");
				while (childNode.IsValid())
				{
					std::string name = childNode.GetString("Name");
					UID uid = childNode.GetUInt("UID");

					if (name != "" && uid != 0)
					{
						UIDsByName.emplace(name, uid);
					}
					childNode = childNode.GetNextSibling("ReferedObject");
				}
			}
		}
	}

	if (scene->HasAnimations())
	{
		for (unsigned int i = 0; i < scene->mNumAnimations; i++)
		{
			ImporterAnimation* animImporter = new ImporterAnimation();
			aiAnimation* assimpAnim = scene->mAnimations[i];

			// 1. Obtener nombre y buscar UID persistente
			std::string animName = assimpAnim->mName.C_Str();
			if (animName.empty()) animName = "Animation_" + std::to_string(i);

			UID animUID = 0;

			// Si ya existe en el meta (reimportación), mantenemos el UID
			if (UIDsByName.find(animName) != UIDsByName.end())
			{
				animUID = UIDsByName[animName];
			}
			else
			{
				animUID = GenerateNewUID();
			}

			// 2. Importar la animación a la librería
			// Asegúrate de que tu función GetLibraryPath funciona con Resource::Type::animation
			// O construye la ruta manualmente si es necesario (ej: "Library/Animations/" + uid)
			std::string libPath = GetLibraryPath(animUID);

			bool success = animImporter->Import(libPath, animUID, assimpAnim);

			if (success)
			{
				// 3. Añadir a la lista de referencias para que salga en el .meta
				ReferedsData importData;
				importData.name = animName;
				importData.type = Resource::Type::animation; // Asegúrate de tener este enum

				referedUIDs.emplace(animUID, importData);

				LOG(LogType::LOG_INFO, "Animation '%s' imported successfully.", animName.c_str());
			}
			else
			{
				LOG(LogType::LOG_ERROR, "Failed to import animation '%s'.", animName.c_str());
			}
			delete animImporter;
		}

		
	}

	GameObject* modelGameObject = new GameObject(true, GetFileName(assetPath));
	modelGameObject->UUID = 0;
	if (!modelGameObject || !ProcessNode(scene->mRootNode, scene, modelGameObject))
	{
		LOG(LogType::LOG_ERROR, "Failed to process root node for model: %s", assetPath.c_str());
		return false;
	}

	//SAVE LIBRARY
	Config gameObjectConfig;
	Config gameObjectNode = gameObjectConfig.AddChild("GameObject");
	modelGameObject->Save(gameObjectNode);

	std::string xmlBuffer;
	gameObjectConfig.SaveToString(xmlBuffer);

	std::ofstream file(libraryPath, std::ios::out | std::ios::binary);
	if (file.is_open())
	{
		file.write((const char*)&uid, sizeof(UID));
		file.write((const char*)&type, sizeof(int));

		uint32_t size = (uint32_t)xmlBuffer.size();
		file.write((const char*)&size, sizeof(uint32_t));
		file.write(xmlBuffer.c_str(), size);
		file.close();
		LOG(LogType::LOG_INFO, "Model %s imported to Library: %s", GetFileName(assetPath).c_str(), libraryPath.c_str());
	}


	//SAVE META
	SaveMeta();

	referedUIDs.clear();
	UIDsByName.clear();
	if (modelGameObject) 
	{
		modelGameObject->CleanUpRecursive();
		delete modelGameObject;
		modelGameObject = nullptr;
	}

	return true;
}

bool ImporterModel::ProcessNode(aiNode* node, const aiScene* scene, GameObject* targetGameObject)
{
	//APPLY NODE TRANSFORMS
	aiVector3D position;
	aiQuaternion rotation;
	aiVector3D scaling;
	node->mTransformation.Decompose(scaling, rotation, position);

	targetGameObject->transform->SetLocalPosition(glm::vec3(position.x, position.y, position.z));
	targetGameObject->transform->SetLocalQuaternionRotation(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
	targetGameObject->transform->SetLocalScale(glm::vec3(scaling.x, scaling.y, scaling.z));

	//PROCESS MESHES
	if (node->mNumMeshes == 1)
	{
		aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[0]];

		if (!AddMeshAndTexture(assimpMesh, scene, targetGameObject))
		{
			LOG(LogType::LOG_ERROR, "Failed processing mesh for node %s. Node will be empty.", node->mName.C_Str());
		}
	}
	else if (node->mNumMeshes > 0)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];
			GameObject* meshGameObject = new GameObject(true, assimpMesh->mName.C_Str());
			meshGameObject->UUID = 0;

			if (meshGameObject && AddMeshAndTexture(assimpMesh, scene, meshGameObject))
			{
				targetGameObject->AddChild(meshGameObject);
			}
			else
			{
				LOG(LogType::LOG_ERROR, "Failed processing mesh %s, skipping.", assimpMesh->mName.C_Str());
				meshGameObject->CleanUpRecursive();
				delete meshGameObject;
			}
		}
	}

	//RECURSIVE CHILDS CREATION
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		GameObject* childNodeGO = new GameObject(true, node->mChildren[i]->mName.C_Str());
		childNodeGO->UUID = 0;
			
		if (ProcessNode(node->mChildren[i], scene, childNodeGO))
		{
			targetGameObject->AddChild(childNodeGO);
		}
		else
		{
			childNodeGO->CleanUpRecursive();
			delete childNodeGO;
		}
	}

	return true;
}

bool ImporterModel::AddMeshAndTexture(aiMesh* assimpMesh, const aiScene* scene, GameObject* target)
{
	if (target)
	{
		//ADD MESH
		LoadMesh(assimpMesh, target);

		//ADD TEXTURE
		if (scene->HasMaterials())
		{
			aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
			LoadTexture(material, scene, target);
		}
		return true;
	}
	return false;
}

bool ImporterModel::LoadMesh(aiMesh* assimpMesh, GameObject* target)
{
	Mesh* meshComp = (Mesh*)target->AddComponent(ComponentType::Mesh);
	if (!meshComp) return false;

	//GET UID
	UID meshUID = 0;
	if (UIDsByName.find(target->name) != UIDsByName.end())
	{
		meshUID = UIDsByName[target->name];
	}
	else meshUID = GenerateNewUID();

	ImporterMesh* importer = new ImporterMesh();

	bool success = importer->Import(GetLibraryPath(meshUID), meshUID, Resource::Type::mesh, assimpMesh);

	delete importer;

	if (success)
	{
		meshComp->SetResource(meshUID);
		ReferedsData importMeshData;
		importMeshData.name = target->name;
		importMeshData.type = Resource::Type::mesh;
		referedUIDs.emplace(meshUID,importMeshData);
		return true;
	}
	else
	{
		LOG(LogType::LOG_ERROR, "Failed loading mesh data for %s.", target->name.c_str());
		return false;
	}
}

bool ImporterModel::LoadTexture(aiMaterial* material, const aiScene* scene, GameObject* obj)
{
	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString aiPath;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);

		std::string texPath = "";
		std::string fileName = "";
		bool foundFile = false;

		// 1. INTENTAR CARGAR COMO TEXTURA EMBEBIDA (EMBEDDED)
		// Esta función comprueba si el path apunta a una textura dentro de la escena
		// Funciona tanto para "*0" como para "nombre_archivo.png"
		const aiTexture* aiTex = scene->GetEmbeddedTexture(aiPath.C_Str());

		if (aiTex != nullptr)
		{
			LOG(LogType::LOG_INFO, "Texture found embedded in the model: %s", aiPath.C_Str());

			// Procesar textura embebida
			if (aiTex->mHeight == 0) // Comprimida (jpg/png)
			{
				// Decidir extensión
				std::string extension = "";
				if (aiTex->achFormatHint[0])
					extension = std::string(aiTex->achFormatHint);
				if (extension.empty())
					extension = "png"; // Fallback seguro

				// Crear nombre único. Usamos el aiPath para generar un nombre o un índice si es posible.
				// Nota: GetFileName limpia la ruta y deja solo el nombre
				std::string rawName = GetFileNameNoExtension(aiPath.C_Str());
				if (rawName.empty() || rawName == "*") rawName = "embedded_tex";

				// Sugerencia: Añadir el nombre del modelo para evitar colisiones
				std::string modelName = GetFileNameNoExtension(assetPath);
				fileName = modelName + "_" + rawName + "." + extension;

				std::string modelDir = GetDirectoryFromPath(assetPath);
				texPath = modelDir + fileName;

				// Escribir a disco si no existe
				if (!DoesFileExist(texPath))
				{
					std::ofstream file(texPath, std::ios::binary);
					if (file.is_open())
					{
						file.write((char*)aiTex->pcData, aiTex->mWidth);
						file.close();
						LOG(LogType::LOG_INFO, "Extracted embedded texture to: %s", texPath.c_str());
					}
				}
				foundFile = true;
			}
			else
			{
				LOG(LogType::LOG_WARNING, "Embedded texture is raw ARGB(not supported yet).");
			}
		}
		// 2. SI NO ES EMBEBIDA, BUSCAR EN DISCO (Lógica original)
		else
		{
			LOG(LogType::LOG_INFO, "Texture not embedded, searching in disk: %s", aiPath.C_Str());

			std::string modelDirectory = GetDirectoryFromPath(assetPath);
			fileName = GetFileName(aiPath.C_Str());

			// Opción A: Ruta tal cual viene
			texPath = modelDirectory + aiPath.C_Str();
			if (DoesFileExist(texPath)) foundFile = true;

			// Opción B: En la misma carpeta del modelo
			if (!foundFile) {
				texPath = modelDirectory + fileName;
				if (DoesFileExist(texPath)) foundFile = true;
			}

			// Opción C: Búsqueda recursiva (costosa, úsala con cuidado)
			if (!foundFile) {
				texPath = FindFileInDirectory(modelDirectory, fileName);
				if (DoesFileExist(texPath)) foundFile = true;
			}
		}

		// 3. CARGAR EN EL MOTOR
		if (foundFile)
		{
			Texture* texture = (Texture*)obj->AddComponent(ComponentType::Texture);
			if (texture)
			{
				// Asegurar importación (meta data, etc)
				Engine::GetInstance().moduleResources->CheckFileLoaded(texPath);
				Engine::GetInstance().moduleResources->PublishAssetChangedEvent();

				UID textureUID = Engine::GetInstance().moduleResources->Find(texPath);
				if (textureUID != 0)
				{
					texture->SetResource(textureUID);
					return true; // Éxito
				}
				else
				{
					LOG(LogType::LOG_ERROR, "Failed loading texture resource % s", texPath.c_str());
				}
			}
		}
		else
		{
			LOG(LogType::LOG_ERROR, "Could not find texture '%s' (Embedded check failed & Disk check failed)", aiPath.C_Str());
		}
	}

	return false;
}

bool ImporterModel::SaveMeta()
{
	Config meta;

	SaveBasicMeta(meta);

	meta.SetUInt("ReferedObjects", referedUIDs.size());
	for (auto pair : referedUIDs)
	{
		Config refered = meta.AddChild("ReferedObject");
		refered.SetUInt("UID", pair.first);
		refered.SetString("Name", pair.second.name.c_str());
		refered.SetInt("Type", pair.second.type);
	}

	return meta.Save(GetMetaPath(assetPath).c_str());
}