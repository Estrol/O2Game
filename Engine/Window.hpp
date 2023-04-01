#pragma once
#include "framework.h"
#include <string>

class Window {
public:
	bool Create(std::string title, int width, int height, int bufferWidth, int bufferHeight);
	bool Destroy();

	SDL_Window* GetWindow() const;
	HWND GetHandle() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetBufferWidth() const;
	int GetBufferHeight() const;

	void SetWindowTitle(std::string& title);

	static Window* GetInstance();
	static void Release();
private:
	Window();
	~Window();

	static Window* s_instance;

	int m_width;
	int m_height;
	int m_bufferWidth;
	int m_bufferHeight;
	
	SDL_Window* m_window;
};