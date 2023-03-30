#pragma once
#include "framework.h"

#include <functional>
#include <unordered_map>
#include <vector>
#include <SDL2/SDL.h>

#include "Keys.h"
#include "InputEvent.hpp"

typedef std::function<void(const KeyState&)> key_event_callback;

class InputManager {
public:
	InputManager();
	~InputManager();
	
	void Update(SDL_Event& event);

	bool IsKeyDown(Keys key);
	bool IsMouseButton(MouseButton button);

	InputEvent* ListenKeyEvent(key_event_callback callback);

	RECT GetMousePosition();

	static InputManager* GetInstance();
	static void Release();
private:
	static InputManager* s_instance;

	void OnKeyEvent(SDL_Event& event, bool isDown);
	void OnMouseEvent(SDL_MouseButtonEvent& event, bool isDown);
	void OnMousePositionEvent(SDL_MouseMotionEvent& event);

	RECT m_mousePosition;
	std::unordered_map<Keys, bool> m_keyStates;
	std::unordered_map<MouseButton, bool> m_mouseButton;

	int m_keyEventCallbackIndex;
	std::unordered_map<int, key_event_callback> m_keyEventCallbacks;
};