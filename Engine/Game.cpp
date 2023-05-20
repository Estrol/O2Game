#include <iostream>

#include "Game.hpp"
#include "framework.h"
#include "Window.hpp"
#include "AudioManager.hpp"
#include "Imgui/imgui_impl_sdl2.h"
#include "Imgui/imgui_impl_dx11.h"
#include "FontResources.hpp"
#include "MsgBox.hpp"

namespace {
	thread_local double curTick = 0.0;
	thread_local double lastTick = 0.0;

	bool InitSDL() {
		return SDL_Init(SDL_INIT_EVERYTHING) == 0;
	}

	//Improve accuracy frame limiter
	double FrameLimit(double MaxFrameRate) {
		double newTick = SDL_GetTicks();
		double deltaTick = 1000.0 / MaxFrameRate - (newTick - lastTick);

		if (deltaTick > 0.0) {
			int delayTicks = (int)deltaTick;
			if (delayTicks > 0) {
				SDL_Delay(delayTicks);
				newTick += delayTicks;
				deltaTick -= delayTicks;
			}
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

	m_renderMode = RendererMode::DIRECTX;
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

	std::cout << "Window::Create << " << std::endl;
	m_window = Window::GetInstance();
	if (!m_window->Create(m_renderMode, "Game", m_windowWidth, m_windowHeight, m_bufferWidth, m_bufferHeight)) {
		return false;
	}
	
	std::cout << "Renderer::Create << " << std::endl;
	m_renderer = Renderer::GetInstance();
	if (!m_renderer->Create(m_renderMode, m_window)) {
		return false;
	}

	std::cout << "InputManager::Create << " << std::endl;
	m_inputManager = InputManager::GetInstance();

	std::cout << "SceneManager::Create << " << std::endl;
	m_sceneManager = SceneManager::GetInstance();
	m_sceneManager->SetParent(this);

	std::cout << "AudioManager::Create << " << std::endl;
	if (!AudioManager::GetInstance()->Init(m_window)) {
		return false;
	}

	FontResources::PreloadFontCaches();
	m_frameText = new Text("Arial", 13);
	
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

	bool show_demo_window = true;
	m_renderThread = std::thread([&] {
		while (m_running) {
			if (m_threadMode == ThreadMode::MULTI_THREAD) {
				double delta = FrameLimit(m_frameLimit);
				
				Update(delta);
				
				m_renderer->BeginRender();
				Render(delta);
				MsgBox::Draw();
				DrawFPS(delta);
				m_renderer->EndRender();
			}
			else {
				FrameLimit(15.0f);
			}
		}
	});

	m_inputManager->ListenKeyEvent([&](const KeyState& state) {
		if (state.type == KeyEventType::KEY_DOWN) {
			m_sceneManager->OnKeyDown(state);
		}
		else {
			m_sceneManager->OnKeyUp(state);
		}
	});

	m_inputManager->ListenMouseEvent([&](const MouseState& state) {
		if (state.isDown) {
			m_sceneManager->OnMouseDown(state);
		}
		else {
			m_sceneManager->OnMouseUp(state);
		}
	});

	while (m_running) {
		double delta = FrameLimit(m_threadMode == ThreadMode::MULTI_THREAD ? 1000.0 : m_frameLimit);
		
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					MsgBox::Show("Quit", "Quit confirmation", "Are you sure you want to quit?", MsgBoxType::YESNO);
					break;
			}

			ImGui_ImplSDL2_ProcessEvent(&event);
			m_inputManager->Update(event);
		}

		if (MsgBox::GetResult("Quit") == 1) {
			Stop();
		}

		if (m_threadMode == ThreadMode::MULTI_THREAD) {
			Input(delta);
		}
		else {
			Input(delta);

			Update(delta);

			m_renderer->BeginRender();
			Render(delta);

			MsgBox::Draw();
			DrawFPS(delta);
			m_renderer->EndRender();
		}
	}

	m_audioThread.join();
	m_renderThread.join();
	
	m_notify = false;
}

void Game::Stop() {
	m_running = false;
}

void Game::SetThreadMode(ThreadMode mode) {
	m_threadMode = mode;
}

void Game::SetRenderMode(RendererMode mode) {
	m_renderMode = mode;
}

void Game::SetFramelimit(double frameRate) {
	m_frameLimit = frameRate;
}

void Game::SetBufferSize(int width, int height) {
	m_bufferWidth = width;
	m_bufferHeight = height;
}

void Game::SetWindowSize(int width, int height) {
	m_windowWidth = width;
	m_windowHeight = height;
}

void Game::Update(double deltaTime) {
	
}

void Game::Render(double deltaTime) {
	
}

void Game::Input(double deltaTime) {
	
}

void Game::Mouse(double deltaTime) {

}

void Game::DrawFPS(double delta) {
	m_frameInterval += delta;
	m_frameCount += 1;

	if (m_frameInterval >= 1.0) {
		m_frameInterval = 0.0;

		m_currentFrameCount = std::round(m_frameCount);
		m_frameCount = 0.0;
	}

	if (m_currentFrameCount >= 35 && m_currentFrameCount < 60) {
		m_frameText->Color3 = Color3(1.0f, 0.8f, 0.0f);
	}
	else if (m_currentFrameCount >= 0 && m_currentFrameCount < 35) {
		m_frameText->Color3 = Color3(1.0f, 0.0f, 0.0f);
	}
	else {
		m_frameText->Color3 = Color3(1.0f, 1.0f, 1.0f);
	}

	m_frameText->Position = UDim2::fromOffset(5, 5);
	m_frameText->DrawOverEverything = true;
	//m_frameText->Draw("FPS: " + std::to_string(m_currentFrameCount));
}
