#pragma once
#include <string>
#include <glm/glm.hpp>

class GameObject;

struct Event
{
    enum class Type
    {
        WindowResize,
        WindowClose,
        FileDropped,

        SceneLoaded,
        SceneSaved,
        SceneCleared,

        GameObjectCreated,
        GameObjectDestroyed,
        GameObjectEnabled,
        GameObjectDisabled,
        GameObjectSelected,
        GameObjectDeselected,

        TransformChanged,

        StaticChanged,

        Play,
        Pause,
        Unpause,
        Stop,

        TimeScaleChanged,

        Custom,
        Invalid
    };

    Type type;

    Event(Type type) : type(type) {}
    virtual ~Event() = default;
};


struct WindowResizeEvent : public Event
{
    int width;
    int height;

    WindowResizeEvent(int w, int h)
        : Event(Type::WindowResize), width(w), height(h) {
    }
};

struct FileDroppedEvent : public Event
{
    std::string filePath;

    FileDroppedEvent(const std::string& path)
        : Event(Type::FileDropped), filePath(path) {
    }
};

struct GameObjectEvent : public Event
{
    GameObject* gameObject;

    GameObjectEvent(Type type, GameObject* go)
        : Event(type), gameObject(go) {
    }
};

struct TransformChangedEvent : public Event
{
    GameObject* gameObject;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    TransformChangedEvent(GameObject* go, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
        : Event(Type::TransformChanged), gameObject(go), position(pos), rotation(rot), scale(scl) {
    }
};

struct StaticChangedEvent : public Event
{
    GameObject* gameObject;
    bool isStatic;

    StaticChangedEvent(GameObject* go, bool static_)
        : Event(Type::StaticChanged), gameObject(go), isStatic(static_) {
    }
};

struct TimeScaleChangedEvent : public Event
{
    float timeScale;

    TimeScaleChangedEvent(float scale)
        : Event(Type::TimeScaleChanged), timeScale(scale) {
    }
};

struct CustomEvent : public Event
{
    std::string eventName;
    void* data;

    CustomEvent(const std::string& name, void* data = nullptr)
        : Event(Type::Custom), eventName(name), data(data) {
    }
};