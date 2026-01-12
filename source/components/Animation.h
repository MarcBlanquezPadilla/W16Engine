#pragma once
#include "Component.h"
#include "../resources/ResourceAnimation.h"
#include "../EventListener.h"
#include <map>
#include <string>
#include <vector>

class GameObject;
class Transform;

// ESTRUCTURA DE CACHÉ OPTIMIZADA
struct BoneLink {
    std::string boneName;
    Transform* transform;
    const Channel* channelA;
    const Channel* channelB;

    glm::vec3 originalPos;
    glm::quat originalRot;
    glm::vec3 originalScl;
};

struct AnimationData {
    uint32_t uid = 0;
    std::string resourceName = " ";
    bool loop = true;
};

class Animation : public Component, public ResourceUser, public EventListener
{
public:
    Animation(GameObject* owner);
    virtual ~Animation();

    void CleanUp() override;

    void Update() override;

    ComponentType GetType() override { return ComponentType::Animation; };

    void AddAnimation(const std::string& name, uint32_t uid, std::string resourceName);

    void Play(const std::string& name, float blendTime = 0.2f);
    void ResetPose();
    void Stop();
    void Pause();

    void OnEditor() override;
    void OnEvent(const Event& event) override;
    void OnResourceLost(UID resourceUID) override;


private:

    void EnsureSkeletonMatches(const ResourceAnimation* anim);
    void UpdateChannelPointers();
    const Channel* FindChannel(const ResourceAnimation* anim, const std::string& name);

    glm::vec3 GetPositionValue(const Channel& channel, float currentAnimTime);
    glm::quat GetRotationValue(const Channel& channel, float currentAnimTime);
    glm::vec3 GetScaleValue(const Channel& channel, float currentAnimTime);

    void UpdateTransformations(const ResourceAnimation* animation, float currentAnimTime);


public:
    UID currentAnimationUID = 0;
    UID targetAnimationUID = 0;
    ResourceAnimation* currentAnimation = nullptr;
    ResourceAnimation* targetAnimation = nullptr;

    std::map<std::string, AnimationData> animationsLibrary;

    bool loop = true;
    bool playing = false;
    float speed = 1.0f;

    float currentTime = 0.0f;

private:

    // NUEVAS VARIABLES DE BLENDING
    float targetTime = 0.0f;

    bool isBlending = false;
    float blendDuration = 0.0f;
    float currentBlendTime = 0.0f;

    // LA NUEVA CACHÉ
    std::vector<BoneLink> skeletonCache;
    std::map<std::string, int> boneIndexMap;


    bool debugDraw = false;
    bool invalidatingFlag = false;
    bool addAnimation = false;
};