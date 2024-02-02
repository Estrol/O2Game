/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Screens/Base.h>
using namespace Screens;

void Base::Update(double delta)
{
    (void)delta;
}

void Base::Draw(double delta)
{
    (void)delta;
}

void Base::Input(double delta)
{
    (void)delta;
}

void Base::FixedUpdate(double fixedDelta)
{
    (void)fixedDelta;
}

void Base::OnKeyDown(const Inputs::State &state)
{
    (void)state;
}

void Base::OnKeyUp(const Inputs::State &state)
{
    (void)state;
}

bool Base::Attach()
{
    return false;
}

bool Base::Detach()
{
    return false;
}