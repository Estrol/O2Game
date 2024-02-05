@echo off

IF NOT DEFINED VULKAN_SDK (
    echo "VULKAN_SDK is not defined. Please set it to the Vulkan SDK directory"
    exit /b 1
)

SET GLSLC=%VULKAN_SDK%\Bin\glslangValidator.exe

SET SHADER_DIR=lib\src\Graphics\Shaders
SET SHADER_GLSL_DIR=%SHADER_DIR%\GLSL

FOR %%f IN (%SHADER_GLSL_DIR%\*.vert %SHADER_GLSL_DIR%\*.frag) DO (
    %GLSLC% -V -o %SHADER_DIR%\SPV\%%~nf.spv.h --vn "__glsl_%%~nf" %%f
)