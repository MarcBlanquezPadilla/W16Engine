class GameObject;

enum class ComponentType {
    None,
    Transform,
    Mesh,
    Texture
};

class Component
{
public:

    Component(GameObject* owner) : owner(owner), enabled(true) {}

    virtual ~Component() {}

    virtual void Start() {}

    virtual void Update(float deltaTime) {}

    virtual void OnDestroy() {}

    virtual void OnEnable() {}
    
    virtual void OnDisable() {}
    
    virtual ComponentType GetType() const = 0;

public:
    GameObject* owner;
    bool enabled;
};