#include "Animation.h"
#include "Transform.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleResources.h"
#include "../ModuleEvents.h"
#include "../ModuleInput.h"
#include "../utils/Time.h"
#include "../utils/Log.h"
#include "imgui.h"

#include "../ModuleInput.h"

Animation::Animation(GameObject* owner) : Component(owner)
{
    Engine::GetInstance().moduleEvents->Subscribe(Event::Type::GameObjectDestroyed, this);
}

Animation::~Animation()
{
    CleanUp();
}

void Animation::CleanUp()
{
    Engine::GetInstance().moduleEvents->Unsubscribe(Event::Type::GameObjectDestroyed, this);

    UnloadAnimation(currentAnimation);
}

void Animation::AddAnimation(const std::string& name, uint32_t uid, std::string resourceName)
{
    if (name.empty() || uid == 0) return;

    AnimationData data;
    data.uid = uid;
    data.resourceName = resourceName;
    data.loop = true;

    animationsLibrary[name] = data;
}

void Animation::RemoveAnimation(const std::string& name)
{
    auto it = animationsLibrary.find(name);

    if (it == animationsLibrary.end()) return;

    UID uidToRemove = it->second.uid;

    if (currentAnimation.uid == uidToRemove)
    {
        Stop();
    }

    animationsLibrary.erase(it);
}

void Animation::ResetPose()
{
    for (const auto& link : skeletonCache)
    {
        if (link.transform)
        {
            link.transform->SetLocalPosition(link.originalPos);
            link.transform->SetLocalQuaternionRotation(link.originalRot);
            link.transform->SetLocalScale(link.originalScl);
        }
    }
}

void Animation::Play(const std::string& name, float blendTime)
{
    auto it = animationsLibrary.find(name);
    if (it == animationsLibrary.end()) return;

    if (currentAnimation.uid == it->second.uid && !isBlending) return;

    CaptureSnapshot();
    UnloadAnimation(currentAnimation);

    currentAnimation.uid = it->second.uid;
    currentAnimation.speed = it->second.speed;
    currentAnimation.loop = it->second.loop;
    currentAnimation.resource = (ResourceAnimation*)Engine::GetInstance().moduleResources->RequestResource(currentAnimation.uid);
    if (currentAnimation.resource) {
        currentAnimation.resource->LoadToMemory();
        currentAnimation.resource->AddReference(this);
    }

    currentAnimation.currentTime = 0.0f;
    currentAnimation.ended = false;

    if (blendTime > 0.0f) {
        isBlending = true;
        currentBlendTime = 0.0f;
        blendDuration = blendTime;
    }
    else {
        isBlending = false;
    }

    playing = true;

    EnsureSkeletonMatches(currentAnimation.resource);
    UpdateChannelPointers();
}

void Animation::Stop()
{
    playing = false;
    isBlending = false;
    currentBlendTime = 0.0f;
    
    UnloadAnimation(currentAnimation);

    ResetPose();
}

void Animation::SetAnimationSpeed(const std::string& name, float newSpeed)
{

    auto it = animationsLibrary.find(name);
    if (it == animationsLibrary.end())
    {
        LOG(LogType::LOG_WARNING, "Trying to set speed for non-existent animation '%s'", name.c_str());
        return;
    }

    float speed = newSpeed;

    if (speed < 0) speed = 0;

    AnimationData& data = it->second;
    data.speed = speed;


    if (currentAnimation.resource && currentAnimation.uid == data.uid)
    {
        currentAnimation.speed = speed;
    }
}

void Animation::SetAnimationLoop(const std::string& name, bool loop)
{
    auto it = animationsLibrary.find(name);
    if (it == animationsLibrary.end()) return;

    AnimationData& data = it->second;
    data.loop = loop;

    if (currentAnimation.resource && currentAnimation.uid == data.uid)
    {
        currentAnimation.loop = loop;
    }
}

