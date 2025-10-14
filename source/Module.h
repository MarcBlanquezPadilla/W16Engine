#pragma once
#include <string>

class GuiControl;

class Module
{
public:

	Module(bool startEnabled) : active(startEnabled) {}

	virtual ~Module() {}

	virtual bool Awake()
	{
		return true;
	}

	virtual bool Start()
	{
		return true;
	}

	virtual bool Update(float dt)
	{
		return true;
	}


	virtual bool CleanUp()
	{
		return true;
	}

	void Enable()
	{
		if (!active)
		{
			active = true;
			Start();
		}
	}

	void Disable()
	{
		if (active)
		{
			active = false;
			CleanUp();

		}
	}

	inline bool IsActive() const { return active; }

public:

	std::string name;
	bool active;
};