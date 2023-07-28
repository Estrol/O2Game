# Unnamed O2 Clone, The Game
This is experimental (currently Windows-only) software for playing rhythm game's chart.

Written in C++ with SDL (+ imgui for it's GUI elements)

# Features
- BPM Changes with negative BPM.
- OJN, BMS, and osu!mania files.
- Beat-based judgement.
- Audio sample non-pitched.

# Currently supported game format
- O2 (.ojn, .ojm) (Encrypted OJN supported)
- BMS (.bms, .bme, .bml) (2 Player or PMS is not supported)
- osu!mania (.osu)

# TODO List
- Implement song selection list (DONE).
- Implement SDL Based rendering (DONE).
- Replace any Windows only code to STL implementation.
- Crossplatform support.
- Implement more UI.
- Probably a lots.

# Project directory
- Engine, Game engine that powered this game.
- Game, Game code for this game logic.

# Compiling
### Requirements
- vcpkg
- Visual Studio 2022 with C++20 Desktop Development or GCC 11.2.0
- BASS Library from https://www.un4seen.com/

### Installing external library
- Install vcpkg first
- then add VCPKG_ROOT in your environment variable to your vcpkg directory
- and let build system handle the rest.

### Compiling
- Open this project in Visual Studio Code with CMake Tools extension or Visual Studio 2022 with C\+\+20 Desktop Development and Linux Development with C++.
- Select your build target (x64-Debug or x64-Release)
- Build it.

# Crossplatform
Currently the CMakeLists not properly configured for crossplatform, but you can try to compile it with CMake. \
Make sure you contribute back when you successfully compiled it.

# License
MIT License

Copyright (c) 2023 Estrol Mendex

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.