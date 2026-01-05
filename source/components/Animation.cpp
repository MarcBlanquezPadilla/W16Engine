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
        currentAnimation->UnloadFromMemory(); // Importante liberar al borrar el componente
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
            InvalidateBoneMap();
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
    if (Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_I) == KEY_DOWN) SetAnimation(1545900184);

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
    // 1. Caso base: Solo hay una key
    if (channel.positionKeys.size() == 1) return channel.positionKeys[0].value;

    // 2. Buscar entre qué dos keys estamos
    // Iteramos hasta encontrar la key justo ANTES de nuestro tiempo actual
    int p0_index = -1;
    for (int i = 0; i < channel.positionKeys.size() - 1; i++)
    {
        if (currentAnimTime < channel.positionKeys[i + 1].time)
        {
            p0_index = i;
            break;
        }
    }

    // Seguridad: si nos pasamos de tiempo (final de la anim), devolvemos la última
    if (p0_index == -1) return channel.positionKeys.back().value;

    // 3. Calcular factor de interpolación (0.0 a 1.0)
    int p1_index = p0_index + 1;
    const auto& key0 = channel.positionKeys[p0_index];
    const auto& key1 = channel.positionKeys[p1_index];

    float deltaTime = (float)(key1.time - key0.time);
    float factor = (currentAnimTime - (float)key0.time) / deltaTime;

    // Seguridad numérica
    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;

    // 4. Interpolación Lineal (LERP) para posición [cite: 66]
    return glm::mix(key0.value, key1.value, factor);
}

glm::quat Animation::GetRotationValue(const Channel& channel, float currentAnimTime)
{
    if (channel.rotationKeys.size() == 1) return channel.rotationKeys[0].value;

    int p0_index = -1;
    for (int i = 0; i < channel.rotationKeys.size() - 1; i++)
    {
        if (currentAnimTime < channel.rotationKeys[i + 1].time)
        {
            p0_index = i;
            break;
        }
    }

    if (p0_index == -1) return channel.rotationKeys.back().value;

    int p1_index = p0_index + 1;
    const auto& key0 = channel.rotationKeys[p0_index];
    const auto& key1 = channel.rotationKeys[p1_index];

    float deltaTime = (float)(key1.time - key0.time);
    float factor = (currentAnimTime - (float)key0.time) / deltaTime;

    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;

    // 4. Interpolación Esférica (SLERP) para rotación [cite: 67]
    // glm::slerp maneja el camino más corto automáticamente
    return glm::slerp(key0.value, key1.value, factor);
}

glm::vec3 Animation::GetScaleValue(const Channel& channel, float currentAnimTime)
{
    if (channel.scaleKeys.size() == 1) return channel.scaleKeys[0].value;

    int p0_index = -1;
    for (int i = 0; i < channel.scaleKeys.size() - 1; i++)
    {
        if (currentAnimTime < channel.scaleKeys[i + 1].time)
        {
            p0_index = i;
            break;
        }
    }

    if (p0_index == -1) return channel.scaleKeys.back().value;

    int p1_index = p0_index + 1;
    const auto& key0 = channel.scaleKeys[p0_index];
    const auto& key1 = channel.scaleKeys[p1_index];

    float deltaTime = (float)(key1.time - key0.time);
    float factor = (currentAnimTime - (float)key0.time) / deltaTime;

    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;

    // 4. Interpolación Lineal (LERP) para escala
    return glm::mix(key0.value, key1.value, factor);
}

void Animation::UpdateTransformations(const ResourceAnimation* animation, float currentAnimTime)
{
    // Recorremos todos los canales (huesos) que tiene la animación
    for (const auto& channel : animation->channels)
    {
        // Buscamos el GameObject en nuestro mapa caché
        auto it = boneMap.find(channel.name);

        if (it != boneMap.end())
        {
            GameObject* boneGO = it->second;
            Transform* transform = (Transform*)boneGO->GetComponent(ComponentType::Transform);

            if (transform)
            {
                // 1. Calculamos valores interpolados
                glm::vec3 position = GetPositionValue(channel, currentAnimTime);
                glm::quat rotation = GetRotationValue(channel, currentAnimTime);
                glm::vec3 scale = GetScaleValue(channel, currentAnimTime);

                // 2. Aplicamos al Transform
                // IMPORTANTE: Las animaciones siempre son locales respecto al padre
                transform->SetPosition(position);
                transform->SetQuaternionRotation(rotation); // Asegúrate de tener este setter
                transform->SetScale(scale);
            }
        }
    }
}