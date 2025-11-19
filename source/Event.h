#pragma once
#include <string>
#include <glm/glm.hpp>
#include "SDL3/SDL.h"

class GameObject;
class Transform;

struct Event
{
    enum class Type
    {
        //WINDOW
        WindowResize,
        WindowClose,

        //INPUT
        EventSDL,
        FileDropped,

        //SCENE
        SceneLoaded,
        SceneSaved,
        SceneCleared,

        //GAMEOBJECT
        GameObjectCreated,
        GameObjectDestroyed,
        GameObjectEnabled,
        GameObjectDisabled,
        GameObjectSelected,
        GameObjectDeselected,
        StaticChanged,

        //TRANSFORM
        TransformChanged,

        //GAME
        Play,
        Pause,
        Unpause,
        Stop,
        TimeScaleChanged,

        //OTHERS
        Custom,
        Invalid
    };

    Type type;

    struct Point2dData {
        int x;
        int y;
    };

    struct StringData {
        char filePath[260];
    };

    struct GameObjectData {
        GameObject* gameObject;
    };

    struct SDLEvent
    {
        SDL_Event* event;
    };

    union Data
    {
        Point2dData point;
        StringData string;
        GameObjectData gameObject;
        SDLEvent event;
    } data;


    Event(Type t) : type(t) {}

    Event(Type t, int w, int h) : type(t) {
        data.point.x = w;
        data.point.y = h;
    }

    Event(Type t, const char* path) : type(t) {
        strncpy_s(data.string.filePath, path, 260);
    }

    Event(Type t, GameObject* gameObject) : type(t) {
        data.gameObject.gameObject = gameObject;
    }

    Event(Type t, SDL_Event* event) : type(t) {
        data.event.event = event;
    }

    Event() : type(Type::Invalid) {}
};

