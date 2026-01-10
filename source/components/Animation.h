#pragma once
#include "Component.h"
#include "../resources/ResourceAnimation.h"
#include "../EventListener.h"
#include <map>
#include <string>
#include <vector> // Importante para std::vector

class GameObject;
class Transform;

// ESTRUCTURA DE CACHÉ OPTIMIZADA
struct AnimLink {
    const Channel* channel;
    Transform* transform;

    glm::vec3 originalPos;
    glm::quat originalRot;
    glm::vec3 originalScl;
};

class Animation : public Component, public ResourceUser, public EventListener
{
public:
    Animation(GameObject* owner);
    virtual ~Animation();

    void CleanUp() override;

    void Update() override;

    ComponentType GetType() override { return ComponentType::Animation; };

    void AddAnimation(const std::string& name, uint32_t uid);



    void Play(const std::string& name);
    void ResetPose();
    void Stop();
    void Pause();

    void SetAnimation(UID animUID);

    void OnEditor() override;

    void OnEvent(const Event& event) override;
    void OnResourceLost(UID resourceUID) override;


private:
    void BuildAnimCache();
    void InvalidateBoneMap();

    glm::vec3 GetPositionValue(const Channel& channel, float currentAnimTime);
    glm::quat GetRotationValue(const Channel& channel, float currentAnimTime);
    glm::vec3 GetScaleValue(const Channel& channel, float currentAnimTime);

    void UpdateTransformations(const ResourceAnimation* animation, float currentAnimTime);


public:
    UID resourceUID = 0;
    ResourceAnimation* resource = nullptr;

    std::map<std::string, UID> animationsLibrary;

    bool loop = true;
    bool playing = false;
    float speed = 1.0f;

    float currentTime = 0.0f;

private:
    //CACHE
    std::map<std::string, GameObject*> boneMap;
    std::vector<AnimLink> animCache;


    bool debugDraw = false;
    bool invalidatingFlag = false;
};