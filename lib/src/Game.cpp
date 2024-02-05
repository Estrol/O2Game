/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "Fonts/FontManager.h"
#include <Audio/AudioEngine.h>
#include <Exceptions/EstException.h>
#include <Game.h>
#include <Graphics/GraphicsBackendBase.h>
#include <Graphics/NativeWindow.h>
#include <Inputs/InputManager.h>
#include <MsgBox.h>
#include <Screens/ScreenManager.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>

Game::Game()
{
}

Game::~Game()
{
}

void Game::Run(RunInfo info)
{
    int result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    if (result != 0) {
        std::cout << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return;
    }

    try {
        Graphics::WindowInfo creationInfo = {};
        creationInfo.windowSize = { 0, 0, (int)info.resolution.X, (int)info.resolution.Y };
        creationInfo.bufferSize = { 0, 0, (int)info.buffer_resolution.X, (int)info.buffer_resolution.Y };
        creationInfo.title = info.title;
        creationInfo.fullscreen = info.fullscreen;
        creationInfo.graphics = info.graphics;

        auto window = Graphics::NativeWindow::Get();
        window->Init(creationInfo);

        auto engine = Audio::Engine::Get();
        engine->Init();

        auto inputs = Inputs::Manager::Get();
        window->AddSDLCallback([=](SDL_Event &event) {
            inputs->Update(event);
        });

        inputs->ListenOnKeyEvent([=](const Inputs::State &state) {
            if (state.Type == Inputs::Type::KeyDown) {
                OnKeyDown(state);
            } else if (state.Type == Inputs::Type::KeyUp) {
                OnKeyUp(state);
            }
        });

        m_FixedUpdateTimer = AccurateTimeWatch(1.0f / info.fixedFrameRate);

        auto renderer = Graphics::Renderer::Get();
        auto scenemanager = Screens::Manager::Get();
        auto fontmanager = Fonts::FontManager::Get();

        if (info.threadMode == ThreadMode::Multi) {
            auto oninit = [=]() {
                scenemanager->Init(this);
                renderer->Init(info.graphics, info.samplerInfo);
                renderer->SetVSync(info.renderVsync);
                OnLoad();
            };

            auto onshutdown = [=]() {
                OnUnload();
                Screens::Manager::Destroy();
                Fonts::FontManager::Destroy();
                Graphics::Renderer::Destroy();
            };

            auto onupdate = [=](double delta) {
                m_FixedUpdateTimer.Update(delta);

                while (m_FixedUpdateTimer.Tick()) {
                    OnFixedUpdate(m_FixedUpdateTimer.GetFixedStep());
                }

                OnUpdate(delta);

                bool shouldDraw = renderer->BeginFrame();
                if (shouldDraw) {
                    OnDraw(delta);
                    renderer->EndFrame();
                }
            };

            m_DrawThread = Thread(oninit, onupdate, onshutdown, info.renderFrameRate);

            auto oninput = [=](double delta) {
                window->PumpEvents();

                OnInput(delta);
            };

            m_InputThread = Thread(oninput, info.inputFrameRate);

            m_DrawThread.Start();

            while (!window->ShouldExit()) {
                m_InputThread.Tick();

                auto drawException = m_DrawThread.GetException();
                if (drawException) {
                    std::rethrow_exception(drawException);
                }
            }

            m_DrawThread.Stop();
        } else {
            renderer->Init(info.graphics, info.samplerInfo);
            renderer->SetVSync(info.renderVsync);
            scenemanager->Init(this);
            OnLoad();

            auto oninput = [=](double delta) {
                window->PumpEvents();

                m_FixedUpdateTimer.Update(delta);

                while (m_FixedUpdateTimer.Tick()) {
                    OnFixedUpdate(m_FixedUpdateTimer.GetFixedStep());
                }

                OnInput(delta);
                OnUpdate(delta);

                bool shouldDraw = renderer->BeginFrame();
                if (shouldDraw) {
                    OnDraw(delta);
                    renderer->EndFrame();
                }
            };

            auto onfixed = [=](double fixedStep) {
                OnFixedUpdate(fixedStep);
            };

            m_InputThread = Thread(oninput, info.renderFrameRate);

            while (!window->ShouldExit()) {
                m_InputThread.Tick();
            }

            OnUnload();

            Screens::Manager::Destroy();
            Fonts::FontManager::Destroy();
            Graphics::Renderer::Destroy();
        }

        Graphics::NativeWindow::Destroy();
        Inputs::Manager::Destroy();

    } catch (Exceptions::EstException &e) {
        MsgBox::Show("Error", e.what(), MsgBox::Type::Ok, MsgBox::Flags::Error);
    }

    SDL_Quit();
}

void Game::OnLoad()
{
}

void Game::OnUnload()
{
}

void Game::OnInput(double delta)
{
    auto scenemanager = Screens::Manager::Get();
    scenemanager->Input(delta);
}

void Game::OnUpdate(double delta)
{
    auto scenemanager = Screens::Manager::Get();
    scenemanager->Update(delta);
}

void Game::OnFixedUpdate(double fixedStep)
{
    auto scenemanager = Screens::Manager::Get();
    scenemanager->FixedUpdate(fixedStep);
}

void Game::OnDraw(double delta)
{
    auto scenemanager = Screens::Manager::Get();
    scenemanager->Draw(delta);
}

void Game::OnKeyDown(const Inputs::State &state)
{
    auto scenemanager = Screens::Manager::Get();
    scenemanager->OnKeyDown(state);
}

void Game::OnKeyUp(const Inputs::State &state)
{
    auto scenemanager = Screens::Manager::Get();
    scenemanager->OnKeyUp(state);
}