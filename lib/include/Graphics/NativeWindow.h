/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __NATIVE_WINDOW_H__
#define __NATIVE_WINDOW_H__

#include "Utils/Rect.h"
#include <Math/Vector2.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

namespace Graphics {
    struct SDLWindowSmartDeallocator
    {
        inline void operator()(SDL_Window *window) const
        {
            SDL_DestroyWindow(window);
        }
    };

    enum class API;

    struct WindowInfo
    {
        std::string title;
        Rect        windowSize;
        Rect        bufferSize;
        bool        fullscreen;
        API         graphics;
    };

    class NativeWindow
    {
    public:
        void Init(WindowInfo info);

        void PumpEvents();
        bool ShouldExit();

        Rect    GetWindowSize();
        Rect    GetBufferSize();
        Vector2 GetWindowScale();
        void    SetWindowSize(Rect size);

        SDL_Window *GetWindow();

        void AddSDLCallback(std::function<void(SDL_Event &)> callback);
        void AddExitCallback(std::function<void(bool &)> callback);

        static NativeWindow *Get();
        static void          Destroy();

    private:
        ~NativeWindow();

        static NativeWindow *s_Instance;

        bool                                                   m_ShouldExit = false;
        Rect                                                   m_WindowRect;
        Rect                                                   m_BufferRect;
        std::vector<std::function<void(SDL_Event &)>>          m_Callbacks;
        std::vector<std::function<void(bool &)>>               m_ExitCallbacks;
        std::unique_ptr<SDL_Window, SDLWindowSmartDeallocator> m_Window;
    };
} // namespace Graphics

#endif // __NATIVE_WINDOW_H__