#pragma once
#include "Component.h"
#include "../resources/ResourceAnimation.h"
#include <map>
#include <string>

class GameObject;

class Animation : public Component
{
public:
    Animation(GameObject* owner);
    virtual ~Animation();

    void Update() override; // Aquí actualizaremos el tiempo y los huesos

    ComponentType GetType() override {
        return ComponentType::Animation;
    };

    // Gestión de la animación
    void SetAnimation(UID animUID);
    void Play();
    void Stop();
    void Pause();

    void OnEditor() override; // Para verlo en el inspector

private:
    // Esta función busca los GameObjects hijos que coincidan con los nombres de los canales
    void InvalidateBoneMap();
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

    float currentTime = 0.0f; // Tiempo actual en Ticks (no en segundos)

private:
    // CACHÉ: Nombre del hueso -> Puntero al GameObject
    // Ejemplo: "Mixamorig:LeftHand" -> GameObject* (0x00...)
    std::map<std::string, GameObject*> boneMap;

    // Auxiliar para dibujar el esqueleto (Debug)
    bool debugDraw = false;
};