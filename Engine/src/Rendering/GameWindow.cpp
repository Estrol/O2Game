#include "../Data/Imgui/imgui_impl_sdl2.h"
#include "MsgBox.h"
#include "Rendering/Renderer.h"
#include "Rendering/Window.h"
#include <SDL2/SDL_syswm.h>
#include <codecvt>

GameWindow::GameWindow()
{
    m_window = nullptr;
    m_resizeRenderer = false;

    m_width = 0;
    m_height = 0;
    m_bufferWidth = 0;
    m_bufferHeight = 0;

    m_mainTitle = "";
    m_subTitle = "";
}

GameWindow::~GameWindow()
{
    if (m_window != nullptr) {
        Destroy();
    }
}

GameWindow *GameWindow::s_instance = nullptr;

bool GameWindow::Create(RendererMode mode, std::string title, int width, int height, int bufferWidth, int bufferHeight)
{
    if (m_window != nullptr) {
        return false;
    }

    m_width = width;
    m_height = height;
    m_bufferWidth = bufferWidth;
    m_bufferHeight = bufferHeight;
    m_mainTitle = title;

    uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    if (mode == RendererMode::OPENGL) {
        flags |= SDL_WINDOW_OPENGL;
    }
    if (mode == RendererMode::VULKAN) {
        flags |= SDL_WINDOW_VULKAN;
    }

    // check if width and height are same in current display
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);

    bool is_fullscreen = false;
    if (width == dm.w && height == dm.h) {
        is_fullscreen = true;
    }

    m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

    if (m_window == nullptr) {
        MsgBox::ShowOut(SDL_GetError(), "EstEngine Error");

        return false;
    }

    if (is_fullscreen) {
        SDL_SetWindowBordered(m_window, SDL_FALSE);
    }

    return true;
}

bool GameWindow::Destroy()
{
    if (m_window == nullptr) {
        return true;
    }

    SDL_DestroyWindow(m_window);
    m_window = nullptr;

    return true;
}

void GameWindow::ResizeWindow(int width, int height)
{
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);

    bool is_fullscreen = false;
    if (width == dm.w && height == dm.h) {
        is_fullscreen = true;
    }

    int lastWidth = m_width;
    int lastHeight = m_height;

    m_width = width;
    m_height = height;

    SDL_SetWindowSize(m_window, width, height);
    SDL_SetWindowFullscreen(m_window, is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    SDL_SetWindowBordered(m_window, is_fullscreen ? SDL_FALSE : SDL_TRUE);

    if (!is_fullscreen) {
        SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    m_resizeRenderer = lastWidth != m_width || lastHeight != m_height;
}

void GameWindow::ResizeBuffer(int width, int height)
{
    int lastWidth = m_bufferWidth;
    int lastHeight = m_bufferHeight;

    m_bufferWidth = width;
    m_bufferHeight = height;
}

bool GameWindow::ShouldResizeRenderer()
{
    return m_resizeRenderer;
}

void GameWindow::HandleResizeRenderer()
{
    m_resizeRenderer = false;
}

SDL_Window *GameWindow::GetWindow() const
{
    return m_window;
}

void GameWindow::SetWindowTitle(std::string &title)
{
    m_mainTitle = title;

    if (m_subTitle.size()) {
        title += " - " + m_subTitle;
    }

    SDL_SetWindowTitle(m_window, title.c_str());
}

void GameWindow::SetWindowSubTitle(std::string &subTitle)
{
    m_subTitle = subTitle;
    std::string title = m_mainTitle;

    if (m_mainTitle.size()) {
        title += " - " + subTitle;
    }

    SDL_SetWindowTitle(m_window, title.c_str());
}

int GameWindow::GetWidth() const
{
    return m_width;
}

int GameWindow::GetHeight() const
{
    return m_height;
}

int GameWindow::GetBufferWidth() const
{
    return m_bufferWidth;
}

int GameWindow::GetBufferHeight() const
{
    return m_bufferHeight;
}

float GameWindow::GetWidthScale()
{
    return static_cast<float>(m_width) / static_cast<float>(m_bufferWidth);
}

float GameWindow::GetHeightScale()
{
    return static_cast<float>(m_height) / static_cast<float>(m_bufferHeight);
}

void GameWindow::SetScaleOutput(bool value)
{
    m_scaleOutput = value;
}

bool GameWindow::IsScaleOutput()
{
    return m_scaleOutput;
}

GameWindow *GameWindow::GetInstance()
{
    if (s_instance == nullptr) {
        s_instance = new GameWindow();
    }

    return s_instance;
}

void GameWindow::Release()
{
    if (s_instance != nullptr) {
        delete s_instance;
        s_instance = nullptr;
    }
}
