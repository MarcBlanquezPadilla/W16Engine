#include "Animation.h"
#include "Transform.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleResources.h"
#include "../ModuleEvents.h"
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


    if (currentAnimation)
    {
        currentAnimation->UnloadFromMemory();
        currentAnimation->RemoveReference(this);
        currentAnimation = nullptr;
        currentAnimationUID = 0;
    }

    if (targetAnimation)
    {
        targetAnimation->UnloadFromMemory();
        targetAnimation->RemoveReference(this);
        targetAnimation = nullptr;
    }
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

void Animation::ResetPose()
{
    // Recorremos todo el esqueleto que hemos descubierto hasta ahora
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

    // 1. RECUPERAMOS LOS DATOS DE LA LIBRERÍA
    AnimationData& data = it->second;
    UID newUID = data.uid;
    bool shouldLoop = data.loop; // Leemos la config guardada

    // --- CASO 1: RESET / INICIO ---
    if (blendTime <= 0.0f || !currentAnimation || !playing)
    {
        if (currentAnimation)
        {
            currentAnimation->UnloadFromMemory();
            currentAnimation->RemoveReference(this);
        }

        currentAnimationUID = newUID;
        currentAnimation = (ResourceAnimation*)Engine::GetInstance().moduleResources->RequestResource(newUID);

        if (currentAnimation)
        {
            currentAnimation->LoadToMemory();
            currentAnimation->AddReference(this);
        }

        // APLICAMOS EL LOOP QUE HEMOS LEÍDO
        this->loop = shouldLoop;

        playing = true;
        currentTime = 0.0f;
        isBlending = false;
        targetAnimation = nullptr;

        EnsureSkeletonMatches(currentAnimation);
        UpdateChannelPointers();
    }
    // --- CASO 2: BLENDING ---
    else
    {
        if (currentAnimation->GetUID() == newUID) return;

        targetAnimation = (ResourceAnimation*)Engine::GetInstance().moduleResources->RequestResource(newUID);

        if (targetAnimation)
        {
            targetAnimation->LoadToMemory();
            targetAnimation->AddReference(this);

            targetTime = 0.0f;
            isBlending = true;
            blendDuration = blendTime;
            currentBlendTime = 0.0f;

            // APLICAMOS EL LOOP AQUÍ TAMBIÉN
            this->loop = shouldLoop;

            EnsureSkeletonMatches(targetAnimation);
            UpdateChannelPointers();
        }
    }
}

void Animation::Stop()
{
    playing = false;
    currentTime = 0.0f;

    ResetPose();
}

// EL CORAZÓN DEL SISTEMA
void Animation::Update()
{
    // Si no estamos reproduciendo o no hay recurso base, no hacemos nada
    if (!playing || !currentAnimation) return;

    float dt = Time::deltaTime;

    // =============================================================
    // 1. AVANZAR ANIMACIÓN ACTUAL (SOURCE / A)
    // =============================================================
    currentTime += dt * currentAnimation->ticksPerSecond * speed;

    // Gestión del Loop para A
    if (currentTime >= currentAnimation->duration)
    {
        if (loop)
        {
            currentTime = std::fmod(currentTime, currentAnimation->duration);
        }
        else
        {
            currentTime = currentAnimation->duration;

            // IMPORTANTE: Si estamos mezclando, NO paramos 'playing'.
            // Queremos que A se congele en el último frame mientras B entra suavemente.
            if (!isBlending)
            {
                playing = false;
            }
        }
    }

    // =============================================================
    // 2. AVANZAR ANIMACIÓN DESTINO (TARGET / B) - Solo si hay Blend
    // =============================================================
    if (isBlending && targetAnimation)
    {
        // Avanzamos el tiempo de la animación B
        targetTime += dt * targetAnimation->ticksPerSecond * speed;

        // Avanzamos el cronómetro de la mezcla
        currentBlendTime += dt;

        // Gestión del Loop para B (Target)
        // Por defecto asumimos loop, o podrías leer una flag de la targetAnimation
        if (targetTime >= targetAnimation->duration)
        {
            if (this->loop)
            {
                targetTime = std::fmod(targetTime, targetAnimation->duration);
            }
            else
            {
                targetTime = targetAnimation->duration;
                // No paramos 'playing' aquí, esperamos al swap
            }
        }

        // =============================================================
        // 3. FIN DE LA TRANSICIÓN (SWAP)
        // =============================================================
        if (currentBlendTime >= blendDuration)
        {
            // ¡El Rey ha muerto, viva el Rey!

            // A. Liberamos la animación vieja (A)
            // Nota: Si usas contadores de referencia, aquí restas uno.
            currentAnimation->RemoveReference(this);
            currentAnimation->UnloadFromMemory();

            // B. Promocionamos la animación nueva (B -> A)
            currentAnimation = targetAnimation;
            currentAnimationUID = targetAnimation->GetUID(); // Mantener UID sincronizado
            currentTime = targetTime; // Sincronizamos el tiempo para que no salte

            // C. Reseteamos variables de blend
            targetAnimation = nullptr;
            isBlending = false;
            currentBlendTime = 0.0f;

            // D. RE-CONECTAR LOS CABLES
            // Esto es vital: Ahora 'channelA' en el esqueleto debe apuntar 
            // a los canales de la nueva animación (la que antes era B).
            // Y 'channelB' pasará a ser nullptr.
            UpdateChannelPointers();
        }
    }

    // =============================================================
    // 4. APLICAR TRANSFORMACIONES
    // =============================================================
    // Ya no necesitamos pasar argumentos, la función lee 'currentTime', 'targetTime', etc.
    UpdateTransformations(nullptr, 0);
}

