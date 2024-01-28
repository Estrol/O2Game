#include <iostream>

#include "Audio/AudioManager.h"
#include "Data/Imgui/imgui_impl_sdl2.h"
#include "Data/Imgui/imgui_impl_sdlrenderer2.h"
#include "Fonts/FontResources.h"
#include "Game.h"
#include "Imgui/ImguiUtil.h"
#include "Logs/Console.h"
#include "MsgBox.h"
#include "Rendering/Vulkan/VulkanEngine.h"
#include "Rendering/Window.h"
#include "Texture/MathUtils.h"
#include <Logs.h>
#include <SDL2/SDL_image.h>

constexpr auto kInputDefaultRate = 1000.0;
constexpr auto kMenuDefaultRate = 60.0;
constexpr auto kAudioDefaultRate = 24.0;

namespace {
    bool InitSDL()
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
            return false;
        }

        return IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) != 0;
    }

    void DeInitSDL()
    {
        SDL_Quit();
        IMG_Quit();
    }

    thread_local double curTick = 0.0;
    thread_local double lastTick = 0.0;

    double FrameLimit(double MaxFrameRate)
    {
        double newTick = SDL_GetTicks();
        double targetTick = lastTick + 1000.0 / MaxFrameRate;

        // If the frame rate is too high, wait to avoid shaky animations
        if (newTick < targetTick) {
            double delayTicks = targetTick - newTick;
            SDL_Delay(static_cast<Uint32>(delayTicks));
            newTick += delayTicks;
        }

        double delta = (newTick - curTick) / 1000.0;
        curTick = newTick;

        // Update lastTick for the next frame
        lastTick = curTick;

        return delta;
    }
} // namespace

Game::Game()
{
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

Game::~Game()
{
    if (m_running) {
        Stop();

        // Properly wait for threads to finish using synchronization
        if (m_notify) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_conditionVariable.wait(lock, [this] { return !m_notify; });
        }
    }

    SceneManager::Release();
    AudioManager::Release();
    InputManager::Release();
    Renderer::Release();
    GameWindow::Release();

    DeInitSDL();
}

bool Game::Init()
{
    if (!InitSDL()) {
        MsgBox::ShowOut("SDL Failed to Initialize", "EstEngine Error");

        return false;
    }

    m_window = GameWindow::GetInstance();

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

    m_renderer = Renderer::GetInstance();
    if (!m_renderer->Create(m_renderMode, m_window)) {
        return false;
    }

    if (m_renderMode == RendererMode::OPENGL) {
        m_threadMode = ThreadMode::SINGLE_THREAD; // OpenGL doesnt support multithreading
    }

    m_inputManager = InputManager::GetInstance();

    m_sceneManager = SceneManager::GetInstance();
    m_sceneManager->SetParent(this);

    if (!AudioManager::GetInstance()->Init(m_window)) {
        return false;
    }

    FontResources::PreloadFontCaches();
    m_currentFade = 0;
    m_targetFade = 0;

    return true;
}

