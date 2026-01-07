#include "Animation.h"
#include "Transform.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../ModuleResources.h"
#include "../utils/Time.h"
#include "../utils/Log.h"
#include "imgui.h"

//TEST
#include "../ModuleInput.h"

Animation::Animation(GameObject* owner) : Component(owner)
{
}

Animation::~Animation()
{
    if (currentAnimation)
    {
        currentAnimation->UnloadFromMemory();
    }
}

void Animation::SetAnimation(UID uid)
{
    // 1. Limpieza si ya teníamos una
    if (currentAnimation)
    {
        currentAnimation->UnloadFromMemory();
        currentAnimation = nullptr;
    }

    animationUID = uid;

    // 2. Pedir recurso al módulo
    if (uid != 0)
    {
        currentAnimation = (ResourceAnimation*)Engine::GetInstance().moduleResources->RequestResource(uid);
        if (currentAnimation)
        {
            currentAnimation->LoadToMemory();
            int numChannels = currentAnimation->channels.size();

            InvalidateBoneMap();
            RebuildAnimCache();
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
}

// EL CORAZÓN DEL SISTEMA
void Animation::Update()
{
    if (Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_I) == KEY_DOWN) SetAnimation(3573241609);

    if (!playing || !currentAnimation) return;

    // 1. Calcular paso de tiempo
    // dt (segundos) * ticksPorSegundo * velocidad
    float dt = Time::deltaTime; // O GameDeltaTime según tu motor
    currentTime += dt * currentAnimation->ticksPerSecond * speed;

    // 2. Gestión del Loop
    if (currentTime >= currentAnimation->duration)
    {
        if (loop)
        {
            // Opción A: Reset a 0 (más brusco pero fácil)
            // currentTime = 0.0f;

            // Opción B: Módulo (más preciso matemáticas)
            currentTime = fmod(currentTime, currentAnimation->duration);
        }
        else
        {
            // Fin de la animación
            currentTime = currentAnimation->duration;
            playing = false;
        }
    }

    // 3. AQUÍ IRÁ LA MAGIA DE MOVER LOS HUESOS (Siguiente paso)
    UpdateTransformations(currentAnimation, currentTime);
}

void Animation::InvalidateBoneMap()
{
    boneMap.clear();
    if (!currentAnimation || !owner) return;

    // Recorremos los canales de la animación (lo que el archivo dice que se mueve)
    for (const auto& channel : currentAnimation->channels)
    {
        // Buscamos en la jerarquía del GameObject dueño del componente
        // Asumo que tienes una función FindChild recursiva en GameObject
        GameObject* bone = owner->FindChild(channel.name);

        if (bone)
        {
            boneMap[channel.name] = bone;
        }
        else
        {
            LOG("Warning: Animation channel '%s' not found in GameObject hierarchy.", channel.name.c_str());
        }
    }
}

void Animation::OnEditor()
{
    if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Mostrar UID o Nombre
        ImGui::Text("Anim UID: %d", animationUID);

        // Botones de control
        if (ImGui::Button("Play")) Play();
        ImGui::SameLine();
        if (ImGui::Button("Stop")) Stop();

        // Slider de tiempo (scrubbing)
        if (currentAnimation)
        {
            float duration = (float)currentAnimation->duration;
            if (ImGui::SliderFloat("Time", &currentTime, 0.0f, duration))
            {
                // Si movemos el slider, actualizamos la pose manualmente
                // UpdateTransformations();
            }
            ImGui::Checkbox("Loop", &loop);
            ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 5.0f);

            ImGui::Text("Bones mapped: %d / %d", boneMap.size(), currentAnimation->channels.size());
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
        link.transform->SetPosition(position);
        link.transform->SetQuaternionRotation(rotation);
        link.transform->SetScale(scale);
    }
}

void Animation::RebuildAnimCache()
{
    animCache.clear();
    if (!currentAnimation || !owner) return;

    // Reservamos memoria para evitar realocaciones
    animCache.reserve(currentAnimation->channels.size());

    for (const auto& channel : currentAnimation->channels)
    {
        // 1. Buscamos el GameObject (Lento, pero solo 1 vez)
        auto it = boneMap.find(channel.name);

        if (it != boneMap.end())
        {
            GameObject* boneGO = it->second;

            // 2. Buscamos el Transform (Lento, pero solo 1 vez)
            Transform* t = (Transform*)boneGO->GetComponent(ComponentType::Transform);

            if (t)
            {
                // 3. ¡ÉXITO! Creamos el Enlace Directo
                AnimLink link;
                link.channel = &channel; // Guardamos puntero al canal
                link.transform = t;      // Guardamos puntero al transform

                // Reseteamos índices
                link.lastPosIndex = 0;
                link.lastRotIndex = 0;
                link.lastSclIndex = 0;

                animCache.push_back(link);
            }
        }
    }

    LOG("AnimCache reconstruida. %d huesos enlazados.", animCache.size());
}