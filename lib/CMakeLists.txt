cmake_minimum_required(VERSION 3.8.0)

file(GLOB_RECURSE HEADERS "./include/*.h" "./include/*.hpp")

if (WIN32)
    add_compile_definitions("__ENABLE_D3D11__")
endif()

add_library(EstEngine STATIC ${HEADERS}
    "src/Game.cpp" 
    "src/MsgBox.cpp" 
    "src/Logs.cpp"
    "src/Configuration.cpp"

    # Main Graphics
    "src/Graphics/NativeWindow.cpp" 
    "src/Graphics/Renderer.cpp" 

    # Screens
    "src/Screens/Base.cpp" 
    "src/Screens/ScreenManager.cpp"

    # UI
    "src/UI/UIBase.cpp" 
    "src/UI/Rectangle.cpp" 
    "src/UI/Image.cpp" 
    "src/UI/Text.cpp" 
    "src/UI/TextBox.cpp"
    "src/UI/Dialog.cpp" 

    # Vulkan backends
    "src/Graphics/Backends/Vulkan/VulkanBackend.cpp" 
    "src/Graphics/Backends/Vulkan/vkinit.cpp" 
    "src/Graphics/Backends/Vulkan/VulkanBootstrap/VkBootstrap.cpp" 
    "src/Graphics/Backends/Vulkan/Volk/volk.cpp" 

    # Vulkan image backends
    "src/Graphics/Backends/Vulkan/VulkanTexture2D.cpp" 

    # OpenGL backends
    "src/Graphics/Backends/OpenGL/OpenGLBackend.cpp"

    # OpenGl image backends
    "src/Graphics/Backends/OpenGL/OpenGlTexture2D.cpp"

    # D3D11 backends
    "src/Graphics/Backends/D3D11/D3D11Backend.cpp"

    # D3D11 image backends
    "src/Graphics/Backends/D3D11/D3D11Texture2D.cpp"

    # Audio
    "src/Audio/AudioEngine.cpp" 
    "src/Audio/AudioStream.cpp" 
    "src/Audio/AudioSample.cpp" 
    "src/Audio/AudioEncoder.cpp" 

    # Math
    "src/Math/Color3.cpp" 
    "src/Math/UDim.cpp" 
    "src/Math/UDim2.cpp" 
    "src/Math/Vector2.cpp" 
    "src/Math/Vector3.cpp" 
    "src/Math/Vector4.cpp"
    "src/Math/Tween.cpp"

    # Inputs
    "src/Inputs/InputManager.cpp" 

    # Threads
    "src/Threads/Thread.cpp" 
    "src/Threads/TimeWatch.cpp"
    "src/Threads/AccumulatedTimeWatch.cpp"

    # Fonts
    "src/Fonts/FontManager.cpp"
    "src/Fonts/Platform/Win32.cpp"
    "src/Fonts/Platform/Linux.cpp"

    # Misc
    "src/Exceptions/EstException.cpp" 
    "src/Exceptions/EstNotImplemented.cpp"
    "src/Misc/Filesystem.cpp"
    "src/Misc/MD5.cpp"
    "src/Graphics/Utils/stb_image.cpp"
    "src/Graphics/Utils/lodepng.cpp"

    # Imgui
    "src/Imgui/imgui.cpp" 
    "src/Imgui/imgui_draw.cpp" 
    "src/Imgui/imgui_tables.cpp" 
    "src/Imgui/imgui_widgets.cpp"
    "src/Imgui/imgui_demo.cpp"
    "src/Graphics/ImguiBackends/imgui_impl_sdl2.cpp"
    "src/Graphics/ImguiBackends/imgui_impl_vulkan.cpp"
    "src/Graphics/ImguiBackends/imgui_impl_opengl3.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET EstEngine PROPERTY CXX_STANDARD 20)
endif()

include_directories("./include")
include_directories("../audio/include")

target_include_directories(EstEngine PUBLIC include)