void Animation::UnloadAnimation(AnimationInstance& animation)
{
    if (animation.resource)
    {
        animation.resource->UnloadFromMemory();
        animation.resource->RemoveReference(this);
    }
    animation.resource = nullptr;
    animation.uid = 0;
}

void Animation::Update()
{
    auto attackIt = animationsLibrary.find("Attack");
    auto walkIt = animationsLibrary.find("Walk");
    auto idleIt = animationsLibrary.find("Idle");

    UID attackUID = (attackIt != animationsLibrary.end()) ? attackIt->second.uid : 0;
    UID walkUID = (walkIt != animationsLibrary.end()) ? walkIt->second.uid : 0;
    UID idleUID = (idleIt != animationsLibrary.end()) ? idleIt->second.uid : 0;

    bool isAttacking = (currentAnimation.uid == attackUID);
    bool isWalking = (currentAnimation.uid == walkUID);
    bool isIdle = (currentAnimation.uid == idleUID);

    if (!isAttacking || currentAnimation.ended)
    {
        if (Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
        {
            Play("Attack", 0.2f);
        }
        else if (Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_1) == KEY_REPEAT)
        {
            if (!isWalking) Play("Walk", 0.5f);
        }
        else
        {
            if (!isIdle) Play("Idle", 0.5f);
        }
    }

    if (!playing || !currentAnimation.resource) return;

    float dt = Time::deltaTime;

    currentAnimation.currentTime += dt * currentAnimation.resource->ticksPerSecond * currentAnimation.speed;

    if (currentAnimation.currentTime >= currentAnimation.resource->duration) {
        if (currentAnimation.loop)
            currentAnimation.currentTime = std::fmod(currentAnimation.currentTime, currentAnimation.resource->duration);
        else {
            currentAnimation.currentTime = currentAnimation.resource->duration;
            currentAnimation.ended = true;
        }
    }


    if (isBlending) {
        currentBlendTime += dt;
        if (currentBlendTime >= blendDuration) {
            isBlending = false;
            snapshotPose.clear();
        }
    }

    UpdateTransformations();
}

glm::vec3 Animation::GetPositionValue(const Channel& channel, float currentAnimTime)
{
    int frameIndex = (int)currentAnimTime;

    int numKeys = channel.positionKeys.size();

    if (frameIndex >= numKeys - 1) return channel.positionKeys.back();
    if (frameIndex < 0) return channel.positionKeys[0];

    const auto& key0 = channel.positionKeys[frameIndex];
    const auto& key1 = channel.positionKeys[frameIndex + 1];

    float factor = currentAnimTime - (float)frameIndex;

    return glm::mix(key0, key1, factor);
}

glm::quat Animation::GetRotationValue(const Channel& channel, float currentAnimTime)
{
    int frameIndex = (int)currentAnimTime;
    int numKeys = channel.rotationKeys.size();

    if (frameIndex >= numKeys - 1) return channel.rotationKeys.back();
    if (frameIndex < 0) return channel.rotationKeys[0];

    const auto& key0 = channel.rotationKeys[frameIndex];
    const auto& key1 = channel.rotationKeys[frameIndex + 1];

    float factor = currentAnimTime - (float)frameIndex;

    return glm::slerp(key0, key1, factor);
}

glm::vec3 Animation::GetScaleValue(const Channel& channel, float currentAnimTime)
{
    int frameIndex = (int)currentAnimTime;
    int numKeys = channel.scaleKeys.size();

    if (frameIndex >= numKeys - 1) return channel.scaleKeys.back();
    if (frameIndex < 0) return channel.scaleKeys[0];

    const auto& key0 = channel.scaleKeys[frameIndex];
    const auto& key1 = channel.scaleKeys[frameIndex + 1];

    float factor = currentAnimTime - (float)frameIndex;

    return glm::mix(key0, key1, factor);
}

