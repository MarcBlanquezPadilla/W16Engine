#pragma once
#include "Component.h"
#include "../resources/ResourceAnimation.h"
#include <map>
#include <string>
#include <vector> // Importante para std::vector

class GameObject;
class Transform;

// ESTRUCTURA DE CACHÉ OPTIMIZADA
struct AnimLink {
    const Channel* channel;
    Transform* transform;
};

class Animation : public Component
{
public:
    Animation(GameObject* owner);
    virtual ~Animation();

    void Update() override;

    ComponentType GetType() override { return ComponentType::Animation; };

    void SetAnimation(UID animUID);
    void Play();
    void Stop();
    void Pause();

    void OnEditor() override;

private:
    void RebuildAnimCache();
    void InvalidateBoneMap();

    // GETTERS CORREGIDOS: Reciben 'int&' (referencia al índice específico)
    glm::vec3 GetPositionValue(const Channel& channel, float currentAnimTime);
    glm::quat GetRotationValue(const Channel& channel, float currentAnimTime);
    glm::vec3 GetScaleValue(const Channel& channel, float currentAnimTime);

    void UpdateTransformations(const ResourceAnimation* animation, float currentAnimTime);

public:
    UID animationUID = 0;
    ResourceAnimation* currentAnimation = nullptr;

    bool loop = true;
    bool playing = false;
    float speed = 1.0f;

    float currentTime = 0.0f;

private:
    std::map<std::string, GameObject*> boneMap; // Solo se usa al cargar
    std::vector<AnimLink> animCache;            // Se usa en cada frame (Rápido)

    bool debugDraw = false;
};