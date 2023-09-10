@echo off

if "%VCPKG_ROOT%" == "" (
    echo "VCPKG_ROOT is not set. Please set VCPKG_ROOT to the root of your vcpkg installation."
    exit /b 1
)

if "%VCPKG_DEFAULT_TRIPLET%" == "" (
    set VCPKG_DEFAULT_TRIPLET=x64-windows
)

REM create directory ./out/build/x64-debug
if not exist "./build" mkdir "./build"

cd "./build"

cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=%VCPKG_DEFAULT_TRIPLET% -DCMAKE_BUILD_TYPE=Release