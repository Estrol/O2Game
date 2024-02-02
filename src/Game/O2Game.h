/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include <Game.h>

class O2Game : public Game
{
public:
    O2Game();
    ~O2Game();

    void Run(int argv, char **argc);

protected:
    void OnLoad() override;
    void OnUnload() override;

    void OnInput(double delta) override;
    void OnUpdate(double delta) override;
    void OnDraw(double delta) override;
};