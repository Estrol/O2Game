#pragma once

namespace GameScene {
    constexpr int INVALID = 0;
    constexpr int INTRO = 1;
    constexpr int MAINMENU = 2;
    constexpr int SONGSELECT = 3;
    constexpr int LOADING = 4;
    constexpr int GAMEPLAY = 5;
    constexpr int RESULT = 6;

    constexpr int EDITOR = 7;
    constexpr int MULTIPLAYER = 8;
    constexpr int MULTIROOM = 9;

    constexpr int RELOAD = 99;
} // namespace GameScene

namespace GameOverlay {
    constexpr int INVALID = 0;
    constexpr int SETTINGS = 1;
} // namespace GameOverlay

namespace ResolutionWindowScene {
    constexpr int MAINMENU = 0;
    constexpr int SONGSELECT = 1;
    constexpr int GAMEPLAY = 2;
} // namespace ResolutionWindowScene