#include <iostream>

#include "Game.h"
#include "Rendering/Window.h"
#include "Audio/AudioManager.h"
#include "Data/Imgui/imgui_impl_sdl2.h"
#include "Fonts/FontResources.h"
#include "MsgBox.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_image.h>
#include "Imgui/ImguiUtil.h"
#include "Data/Imgui/imgui_impl_sdlrenderer2.h"
#include "Rendering/Vulkan/VulkanEngine.h"
#include "Texture/MathUtils.h"

namespace {
	thread_local double curTick = 0.0;
	thread_local double lastTick = 0.0;

	bool InitSDL() {
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
			return false;
		}

		return IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != 0;
	}

	void DeInitSDL() {
		SDL_Quit();
		IMG_Quit();
	}

	// Improve accuracy frame limiter also fix framedrop if fps limit is enabled
	double FrameLimit(double MaxFrameRate) {
		double newTick = SDL_GetTicks();
		double deltaTick = 1000.0 / MaxFrameRate - (newTick - lastTick);

		if (deltaTick > 0.0) {
			int delayTicks = static_cast<int>(deltaTick);
			SDL_Delay(delayTicks);
			newTick += delayTicks;
			deltaTick -= delayTicks;
		}

		lastTick = (deltaTick < -30.0) ? newTick : newTick + deltaTick;

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

	DeInitSDL();
}

