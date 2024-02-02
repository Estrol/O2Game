/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "./Game/O2Game.h"

#if defined(_WIN32) && defined(_MSC_VER)
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main(int argv, char **argc)
{
    O2Game game;
    game.Run(argv, argc);

    return 0;
}