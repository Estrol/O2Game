#pragma once
#include "framework.h"
#include <string>

class Window {
public:
	Window();
	~Window();

	bool Create(std::string title, int width, int height);
	bool Destroy();

	SDL_Window* GetWindow() const;
	HWND GetHandle() const;
	int GetWidth() const;
	int GetHeight() const;

	void SetWindowTitle(std::string& title);

	static Window* GetInstance();
	static void Release();
private:
	static Window* s_instance;

	int m_width;
	int m_height;
	
	SDL_Window* m_window;
};