bool Game::Init() {
	if (!InitSDL()) {
		MsgBox::ShowOut("SDL Failed to Initialize", "EstEngine Error");

		return false;
	}

	std::cout << "Window::Create << " << std::endl;
	m_window = Window::GetInstance();

	int width = m_windowWidth;
	int height = m_windowHeight;
	if (m_fullscreen) {
		SDL_DisplayMode dm;
		SDL_GetCurrentDisplayMode(0, &dm);
		width = dm.w;
		height = dm.h;
	}

	if (!m_window->Create(m_renderMode, "Game", width, height, m_bufferWidth, m_bufferHeight)) {
		return false;
	}
	
	std::cout << "Renderer::Create << " << std::endl;
	m_renderer = Renderer::GetInstance();
	if (!m_renderer->Create(m_renderMode, m_window)) {
		return false;
	}

	if (m_renderMode == RendererMode::OPENGL) {
		m_threadMode = ThreadMode::SINGLE_THREAD; // OpenGL doesnt support multithreading
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
	m_currentFade = 0;
	m_targetFade = 0;
	
	return true;
}

void Game::Run(double frameRate) {
	m_running = true;
	m_notify = true;
	m_frameLimit = frameRate;
	m_frameLimitMode = FrameLimitMode::MENU;

	mAudioThread.Run([&] {
		double delta = FrameLimit(144.0);
		AudioManager::GetInstance()->Update(delta);
	}, true);

	std::mutex m1, m2;

	mRenderThread.Run([&] {
		if (m_threadMode == ThreadMode::MULTI_THREAD) {
			double delta = 0;

			switch (m_frameLimitMode) {
				case FrameLimitMode::GAME: {
					delta = FrameLimit(m_frameLimit);
					break;
				}

				case FrameLimitMode::MENU: {
					delta = FrameLimit(60.0);
					break;
				}
			}

			if (m_window->ShouldResizeRenderer()) {
				m_renderer->Resize();

				FontResources::DoRebuild();
				m_window->HandleResizeRenderer();
			}

			if (FontResources::ShouldRebuild()) {
				FontResources::PreloadFontCaches();
			}

			Update(delta);

			if (static_cast<int>(m_currentFade) != static_cast<int>(m_targetFade)) {
				float increment = (static_cast<float>(delta) * 5.0f) * 100.0f;

				// compare it using epsilon
				if (std::abs(m_currentFade - m_targetFade) < FLT_EPSILON) {
					m_currentFade = m_targetFade;
				}
				else {
					if (m_currentFade < m_targetFade) {
						m_currentFade = std::clamp(m_currentFade + increment, 0.0f, 100.0f);
					}
					else {
						m_currentFade = std::clamp(m_currentFade - increment, 0.0f, 100.0f);
					}
				}
			}

			if (!m_minimized) {
				std::lock_guard<std::mutex> lock(m1);

				if (m_renderer->BeginRender()) {
					Render(delta);
					MsgBox::Draw();

					if (static_cast<int>(m_currentFade) != 0) {
						auto drawList = ImGui::GetForegroundDrawList();
						float a = static_cast<int>(m_currentFade) / 100.0f;

						drawList->AddRectFilled(ImVec2(0, 0), MathUtil::ScaleVec2(m_window->GetBufferWidth(), m_window->GetBufferHeight()), IM_COL32(0, 0, 0, a * 255));
					}

					m_renderer->EndRender();
				}
			}
		}
		else {
			FrameLimit(15.0f);
		}
	}, true);

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

	m_minimized = false;
	mLocalThread.Run([&] {
		double delta = 0;
		switch (m_frameLimitMode) {
			case FrameLimitMode::GAME: {
				delta = FrameLimit(m_threadMode == ThreadMode::MULTI_THREAD ? 1000.0 : m_frameLimit);
				break;
			}

			case FrameLimitMode::MENU: {
				delta = FrameLimit(60.0);
				break;
			}
		}

		m_imguiInterval += delta;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					MsgBox::Show("Quit", "Quit confirmation", "Are you sure you want to quit?", MsgBoxType::YESNO);
					break;
			}

			m_minimized = SDL_GetWindowFlags(m_window->GetWindow()) & SDL_WINDOW_MINIMIZED;
			ImGui_ImplSDL2_ProcessEvent(&event);
			m_inputManager->Update(event);
		}

		if (MsgBox::GetResult("Quit") == 1) {
			Stop();
		}

		if (m_renderer->IsVulkan() && m_renderer->GetVulkanEngine()->_swapChainOutdated) {
			std::lock_guard<std::mutex> lock(m1);

			int new_width = m_window->GetWidth();
			int new_height = m_window->GetHeight();

			if (new_width > 0 && new_height > 0) {
				m_renderer->GetVulkanEngine()->re_init_swapchains(new_width, new_height);
			}
		}

		if (m_threadMode == ThreadMode::MULTI_THREAD) {
			Input(delta);
		}
		else {
			if (m_window->ShouldResizeRenderer()) {
				m_renderer->Resize();

				FontResources::DoRebuild();
				m_window->HandleResizeRenderer();
			}

			if (FontResources::ShouldRebuild()) {
				FontResources::PreloadFontCaches();
			}

			Input(delta);

			Update(delta);

			if (static_cast<int>(m_currentFade) != static_cast<int>(m_targetFade)) {
				float increment = (static_cast<float>(delta) * 5.0f) * 100.0f;

				// compare it using epsilon
				if (std::abs(m_currentFade - m_targetFade) < FLT_EPSILON) {
					m_currentFade = m_targetFade;
				}
				else {
					if (m_currentFade < m_targetFade) {
						m_currentFade = std::clamp(m_currentFade + increment, 0.0f, 100.0f);
					}
					else {
						m_currentFade = std::clamp(m_currentFade - increment, 0.0f, 100.0f);
					}
				}
			}

			if (!m_minimized) {
				if (m_renderer->BeginRender()) {
					Render(delta);
					MsgBox::Draw();

					if (static_cast<int>(m_currentFade) != 0) {
						auto drawList = ImGui::GetForegroundDrawList();
						float a = static_cast<int>(m_currentFade) / 100.0f;

						drawList->AddRectFilled(ImVec2(0, 0), MathUtil::ScaleVec2(m_window->GetBufferWidth(), m_window->GetBufferHeight()), IM_COL32(0, 0, 0, a * 255));
					}

					m_renderer->EndRender();
				}
			}
		}
	}, false);

	while (m_running) {
		mLocalThread.Update();
	}

	mAudioThread.Stop();
	mRenderThread.Stop();

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

void Game::SetFrameLimitMode(FrameLimitMode mode) {
	m_frameLimitMode = mode;
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

void Game::SetFullscreen(bool fullscreen) {
	m_fullscreen = fullscreen;
}

GameThread* Game::GetRenderThread() {
	return &mRenderThread;
}

GameThread* Game::GetMainThread() {
	return &mLocalThread;
}

void Game::DisplayFade(int transparency) {
	m_targetFade = static_cast<float>(transparency);
}

float Game::GetDisplayFade() {
	return m_currentFade;
}

ThreadMode Game::GetThreadMode() {
	return m_threadMode;
}

void Game::Update(double deltaTime) {
	
}

void Game::Render(double deltaTime) {
	
}

void Game::Input(double deltaTime) {
	
}

void Game::Mouse(double deltaTime) {

}