void Animation::UpdateTransformations()
{
    float factor = isBlending ? glm::clamp(currentBlendTime / blendDuration, 0.0f, 1.0f) : 1.0f;

    for (size_t i = 0; i < skeletonCache.size(); ++i) {
        auto& link = skeletonCache[i];
        if (!link.transform) continue;

        // DESTINO: Siempre es la currentAnimation (Channel A)
        glm::vec3 targetPos = link.originalPos;
        glm::quat targetRot = link.originalRot;
        glm::vec3 targetScl = link.originalScl;

        if (link.channelA) {
            targetPos = GetPositionValue(*link.channelA, currentAnimation.currentTime);
            targetRot = GetRotationValue(*link.channelA, currentAnimation.currentTime);
            targetScl = GetScaleValue(*link.channelA, currentAnimation.currentTime);
        }

        // MEZCLA: Foto -> Destino
        if (isBlending && i < snapshotPose.size()) {
            link.transform->SetLocalPosition(glm::mix(snapshotPose[i].pos, targetPos, factor));
            link.transform->SetLocalQuaternionRotation(glm::slerp(snapshotPose[i].rot, targetRot, factor));
            link.transform->SetLocalScale(glm::mix(snapshotPose[i].scl, targetScl, factor));
        }
        else {
            link.transform->SetLocalPosition(targetPos);
            link.transform->SetLocalQuaternionRotation(targetRot);
            link.transform->SetLocalScale(targetScl);
        }
    }
}

void Animation::UpdateChannelPointers()
{
    if (!currentAnimation.resource) return;

    for (auto& link : skeletonCache)
    {
        link.channelA = FindChannel(currentAnimation.resource, link.boneName);
    }
}

void Animation::EnsureSkeletonMatches(const ResourceAnimation* anim)
{
    if (!anim || !owner) return;

    for (const auto& channel : anim->channels)
    {
        if (boneIndexMap.find(channel.name) == boneIndexMap.end())
        {
            GameObject* go = owner->FindChild(channel.name);
            if (go)
            {
                BoneLink newLink;
                newLink.boneName = channel.name;
                newLink.transform = (Transform*)go->transform;
                newLink.channelA = nullptr;
                newLink.channelB = nullptr;

                Transform* t = newLink.transform;
                newLink.originalPos = t->GetLocalPosition();
                newLink.originalRot = t->GetLocalQuaterionRotation();
                newLink.originalScl = t->GetLocalScale();

                skeletonCache.push_back(newLink);
                boneIndexMap[channel.name] = skeletonCache.size() - 1;
            }
        }
    }
}

const Channel* Animation::FindChannel(const ResourceAnimation* anim, const std::string& name)
{
    if (!anim) return nullptr;
    for (const auto& ch : anim->channels)
    {
        if (ch.name == name) return &ch;
    }
    return nullptr;
}

