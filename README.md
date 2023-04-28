# Unnamed O2 Clone Game
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
MIT License.
