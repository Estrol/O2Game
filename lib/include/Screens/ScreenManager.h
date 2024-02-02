/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __SCREENSMANAGER_H_
#define __SCREENSMANAGER_H_

#include "Base.h"
#include <memory>
#include <unordered_map>

class Game;

namespace Screens {
    class Manager
    {
    public:
        void Init(Game *game);

        void Update(double delta);
        void Draw(double delta);
        void Input(double delta);
        void FixedUpdate(double fixedDelta);

        void OnKeyDown(const Inputs::State &state);
        void OnKeyUp(const Inputs::State &state);

        void AddScreen(uint32_t Id, Base *screen);
        void SetScreen(uint32_t Id);

        static Manager *Get();
        static void     Destroy();

    private:
        Manager() = default;
        ~Manager() = default;

        std::unordered_map<uint32_t, std::unique_ptr<Base>> m_Screens;

        Game *m_Game;
        Base *m_CurrentScreen;
        Base *m_NextScreen;
    };
} // namespace Screens

#endif