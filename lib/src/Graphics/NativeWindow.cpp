/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Exceptions/EstException.h>
#include <Graphics/NativeWindow.h>
#include <Graphics/Renderer.h>

using namespace Graphics;

NativeWindow *NativeWindow::s_Instance = nullptr;

NativeWindow *NativeWindow::Get()
{
    if (s_Instance == nullptr) {
        s_Instance = new NativeWindow();
    }
    return s_Instance;
}

void NativeWindow::Destroy()
{
    if (s_Instance != nullptr) {
        delete s_Instance;
        s_Instance = nullptr;
    }
}

void NativeWindow::Init(WindowInfo info)
{
    Uint32 flags = 0;
    if (info.fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    switch (info.graphics) {
        case API::OpenGL:
        {
            flags |= SDL_WINDOW_OPENGL;
            break;
        }

        case API::Vulkan:
        {
            flags |= SDL_WINDOW_VULKAN;
            break;
        }

        default:
        {
            throw Exceptions::EstException("Unknown graphics API");
        }
    }

    auto Window = SDL_CreateWindow(info.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, info.windowSize.Width, info.windowSize.Height, flags);
    if (Window == nullptr) {
        throw Exceptions::EstException("Failed to initialize window");
    }

    m_Window = std::unique_ptr<SDL_Window, SDLWindowSmartDeallocator>(Window);
    m_WindowRect = info.windowSize;
    m_BufferRect = info.bufferSize;
}

NativeWindow::~NativeWindow()
{
}

Rect NativeWindow::GetWindowSize()
{
    return m_WindowRect;
}

Rect NativeWindow::GetBufferSize()
{
    return m_BufferRect;
}

Vector2 NativeWindow::GetWindowScale()
{
    float scaleX = GetWindowSize().Width / (float)GetBufferSize().Width;
    float scaleY = GetWindowSize().Height / (float)GetBufferSize().Height;

    return {
        scaleX, scaleY
    };
}

void NativeWindow::SetWindowSize(Rect size)
{
    SDL_SetWindowSize(m_Window.get(), size.Width, size.Height);
    m_WindowRect = size;
}

SDL_Window *NativeWindow::GetWindow()
{
    return m_Window.get();
}

void NativeWindow::PumpEvents()
{
    bool ExitRequested = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                ExitRequested = true;
                break;
        }

        for (auto &callback : m_Callbacks) {
            callback(event);
        }
    }

    if (ExitRequested) {
        for (auto &callback : m_ExitCallbacks) {
            callback(ExitRequested);

            if (!ExitRequested) {
                break;
            }
        }

        if (ExitRequested) {
            m_ShouldExit = true;
        }
    }
}

void NativeWindow::AddSDLCallback(std::function<void(SDL_Event &)> callback)
{
    m_Callbacks.push_back(callback);
}

void NativeWindow::AddExitCallback(std::function<void(bool &)> callback)
{
    m_ExitCallbacks.push_back(callback);
}

bool NativeWindow::ShouldExit()
{
    return m_ShouldExit;
}