#pragma once
#include <string>
#include "../Rendering/WindowsTypes.h"
#include <SDL2/SDL.h>

enum class RendererMode;

class GameWindow {
public:
	bool Create(RendererMode mode, std::string title, int width, int height, int bufferWidth, int bufferHeight);
	bool Destroy();

	void ResizeWindow(int width, int height);
	void ResizeBuffer(int width, int height);

	bool ShouldResizeRenderer();
	void HandleResizeRenderer();

	SDL_Window* GetWindow() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetBufferWidth() const;
	int GetBufferHeight() const;
	float GetWidthScale();
	float GetHeightScale();

	void SetScaleOutput(bool value);
	bool IsScaleOutput();

	void SetWindowTitle(std::string& title);
	void SetWindowSubTitle(std::string& subTitle);

	static GameWindow* GetInstance();
	static void Release();
private:
	GameWindow();
	~GameWindow();

	static GameWindow* s_instance;

	bool m_scaleOutput;
	bool m_resizeRenderer;
	int m_width;
	int m_height;
	int m_bufferWidth;
	int m_bufferHeight;

	std::string m_mainTitle;
	std::string m_subTitle;
	
	SDL_Window* m_window;
};