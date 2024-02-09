/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "./Game/Env.h"
#include "./Game/O2Game.h"

#if defined(_WIN32) && defined(_MSC_VER)
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#if defined(_WIN32)
#include "renderdoc.h"
#include <Windows.h>

void init_render_doc()
{
    RENDERDOC_API_1_1_2 *rdoc_api = NULL;

    // At init, on windows
    if (HMODULE mod = GetModuleHandleA("renderdoc.dll")) {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
        assert(ret == 1);
    }

    Env::SetPointer("RenderDoc", rdoc_api);
}
#endif

int main(int argv, char **argc)
{
    O2Game game;
    game.Run(argv, argc);

    return 0;
}