#pragma once

#include "Module.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_rect.h"
#include "Vector2D.h"
#include <vector>
#include <functional>

#define NUM_MOUSE_BUTTONS 5
#define MAX_GAMEPAD_BUTTONS SDL_GAMEPAD_BUTTON_COUNT

enum EventWindow
{
	WE_QUIT = 0,
	WE_HIDE = 1,
	WE_SHOW = 2,
	WE_COUNT
};

enum KeyState
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

class Input : public Module
{

public:

	Input(bool startEnabled);

	virtual ~Input();

	bool Awake();

	bool Start();

	bool PreUpdate();

	bool CleanUp();

	KeyState GetKey(int id) const
	{
		return keyboard[id];
	}

	KeyState GetMouseButtonDown(int id) const
	{
		return mouseButtons[id - 1];
	}

	bool GetWindowEvent(EventWindow ev);

	Vector2D GetMousePosition();
	
	Vector2D GetMouseMotion();
	
	float GetMouseWheelY();

	using InputListener = std::function<void(SDL_Event*)>;



public:

	KeyState gamepadButtons[MAX_GAMEPAD_BUTTONS] = {};

	SDL_Gamepad* controller = nullptr;

	KeyState GetGamepadButton(SDL_GamepadButton button) const;

private:

	bool windowEvents[WE_COUNT];
	KeyState* keyboard;
	KeyState mouseButtons[NUM_MOUSE_BUTTONS];
	int	mouseMotionX;
	int mouseMotionY;
	int mouseX;
	int mouseY;
	int scale;
	float mouseWheelY;
};