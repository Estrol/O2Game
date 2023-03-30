#include "InputManager.hpp"

InputManager::InputManager() {
	for (Keys i = Keys::A; i != Keys::INVALID_KEY; i = static_cast<Keys>(static_cast<int>(i) + 1)) {
		m_keyStates[i] = false;
	}

	for (MouseButton i = MouseButton::LEFT; i != MouseButton::INVALID_BUTTON; i = static_cast<MouseButton>(static_cast<int>(i) + 1)) {
		m_mouseButton[i] = false;
	}

	m_mousePosition = { 0, 0, 0, 0 };

	m_keyEventCallbackIndex = 1;
}

InputManager::~InputManager() {
	for (auto it = m_keyEventCallbacks.begin(); it != m_keyEventCallbacks.end(); it++) {
		it->second = nullptr;
	}
}

InputManager* InputManager::s_instance = nullptr;

void InputManager::Update(SDL_Event& event) {
	switch (event.type) {
		case SDL_KEYDOWN:
			OnKeyEvent(event, true);
			break;
		case SDL_KEYUP:
			OnKeyEvent(event, false);
			break;

		case SDL_MOUSEBUTTONDOWN:
			OnMouseEvent(event.button, true);
			break;

		case SDL_MOUSEBUTTONUP:
			OnMouseEvent(event.button, false);
			break;
			
		case SDL_MOUSEMOTION:
			OnMousePositionEvent(event.motion);
			break;
	}
}

InputManager* InputManager::GetInstance() {
	if (s_instance == nullptr) {
		s_instance = new InputManager();
	}

	return s_instance;
}

void InputManager::Release() {
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
	}
}

bool InputManager::IsKeyDown(Keys key) {
	return m_keyStates[key];
}

bool InputManager::IsMouseButton(MouseButton button) {
	return m_mouseButton[button];
}

InputEvent* InputManager::ListenKeyEvent(key_event_callback callback) {
	int index = m_keyEventCallbackIndex++;
	m_keyEventCallbacks[m_keyEventCallbackIndex++] = callback;

	InputEvent* env = new InputEvent;
	env->CreateCallback([&](int idx) {
		m_keyEventCallbacks.erase(idx);
	}, index);

	return env;
}

RECT InputManager::GetMousePosition() {
	return m_mousePosition;
}

void InputManager::OnKeyEvent(SDL_Event& event, bool isDown) {
	Keys key = (Keys)event.key.keysym.scancode;
	if (key == Keys::INVALID_KEY) {
		return;
	}

	int lastState = m_keyStates[key];
	KeyState state = { key, isDown ? KeyEventType::KEY_DOWN : KeyEventType::KEY_UP };

	if (isDown) {
		m_keyStates[key] = true;
		
		if (!lastState && m_keyStates[key]) {
			for (auto it : m_keyEventCallbacks) {
				it.second(state);
			}
		}
	}
	else {
		m_keyStates[key] = false;
		
		if (lastState && !m_keyStates[key]) {
			for (auto it : m_keyEventCallbacks) {
				it.second(state);
			}
		}
	}
}

void InputManager::OnMouseEvent(SDL_MouseButtonEvent& event, bool isDown) {
	MouseButton button = MouseButton::INVALID_BUTTON;

	switch (event.button) {
		case SDL_BUTTON_LEFT: button = MouseButton::LEFT; break;
		case SDL_BUTTON_RIGHT: button = MouseButton::RIGHT; break;
		case SDL_BUTTON_MIDDLE: button = MouseButton::MIDDLE; break;
	}

	if (button != MouseButton::INVALID_BUTTON) {
		m_mouseButton[button] = isDown;
	}
}

void InputManager::OnMousePositionEvent(SDL_MouseMotionEvent& event) {
	m_mousePosition = { event.x, event.y, 0, 0 };
}
