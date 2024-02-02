/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __RECTANGLE_H_
#define __RECTANGLE_H_

#include "UIBase.h"

namespace UI {
    class Rectangle : public Base
    {
    public:
        Rectangle();

    protected:
        void OnDraw() override;
    };
} // namespace UI

#endif