glm::vec3 Animation::GetPositionValue(const Channel& channel, float currentAnimTime)
{
    // 1. CALCULO DIRECTO DEL ÍNDICE (O(1))
    // Convertimos el tiempo (float) directamente a entero (int).
    // Ej: 15.7 -> 15
    int frameIndex = (int)currentAnimTime;

    // 2. SEGURIDAD RÁPIDA (Solo comprobamos bordes)
    int numKeys = channel.positionKeys.size();

    // Si nos salimos por arriba, devolvemos la última
    if (frameIndex >= numKeys - 1) return channel.positionKeys.back();
    // Si es negativo (raro), la primera
    if (frameIndex < 0) return channel.positionKeys[0];

    // 3. INTERPOLACIÓN (Suavizado entre frames)
    // Aunque esté baked a 30FPS, si el juego va a 60FPS necesitamos interpolar
    // para que no se vea a saltos.
    const auto& key0 = channel.positionKeys[frameIndex];
    const auto& key1 = channel.positionKeys[frameIndex + 1];

    // El factor es simplemente la parte decimal del tiempo
    // Ej: Tiempo 15.7 -> frame 15, factor 0.7
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

    // AQUÍ SÍ USAMOS SLERP (Para evitar glitches de rotación)
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

// =============================================================
// UPDATE LIMPIO (Sin Strings, Sin Mapas)
// =============================================================

void Animation::UpdateTransformations(const ResourceAnimation* ignored, float currentAnimTime)
{
    float factor = 0.0f;
    if (isBlending)
    {
        factor = currentBlendTime / blendDuration;
        if (factor > 1.0f) factor = 1.0f;
    }

    for (const auto& link : skeletonCache)
    {
        if (!link.transform) continue;

        // VALORES DEFAULT
        glm::vec3 finalPos = link.transform->GetLocalPosition();
        glm::quat finalRot = link.transform->GetLocalQuaterionRotation();
        glm::vec3 finalScl = link.transform->GetLocalScale();

        // APORTACIÓN A
        if (link.channelA)
        {
            finalPos = GetPositionValue(*link.channelA, currentTime);
            finalRot = GetRotationValue(*link.channelA, currentTime);
            finalScl = GetScaleValue(*link.channelA, currentTime);
        }

        // BLEND CON B
        if (isBlending && link.channelB)
        {
            glm::vec3 posB = GetPositionValue(*link.channelB, targetTime);
            glm::quat rotB = GetRotationValue(*link.channelB, targetTime);
            glm::vec3 sclB = GetScaleValue(*link.channelB, targetTime);

            finalPos = glm::mix(finalPos, posB, factor);
            finalRot = glm::slerp(finalRot, rotB, factor);
            finalScl = glm::mix(finalScl, sclB, factor);
        }

        link.transform->SetLocalPosition(finalPos);
        link.transform->SetLocalQuaternionRotation(finalRot);
        link.transform->SetLocalScale(finalScl);
    }
}

void Animation::UpdateChannelPointers()
{
    // Iteramos sobre NUESTRA caché (todos los huesos que hemos descubierto hasta ahora)
    for (auto& link : skeletonCache)
    {
        // 1. Enlazamos Animación A (Current)
        link.channelA = FindChannel(currentAnimation, link.boneName);

        // 2. Enlazamos Animación B (Target)
        if (isBlending && targetAnimation)
        {
            link.channelB = FindChannel(targetAnimation, link.boneName);
        }
        else
        {
            link.channelB = nullptr;
        }
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

                // --- NUEVO: Capturamos la T-Pose aquí ---
                Transform* t = newLink.transform;
                newLink.originalPos = t->GetLocalPosition();
                newLink.originalRot = t->GetLocalQuaterionRotation();
                newLink.originalScl = t->GetLocalScale();
                // ----------------------------------------

                skeletonCache.push_back(newLink);
                boneIndexMap[channel.name] = skeletonCache.size() - 1;
            }
            // ... logs de error ...
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
        // Iteramos el mapa
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

            if (isNodeOpen && !deleteRequested)
            {
                ImGui::Text("Name: %s", it->second.resourceName.c_str());

                ImGui::Checkbox("Loop", &it->second.loop);
 
                if (ImGui::Button("PLAY", ImVec2(-1, 0)))
                {
                    Play(it->first, 0.5f);
                }

                ImGui::TreePop();
            }

            ImGui::PopID();

            if (deleteRequested)
            {
                it = animationsLibrary.erase(it);
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
    if (currentAnimationUID == lostUID)
    {
        LOG("Animation currentAnimation deleted! Removing reference in Component.");
        currentAnimation = nullptr;
        currentAnimationUID = 0;
    }
}