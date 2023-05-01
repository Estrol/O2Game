# Unnamed O2 Clone Game (Forked from Estrol/O2Game)
This is experimental software for playing rhythm games.

Written in C++ with SDL + BASS + DirectX11 (and Vulkan through DXVK)

# Features
- BPM Changes with negative BPM.
- OJN, BMS, and osu!mania files.
- Beat-based judgement.
- Audio sample non-pitched.

# TODO List
- Implement song selection list.
- Implement more UI.
- Add Text rendering into Game Engine.
- Probably a lots.

# Project directory
- Engine, Game engine that powered this game.
- Game, Game code for this game logic.

# Compiling
### Requirements
- vcpkg
- Visual Studio 2022 with CXX20
- BASS Library from https://www.un4seen.com/

### Installing external library
- Install DirectXTK and libcurl from vcpkg.
- Put BASS x64 library (.lib) to Lib/x64

### Compiling
- Open project with visual studio
- Click Compile
- Copy BASS x64 DLL to build folder
- Copy skins folder from my build in discord server :troll:
- Run/Debug it
- To able use Vulkan, download and extract D3D11.dll and DXGI.dll into vulkan folder.

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