void Animation::OnEditor()
{
    if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Separator();
        ImGui::Text("Library:");

        int i = 0;
        for (auto it = animationsLibrary.begin(); it != animationsLibrary.end(); )
        {
            ImGui::PushID(it->first.c_str());

            bool deleteRequested = false;

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;
            
            bool isNodeOpen = ImGui::TreeNodeEx(it->first.c_str(), flags);

            ImGui::SameLine();

            float buttonWidth = 20.0f;
            float availableWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth - buttonWidth);

            if (ImGui::SmallButton("X"))
            {
                deleteRequested = true;
            }

            if (isNodeOpen)
            {
                if (!deleteRequested)
                {
                    ImGui::Unindent();
                    ImGui::Text("Name: %s", it->second.resourceName.c_str());


                    ImGui::Text("Speed");
                    ImGui::SameLine();
                    float speed = it->second.speed;
                    if (ImGui::InputFloat("##Speed", &speed))
                    {
                        SetAnimationSpeed(it->first, speed);
                    }

                    ImGui::Text("Loop");
                    ImGui::SameLine();
                    bool loop = it->second.loop;
                    if (ImGui::Checkbox("##Loop", &loop))
                    {
                        SetAnimationLoop(it->first, loop);
                    }

                    if (ImGui::Button("Play", ImVec2(-1, 0)))
                    {
                        Play(it->first, 0.5f);
                    }
                    ImGui::Indent();
                }

                ImGui::TreePop();
            }

            ImGui::PopID();

            if (deleteRequested)
            {
                auto nextIt = it;
                ++nextIt;

                RemoveAnimation(it->first);

                it = nextIt;
            }
            else
            {
                ++it;
            }
            i++;
        }

        if (i == 0)
        {
            ImGui::SameLine();
            ImGui::Text("empty");
        }

        ImGui::Separator();

        static char nameBuffer[64] = "New Animation";
        int availableWidth = ImGui::GetContentRegionAvail().x;

        if (addAnimation)
        {
            ImGui::InputText(" ", nameBuffer, 64);
            ImGui::Button("Drop animation", ImVec2(availableWidth, 20));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(RESOURCE_DRAG))
                {
                    UID droppedUID = *(UID*)payload->Data;

                    const Resource* res = Engine::GetInstance().moduleResources->PeekResource(droppedUID);
                    if (res && res->GetType() == Resource::Type::animation)
                    {
                        AddAnimation(nameBuffer, droppedUID, res->GetName());
                        addAnimation = false;
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }
        else
        {
            if (ImGui::Button("Add animation", ImVec2(availableWidth, 20)))
            {
                addAnimation = true;
            }
        }
    }
}

void Animation::Save(Config& componentNode)
{
    for (const auto& [name, data] : animationsLibrary)
    {
        Config animationNode = componentNode.AddChild("Animation");
        animationNode.SetString("name", name);
        animationNode.SetUInt("UID", data.uid);
        animationNode.SetBool("loop", data.loop);
        animationNode.SetFloat("speed", data.speed);
    }
}

void Animation::Load(Config& componentNode)
{
    Config animationNode = componentNode.GetChild("Animation");
    while (animationNode.IsValid())
    {
        UID animationUID = animationNode.GetUInt("UID");
        if (animationUID != 0)
        {
            std::string animationName = animationNode.GetString("name");
            const Resource* resource = Engine::GetInstance().moduleResources->PeekResource(animationUID);
            if (resource)
            {
                AddAnimation(animationName, animationUID, resource->GetName());
                animationsLibrary[animationName].loop = animationNode.GetBool("loop");
                animationsLibrary[animationName].speed = animationNode.GetFloat("speed");
            }
        }
        animationNode = animationNode.GetNextSibling("Animation");
    }
}

void Animation::OnEvent(const Event& event)
{
    switch (event.type)
    {
    case Event::Type::GameObjectDestroyed:
    {
        if (skeletonCache.empty()) return;

        GameObject* deletedGO = event.data.gameObject.gameObject;

        for (auto& link : skeletonCache)
        {
            if (link.transform && link.transform->owner == deletedGO)
            {
                link.transform = nullptr;
            }
        }
        break;
    }

    default:
        break;
    }
}

void Animation::OnResourceLost(UID lostUID)
{
    if (currentAnimation.uid == lostUID)
    {
        LOG(LogType::LOG_INFO, "Current animation resource deleted! Removing reference in Component.");
        currentAnimation.resource = nullptr;
        currentAnimation.uid = 0;
    }
}

void Animation::CaptureSnapshot()
{
    snapshotPose.clear();
    for (const auto& link : skeletonCache)
    {
        if (link.transform)
        {
            BoneSnapshot snap;
            snap.pos = link.transform->GetLocalPosition();
            snap.rot = link.transform->GetLocalQuaterionRotation();
            snap.scl = link.transform->GetLocalScale();
            snapshotPose.push_back(snap);
        }
    }
    isBlending = true;
}