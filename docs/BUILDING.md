# Building Unnamed O2 Clone Game
Currently only 2 platform are supported: Windows and Linux.

## Table of Contents
- [Requirements](#requirements)
    - [Windows](#windows)
    - [Linux](#linux)
- [Building](#building)
- [Tested Platform](#tested-platform)

## Requirements
To able build this project you need to have installed:
### Windows
- Visual Studio 2022
- CMake 3.21.3
- Vulkan SDK (1.2.198.1)
- Vcpkg

### Linux
- GCC 10.3.0
- CMake 3.21.3
- Vulkan SDK (1.2.198.1)
- Vcpkg

Make sure you have the requirements installed and configured correctly.
- set `VCPKG_ROOT` environment variable to your vcpkg root directory
- If you using Windows then set `VULKAN_SDK` environment variable to your Vulkan SDK directory
- It's recommended to use Vulkan 1.2.198.1 to support most system

## Building
To build the project, follow these steps:

- Clone the repository including submodules:
    - `git clone --recurse-submodules https://github.com/Estrol/o2game.git`
- Create build directory:
    - If windows: `./configure-win32.bat Debug` or `./configure-win32.bat Release`
    - If linux: `./configure-linux.sh Debug` or `./configure-linux.sh Release`
- Build the project:
    - cd to `build` directory
    - Windows: `cmake --build . --config Debug` or `cmake --build . --config Release`
    - Linux: `cmake --build . --config Debug` or `cmake --build . --config Release`
- Run the game:
    - cd to `build` directory
    - Windows: cd to `Game/Debug/` or `Game/Release/` and run `Game.exe`
    - Linux: cd to `Game` and run `./Game`

Note: to switch between `Release` and `Debug` build, you need to delete the `build` directory and reconfigure it again with the new build type.

## Tested Platform
The game is tested on these platform:

| Platform | Compiler | Status |
|----------|----------|--------|
| Windows 11 | Visual Studio 2022 | Working |
| Windows 11 | Mingw-w64 8.1.0 | Broken |
| Ubuntu 22.04 | GCC 11.1.0 | Working |
| WSL Ubuntu 20.04 | GCC 10.3.0 | Working |

Developer note: I ([Estrol](https://github.com/Estrol)) only tested on Windows 11 and Ubuntu 20.04. If you have tested on other platform, please let me know.