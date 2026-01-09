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
    if (resource)
    {
        resource->UnloadFromMemory();
        resource->RemoveReference(this);
        resource = nullptr;
        resourceUID = 0;
    }
}

void Animation::SetAnimation(UID uid)
{
    if (resource)
    {
        resource->UnloadFromMemory();
        resource->RemoveReference(this);
        resource = nullptr;
    }

    resourceUID = uid;

    resource = (ResourceAnimation*)Engine::GetInstance().moduleResources->RequestResource(uid);
    if (resource)
    {

        resource->LoadToMemory();
        resource->AddReference(this);
        int numChannels = resource->channels.size();

        InvalidateBoneMap();
        RebuildAnimCache();
    }
    else
    {
        resource = nullptr;
        resourceUID = 0;
    }
}

void Animation::ResetPose()
{
    // Restauramos la T-Pose guardada
    for (const auto& link : animCache)
    {
        if (link.transform)
        {
            link.transform->SetLocalPosition(link.originalPos);
            link.transform->SetLocalQuaternionRotation(link.originalRot);
            link.transform->SetLocalScale(link.originalScl);
        }
    }
}

void Animation::Play()
{
    playing = true;
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
    if (Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_I) == KEY_DOWN) SetAnimation(3007017118);

    if (!playing || !resource) return;

    if (invalidatingFlag)
    {
        InvalidateBoneMap();
        RebuildAnimCache();
        invalidatingFlag = false;
    }

    float dt = Time::deltaTime;
    currentTime += dt * resource->ticksPerSecond * speed;

    if (currentTime >= resource->duration)
    {
        if (loop)
        {
            currentTime = fmod(currentTime, resource->duration);
        }
        else
        {
            currentTime = resource->duration;
            playing = false;
        }
    }

    UpdateTransformations(resource, currentTime);
}

void Animation::InvalidateBoneMap()
{
    boneMap.clear();
    if (!resource || !owner) return;

    for (const auto& channel : resource->channels)
    {
        GameObject* bone = owner->FindChild(channel.name);

        if (bone)
        {
            boneMap[channel.name] = bone;
        }
    }
}

void Animation::OnEditor()
{
    if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Mostrar UID o Nombre
        ImGui::Text("Anim UID: %d", resource);

        // Botones de control
        if (ImGui::Button("Play")) Play();
        ImGui::SameLine();
        if (ImGui::Button("Stop")) Stop();

        // Slider de tiempo (scrubbing)
        if (resource)
        {
            float duration = (float)resource->duration;
            if (ImGui::SliderFloat("Time", &currentTime, 0.0f, duration))
            {
                // Si movemos el slider, actualizamos la pose manualmente
                // UpdateTransformations();
            }
            ImGui::Checkbox("Loop", &loop);
            ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 5.0f);

            ImGui::Text("Bones mapped: %d / %d", boneMap.size(), resource->channels.size());
        }
        else
        {
            ImGui::Text("No Animation Loaded");
        }
    }
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

void Animation::UpdateTransformations(const ResourceAnimation* animation, float currentAnimTime)
{
    // Iteramos sobre la CACHÉ (std::vector), acceso secuencial rapidísimo
    for (const auto& link : animCache)
    {
        // Usamos los punteros directos que guardamos en RebuildAnimCache
        // Y pasamos los índices por referencia para que se actualicen solos

        glm::vec3 position = GetPositionValue(*link.channel, currentAnimTime);
        glm::quat rotation = GetRotationValue(*link.channel, currentAnimTime);
        glm::vec3 scale = GetScaleValue(*link.channel, currentAnimTime);

        // Aplicamos la transformación en BATCH (Todo de golpe)
        // (Asegúrate de haber implementado SetLocalTransform en Transform.cpp como hablamos)
        link.transform->SetLocalPosition(position);
        link.transform->SetLocalQuaternionRotation(rotation);
        link.transform->SetLocalScale(scale);
    }
}

void Animation::RebuildAnimCache()
{
    animCache.clear();
    if (!resource || !owner) return;

    // Reservamos memoria para evitar realocaciones
    animCache.reserve(resource->channels.size());

    for (const auto& channel : resource->channels)
    {
        // 1. Buscamos el GameObject (Lento, pero solo 1 vez)
        auto it = boneMap.find(channel.name);

        if (it != boneMap.end())
        {
            GameObject* boneGO = it->second;

            // 2. Buscamos el Transform (Lento, pero solo 1 vez)
            Transform* t = (Transform*)boneGO->transform;

            if (t)
            {
                // 3. ¡ÉXITO! Creamos el Enlace Directo
                AnimLink link;
                link.channel = &channel; // Guardamos puntero al canal
                link.transform = t;      // Guardamos puntero al transform

                link.originalPos = t->GetLocalPosition();
                link.originalRot = t->GetLocalQuaterionRotation(); // O GetRotation() local
                link.originalScl = t->GetLocalScale();

                animCache.push_back(link);
            }
        }
    }

    LOG("AnimCache reconstruida. %d huesos enlazados.", animCache.size());
}

void Animation::OnEvent(const Event& event)
{
    switch (event.type)
    {
    case Event::Type::GameObjectDestroyed:
    {
        if (boneMap.empty()) return;

        GameObject* deletedGO = event.data.gameObject.gameObject;

        for (auto pair : boneMap)
        {
            if (pair.second == deletedGO)
            {
                pair.second = nullptr;

                // B. Activamos la bandera para reconstruir en el siguiente Update
                invalidatingFlag = true;
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
    if (resourceUID == lostUID)
    {
        LOG("Animation resource deleted! Removing reference in Component.");
        resource = nullptr;
        resourceUID = 0;
    }
}