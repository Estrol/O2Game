/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __INPUTMANAGER_H_
#define __INPUTMANAGER_H_

#define SDL_MAIN_HANDLED
#include "Keys.h"
#include <Math/Vector2.h>
#include <SDL2/SDL.h>
#include <functional>
#include <map>

namespace Inputs {
    class Manager
    {
    public:
        void Update(SDL_Event &event);

        bool    IsKeyDown(Keys key);
        bool    IsMouseDown(Mouse button);
        Vector2 GetMousePosition();

        void ListenOnKeyEvent(std::function<void(const Inputs::State &state)> callback);
        void ListenOnMouseEvent(std::function<void(const Inputs::State &state)> callback);

        static Manager *Get();
        static void     Destroy();

    private:
        Manager();
        ~Manager();

        Vector2 MousePosition;

        void HandleKeyEvent(SDL_Event &event);
        void HandleMouseEvent(SDL_Event &event);
        void HandleMouseMotionEvent(SDL_Event &event);

        std::map<Keys, bool>  KeyStates;
        std::map<Mouse, bool> MouseStates;

        std::function<void(const Inputs::State &state)> OnKeyEvent;
        std::function<void(const Inputs::State &state)> OnMouseEvent;

        static Manager *Instance;
    };
} // namespace Inputs

#endif