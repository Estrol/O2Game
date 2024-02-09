/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __TWEEEN_H_
#define __TWEEEN_H_
#include "UDim2.h"
#include <string>

enum class TweenType {
    Linear,
    Quadratic,
    Cubic,
    Quartic,
    Quintic,
    Sinusoidal,
    Exponential,
    Circular,
    Elastic,
    Back,
    Bounce
};

class Tween
{
public:
    Tween() = default;
    Tween(UDim2 start, UDim2 end, float duration, TweenType type = TweenType::Linear);

    UDim2 Update(float time);
    void  Reset();

    bool IsFinished() const;

private:
    UDim2 Start;
    UDim2 End;

    float     Duration;
    float     Time;
    TweenType Type;
};
#endif