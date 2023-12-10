#pragma once
#include "Inputs/InputManager.h"
#include "Overlay.h"
#include "Rendering/Renderer.h"
#include "Rendering/Threading/GameThread.h"
#include "SceneManager.h"
#include <thread>

enum class ThreadMode {
    SINGLE_THREAD,
    MULTI_THREAD
};

enum class FrameLimitMode {
    MENU,
    GAME
};

class Game
{
#if _DEBUG
    const char SIGNATURE[25] = "Game";
#endif

public:
    Game();
    virtual ~Game();

    bool virtual Init();
    void virtual Run();
    void virtual Stop();

    void SetThreadMode(ThreadMode mode);
    void SetRenderMode(RendererMode mode);
    void SetFrameLimitMode(FrameLimitMode mode);
    void SetFramelimit(double frameRate);

    void SetBufferSize(int width, int height);
    void SetWindowSize(int width, int height);
    void SetFullscreen(bool fullscreen);

    GameThread *GetRenderThread();
    GameThread *GetMainThread();

    void       DisplayFade(int transparency);
    float      GetDisplayFade();
    ThreadMode GetThreadMode();

protected:
    virtual void Update(double deltaTime);
    virtual void Render(double deltaTime);
    virtual void Input(double deltaTime);
    virtual void Mouse(double deltaTime);

    GameWindow   *m_window;
    Renderer     *m_renderer;
    InputManager *m_inputManager;
    SceneManager *m_sceneManager;

private:
    void DrawConsole();
    void UpdateFade(double delta);
    void DrawFade(double delta);
    void CheckFont();

    double m_frameInterval;
    double m_imguiInterval;
    int    m_frameCount;
    int    m_currentFrameCount;
    float  m_currentFade;
    float  m_targetFade;

    bool m_running;
    bool m_notify;
    bool m_minimized;
    bool m_fullscreen;

    double m_frameLimit;

    int m_bufferWidth, m_bufferHeight;
    int m_windowWidth, m_windowHeight;

    ThreadMode     m_threadMode;
    RendererMode   m_renderMode;
    FrameLimitMode m_frameLimitMode;

    GameThread mRenderThread;
    GameThread mAudioThread;
    GameThread mLocalThread;
};