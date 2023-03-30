#include <iostream>

#include "Game.hpp"
#include "framework.h"
#include "Window.hpp"
#include "AudioManager.hpp"

namespace {
	thread_local double curTick = 0.0;
	thread_local double lastTick = 0.0;

	bool InitSDL() {
		return SDL_Init(SDL_INIT_EVERYTHING) == 0;
	}

	double FrameLimit(double MaxFrameRate) {
		double newTick = SDL_GetTicks();
		double deltaTick = 1000.0 / MaxFrameRate - (newTick - lastTick);

		if (floor(deltaTick) > 0) {
			SDL_Delay(deltaTick);
		}

		if (deltaTick < -30.0) {
			lastTick = newTick;
		}
		else {
			lastTick = newTick + deltaTick;
		}

		double delta = (newTick - curTick) / 1000.0;
		curTick = newTick;

		return delta;
	}
}

Game::Game() {
	m_frameLimit = 60.0;
	m_running = false;
	m_notify = false;
	
	m_window = nullptr;
	m_renderer = nullptr;
	m_inputManager = nullptr;
	m_sceneManager = nullptr;

	m_threadMode = ThreadMode::MULTI_THREAD;
}

Game::~Game() {
	if (m_running) {
		Stop();

		// Wait for threads to finish
		// TODO: Figure a way to do this without using sleep
		while (m_notify) {
			SDL_Delay(500);
		}
	}

	AudioManager::Release();
	SceneManager::Release();
	InputManager::Release();
	Renderer::Release();
	Window::Release();
}

bool Game::Init() {
	if (!InitSDL()) {
		MessageBoxA(NULL, "SDL Failed to Initialize", "EstEngine Error", MB_ICONERROR);

		return false;
	}

	m_window = Window::GetInstance();
	if (!m_window->Create("Game", 1920, 1080)) {
		return false;
	}
	
	RECT rc(0, 0, 800, 600);
	m_renderer = Renderer::GetInstance();
	if (!m_renderer->Create(RendererMode::DIRECTX, m_window, rc)) {
		return false;
	}

	m_inputManager = InputManager::GetInstance();

	m_sceneManager = SceneManager::GetInstance();
	m_sceneManager->SetParent(this);

	AudioManager::GetInstance()->Init(m_window);
	
	return true;
}

void Game::Run(double frameRate) {
	if (!InitSDL()) {
		MessageBoxA(NULL, "SDL Failed to Initialize", "EstEngine Error", MB_ICONERROR);

		return;
	}

	m_running = true;
	m_notify = true;
	m_frameLimit = frameRate;

	m_audioThread = std::thread([&] {
		while (m_running) {
			double delta = FrameLimit(500.0);
			AudioManager::GetInstance()->Update(delta);
		}
	});

	m_renderThread = std::thread([&] {
		while (m_running) {
			if (m_threadMode == ThreadMode::MULTI_THREAD) {
				double delta = FrameLimit(m_frameLimit);
				
				Update(delta);
				
				m_renderer->BeginRender();
				Render(delta);
				m_renderer->EndRender();
			}
			else {
				FrameLimit(15.0f);
			}
		}
	});

	InputEvent* env = m_inputManager->ListenKeyEvent([&](const KeyState& state) {
		if (state.type == KeyEventType::KEY_DOWN) {
			m_sceneManager->OnKeyDown(state);
		}
		else {
			m_sceneManager->OnKeyUp(state);
		}
	});

	while (m_running) {
		double delta = FrameLimit(m_threadMode == ThreadMode::MULTI_THREAD ? 1000.0 : m_frameLimit);
		
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					Stop();
					break;
			}

			m_inputManager->Update(event);
		}

		if (m_threadMode == ThreadMode::MULTI_THREAD) {
			Input(delta);
		}
		else {
			Input(delta);

			Update(delta);

			m_renderer->BeginRender();
			Render(delta);
			m_renderer->EndRender();
		}
	}

	m_audioThread.join();
	m_renderThread.join();
	env->Disconnect();
	delete env;
	
	m_notify = false;
}

void Game::Stop() {
	m_running = false;
}

void Game::SetThreadMode(ThreadMode mode) {
	m_threadMode = mode;
}

void Game::SetFramelimit(double frameRate) {
	m_frameLimit = frameRate;
}

void Game::Update(double deltaTime) {
	
}

void Game::Render(double deltaTime) {
	
}

void Game::Input(double deltaTime) {
	
}
