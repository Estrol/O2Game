/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __GAME_H__
#define __GAME_H__
#include <Graphics/NativeWindow.h>
#include <Graphics/Renderer.h>
#include <Inputs/Keys.h>
#include <Math/Vector2.h>
#include <Threads/AccumulatedTimeWatch.h>
#include <Threads/Thread.h>
#include <UI/Text.h>
#include <memory>
#include <string>

enum class ThreadMode {
    Single,
    Multi
};

struct RunInfo
{
    std::string                  title;
    Vector2                      resolution;
    Vector2                      buffer_resolution;
    bool                         fullscreen;
    ThreadMode                   threadMode;
    Graphics::TextureSamplerInfo samplerInfo;
    Graphics::API                graphics;

    float renderFrameRate;
    bool  renderVsync;
    float inputFrameRate;
    float fixedFrameRate;
};

class Game
{
public:
    Game();
    virtual ~Game();

    void Run(RunInfo info);

protected:
    /**
     * Called when the game is initialized
     * Used for loading resources, always use this function to initialize resources to avoid threading problems with the renderer
     * Thread: Render
     */
    virtual void OnLoad();

    /**
     * Called when the game is deinitialized
     * Used for unloading resources, always use this function
     * Thread: Render
     */
    virtual void OnUnload();

    /*
     * Called when the game should update the input
     * You might want to use this to update the input state like keyboard and mouse
     * Thread: Window
     */
    virtual void OnInput(double delta);

    /**
     * Called when the game should update
     * Thread: Render
     */
    virtual void OnUpdate(double delta);

    /**
     * Called when the game should fixed update
     * Thread: Render
     */
    virtual void OnFixedUpdate(double fixedStep);

    /**
     * Called when the game should draw
     * it might not be called every frame like window minimized
     * Thread: Render
     */
    virtual void OnDraw(double delta);

    /**
     * Called when a input in keyboard pressed
     * Thread: Window
     */
    virtual void OnKeyDown(const Inputs::State &state);

    /**
     * Called when a input in keyboard released
     * Thread: Window
     */
    virtual void OnKeyUp(const Inputs::State &state);

private:
    AccurateTimeWatch m_FixedUpdateTimer;
    Thread            m_InputThread;
    Thread            m_DrawThread;
};

#endif // __GAME_H__