void Game::Run()
{
    m_running = true;
    m_notify = true;
    m_frameLimit = m_frameLimit == 0 ? kMenuDefaultRate : m_frameLimit;
    m_frameLimitMode = FrameLimitMode::MENU;

    mAudioThread.Run([&] {
        double delta = FrameLimit(kAudioDefaultRate);
        AudioManager::GetInstance()->Update(delta);
    },
                     true);

    std::mutex m1, m2;

    int    frameWithoutSwapchain = 0;
    int    frames = 0;
    double time = 0;

    mRenderThread.Run([&] {
        if (m_threadMode == ThreadMode::MULTI_THREAD) {
            double delta = 0;

            switch (m_frameLimitMode) {
                case FrameLimitMode::GAME:
                {
                    delta = FrameLimit(m_frameLimit);
                    break;
                }

                case FrameLimitMode::MENU:
                {
                    delta = FrameLimit(kMenuDefaultRate);
                    break;
                }
            }

            CheckFont();

            Update(delta);

            UpdateFade(delta);

            if (!m_minimized && m_renderer->BeginRender()) {
                if (frameWithoutSwapchain > 0) {
                    Logs::Puts("[Game] Game iterate %d times without swap chain", frameWithoutSwapchain);
                    frameWithoutSwapchain = 0;
                }

                std::lock_guard<std::mutex> lock(m1);

                Render(delta);
                MsgBox::Draw();
                DrawFade(delta);

                DrawConsole();
                m_renderer->EndRender();

                frames++;
            } else {
                frameWithoutSwapchain++;
            }
        } else {
            FrameLimit(15.0f);
        }
    },
                      true);

    m_inputManager->ListenKeyEvent([&](const KeyState &state) {
        if (state.type == KeyEventType::KEY_DOWN) {
            m_sceneManager->OnKeyDown(state);
        } else {
            m_sceneManager->OnKeyUp(state);
        }
    });

    m_inputManager->ListenMouseEvent([&](const MouseState &state) {
        if (state.isDown) {
            m_sceneManager->OnMouseDown(state);
        } else {
            m_sceneManager->OnMouseUp(state);
        }
    });

    m_minimized = false;
    mLocalThread.Run([&] {
        double delta = 0;
        switch (m_frameLimitMode) {
            case FrameLimitMode::GAME:
            {
                delta = FrameLimit(m_threadMode == ThreadMode::MULTI_THREAD ? kInputDefaultRate : m_frameLimit);
                break;
            }

            case FrameLimitMode::MENU:
            {
                delta = FrameLimit(kMenuDefaultRate);
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

        if (m_renderer->IsVulkan()) {
            m_renderer->ReInitVulkan();
        }

        if (m_threadMode == ThreadMode::MULTI_THREAD) {
            Input(delta);
        } else {
            CheckFont();

            Input(delta);

            Update(delta);

            UpdateFade(delta);

            if (!m_minimized && m_renderer->BeginRender()) {
                if (frameWithoutSwapchain > 0) {
                    Logs::Puts("[Game] Game iterate %d times without swap chain", frameWithoutSwapchain);
                    frameWithoutSwapchain = 0;
                }

                Render(delta);
                MsgBox::Draw();
                DrawFade(delta);

                DrawConsole();
                m_renderer->EndRender();

                frames++;
            } else {
                frameWithoutSwapchain++;
            }
        }

        if ((time += delta) > 1.0) {
            std::string fps = "FPS: " + std::to_string(frames);
            m_window->SetWindowSubTitle(fps);

            frames = 0;
            time = 0;
        }
    },
                     false);

    while (m_running) {
        mLocalThread.Update();
    }

    mAudioThread.Stop();
    mRenderThread.Stop();

    m_notify = false;
}

void Game::DrawFade(double delta)
{
    if (static_cast<int>(m_currentFade) != 0) {
        auto  drawList = ImGui::GetForegroundDrawList();
        float a = m_currentFade / 100.0f;

        drawList->AddRectFilled(ImVec2(0, 0), MathUtil::ScaleVec2(m_window->GetBufferWidth(), m_window->GetBufferHeight()), IM_COL32(0, 0, 0, a * 255));
    }
}

void Game::DrawConsole()
{
    Console::Draw();
}

void Game::UpdateFade(double delta)
{
    if (static_cast<int>(m_currentFade) != static_cast<int>(m_targetFade)) {
        float increment = (static_cast<float>(delta) * 5.0f) * 100.0f;

        // compare it using epsilon
        if (std::abs(m_currentFade - m_targetFade) < FLT_EPSILON) {
            m_currentFade = m_targetFade;
        } else {
            if (m_currentFade < m_targetFade) {
                m_currentFade = std::clamp(m_currentFade + increment, 0.0f, 100.0f);
            } else {
                m_currentFade = std::clamp(m_currentFade - increment, 0.0f, 100.0f);
            }
        }
    }
}

void Game::CheckFont()
{
    if (m_window->ShouldResizeRenderer()) {
        m_renderer->Resize();

        FontResources::DoRebuild();
        m_window->HandleResizeRenderer();
    }

    if (FontResources::ShouldRebuild()) {
        FontResources::PreloadFontCaches();
    }
}

void Game::Stop()
{
    if (m_running) {
        m_running = false;

        // Notify condition variable to indicate that the threads should finish
        {
            std::lock_guard<std::mutex> lock(m_mutex); // TODO: Fix game does not properly exit while crash (if not just revert back to SDL_Delay Sleep)
            m_notify = true; // This should be true
        }
        m_conditionVariable.notify_one();
    }
}

void Game::SetThreadMode(ThreadMode mode)
{
    m_threadMode = mode;
}

void Game::SetRenderMode(RendererMode mode)
{
    m_renderMode = mode;
}

void Game::SetFrameLimitMode(FrameLimitMode mode)
{
    m_frameLimitMode = mode;
}

void Game::SetFramelimit(double frameRate)
{
    m_frameLimit = frameRate;
}

void Game::SetBufferSize(int width, int height)
{
    m_bufferWidth = width;
    m_bufferHeight = height;
}

void Game::SetWindowSize(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
}

void Game::SetFullscreen(bool fullscreen)
{
    m_fullscreen = fullscreen;
}

GameThread *Game::GetRenderThread()
{
    return &mRenderThread;
}

GameThread *Game::GetMainThread()
{
    return &mLocalThread;
}

void Game::DisplayFade(int transparency)
{
    m_targetFade = static_cast<float>(transparency);
}

float Game::GetDisplayFade()
{
    return m_currentFade;
}

ThreadMode Game::GetThreadMode()
{
    return m_threadMode;
}

void Game::Update(double deltaTime)
{
}

void Game::Render(double deltaTime)
{
}

void Game::Input(double deltaTime)
{
}

void Game::Mouse(double deltaTime)
{
}
