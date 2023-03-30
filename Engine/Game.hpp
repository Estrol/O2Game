#pragma once
#include <thread>
#include "Renderer.hpp"
#include "InputManager.hpp"
#include "SceneManager.hpp"

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
	void SetFramelimit(double frameRate);
	
protected:
	virtual void Update(double deltaTime);
	virtual void Render(double deltaTime);
	virtual void Input(double deltaTime);

	Window* m_window;
	Renderer* m_renderer;
	InputManager* m_inputManager;
	SceneManager* m_sceneManager;

private:
	bool m_running;
	bool m_notify;
	double m_frameLimit;

	ThreadMode m_threadMode;
	std::thread m_audioThread;
	std::thread m_renderThread;
};