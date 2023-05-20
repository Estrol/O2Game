#pragma once
#include <thread>
#include "Renderer.hpp"
#include "InputManager.hpp"
#include "SceneManager.hpp"
#include "Text.hpp"

enum class ThreadMode {
	SINGLE_THREAD,
	MULTI_THREAD
};

class Game {
public:
	Game();
	~Game();
	
	bool virtual Init();
	void virtual Run(double frameRate);
	void virtual Stop();

	void SetThreadMode(ThreadMode mode);
	void SetRenderMode(RendererMode mode);
	void SetFramelimit(double frameRate);

	void SetBufferSize(int width, int height);
	void SetWindowSize(int width, int height);
	
protected:
	virtual void Update(double deltaTime);
	virtual void Render(double deltaTime);
	virtual void Input(double deltaTime);
	virtual void Mouse(double deltaTime);

	Window* m_window;
	Renderer* m_renderer;
	InputManager* m_inputManager;
	SceneManager* m_sceneManager;

private:
	void DrawFPS(double delta);
	double m_frameInterval;
	int m_frameCount;
	int m_currentFrameCount;

	bool m_running;
	bool m_notify;
	double m_frameLimit;

	int m_bufferWidth, m_bufferHeight;
	int m_windowWidth, m_windowHeight;

	ThreadMode m_threadMode;
	RendererMode m_renderMode;
	std::thread m_audioThread;
	std::thread m_renderThread;

	Text* m_frameText;
};