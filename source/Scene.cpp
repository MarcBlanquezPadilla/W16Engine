#include "Module.h"
#include<list>

class Texture;
class Mesh;
class Transform;

class GameObject
{
	Texture* texture;
	Mesh* mesh;
	Transform* transform;
};

class Render : public Module
{
public:

	Render(bool startEnabled);

	virtual ~Render();

	bool Awake();

	bool PreUpdate();
	bool PostUpdate();

	bool CleanUp();

private:
	
	std::list<GameObject*> gameObjects;
};