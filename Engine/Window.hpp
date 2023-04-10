#pragma once
#include "framework.h"
#include <string>

enum class RendererMode;

class Window {
public:
	bool Create(RendererMode mode, std::string title, int width, int height, int bufferWidth, int bufferHeight);
	bool Destroy();

	SDL_Window* GetWindow() const;
	HWND GetHandle() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetBufferWidth() const;
	int GetBufferHeight() const;

	void SetWindowTitle(std::string& title);
	void SetWindowSubTitle(std::string& subTitle);

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

	std::string m_mainTitle;
	std::string m_subTitle;
	
	SDL_Window* m_window;
};