#pragma once
#include "Component.h"
#include "../resources/ResourceAnimation.h"
#include "../EventListener.h"
#include <map>
#include <string>
#include <vector>

class GameObject;
class Transform;

struct BoneSnapshot {
    glm::vec3 pos;
    glm::quat rot;
    glm::vec3 scl;
};

struct BoneLink {
    std::string boneName;
    Transform* transform;
    const Channel* channelA;
    const Channel* channelB;

    glm::vec3 originalPos;
    glm::quat originalRot;
    glm::vec3 originalScl;
};

struct AnimationInstance {
    UID uid = 0;
    ResourceAnimation* resource = nullptr;

    float currentTime = 0.0f;
    float speed = 1.0f;
    bool loop = true;
    bool ended = false;
};

struct AnimationData {
    uint32_t uid = 0;
    std::string resourceName = " ";
    float speed = 1.0f;
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
    bool IsType(ComponentType type) override { return type == ComponentType::Animation; };

    void Save(Config& componentNode) override;
    void Load(Config& componentNode) override;

    void AddAnimation(const std::string& name, uint32_t uid, std::string resourceName);
    void RemoveAnimation(const std::string& name);

    void UnloadAnimation(AnimationInstance& animation);

    void Play(const std::string& name, float blendTime = 0.2f);
    const bool IsPlaying() { return playing; };
    void ResetPose();
    void Stop();
    void Pause();

    void SetAnimationSpeed(const std::string& name, float newSpeed);
    void SetAnimationLoop(const std::string& name, bool loop);

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

    void UpdateTransformations();
    void CaptureSnapshot();


public:
    
    AnimationInstance currentAnimation;
    std::map<std::string, AnimationData> animationsLibrary;

private:

    bool addAnimation = false;

    bool playing = false;

    bool isBlending = false;
    float blendDuration = 0.0f;
    float currentBlendTime = 0.0f;

    std::vector<BoneSnapshot> snapshotPose;
    std::vector<BoneLink> skeletonCache;
    std::map<std::string, int> boneIndexMap;
};