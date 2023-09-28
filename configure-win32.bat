@echo off

if "%VCPKG_ROOT%" == "" (
    echo "VCPKG_ROOT is not set. Please set VCPKG_ROOT to the root of your vcpkg installation."
    exit /b 1
)

if "%1" == "Debug" (
    set PRESET=x64-debug
) else (
    set PRESET=x64-release
)

cmake . --preset=%PRESET%