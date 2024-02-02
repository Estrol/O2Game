/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __SCREENBASE_H_
#define __SCREENBASE_H_

#include <Inputs/Keys.h>

namespace Screens {
    class Base
    {
    public:
        Base() = default;
        virtual ~Base() = default;

        virtual void Update(double delta);
        virtual void Draw(double delta);
        virtual void Input(double delta);
        virtual void FixedUpdate(double fixedDelta);

        virtual void OnKeyDown(const Inputs::State &state);
        virtual void OnKeyUp(const Inputs::State &state);

        virtual bool Attach();
        virtual bool Detach();
    };
} // namespace Screens

#endif