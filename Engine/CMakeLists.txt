cmake_minimum_required(VERSION 3.0.0)

project(EstEngine VERSION 0.1.0 LANGUAGES C CXX)

# recursive search file in ./src
file(GLOB_RECURSE HEADERS "./include/*.h")

# CXX20
set(CMAKE_CXX_STANDARD 20)

# _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

add_library(EstEngine
	${HEADERS} 

	# Audio
	"src/Audio/Audio.cpp"
	"src/Audio/AudioManager.cpp"
	"src/Audio/AudioSample.cpp"
	"src/Audio/AudioSampleChannel.cpp"
	"src/Audio/BassFXSampleEncoding.cpp"

	#Data
	"src/Data/Imgui/imgui.cpp"
	"src/Data/Imgui/imgui_demo.cpp"
	"src/Data/Imgui/imgui_draw.cpp"
	"src/Data/Imgui/imgui_impl_sdl2.cpp"
	"src/Data/Imgui/imgui_impl_sdlrenderer2.cpp"
	"src/Data/Imgui/imgui_impl_vulkan.cpp"
	"src/Data/Imgui/imgui_tables.cpp"
	"src/Data/Imgui/imgui_widgets.cpp"
	"src/Data/Imgui/ImguiUtil.cpp"
	"src/Data/Imgui/imgui_extends.cpp" 

	#Data/misc/freetype
	"src/Data/Imgui/misc/freetype/imgui_freetype.cpp"

	#Fonts
	"src/Fonts/FontResources.cpp"
	"src/Fonts/FallbackFonts/arial.ttf.cpp"  
	"src/Fonts/FallbackFonts/ch.ttf.cpp" 
	"src/Fonts/FallbackFonts/jp.ttf.cpp" 
	"src/Fonts/FallbackFonts/kr.ttf.cpp"
	
	#Inputs
	"src/Inputs/InputManager.cpp"
	"src/Inputs/InputEvent.cpp"

	#Misc
	"src/Misc/MathUtils.cpp"
	"src/Misc/SDLException.cpp"
	"src/Misc/Lodepng.cpp"
	"src/Misc/md5.cpp"

	#Logs
	"src/Logs/Console.cpp" 
	"src/Logs/Logs.cpp" 

	#Rendering
	"src/Rendering/Renderer.cpp"
	"src/Rendering/GameWindow.cpp"
	"src/Rendering/Vulkan/Texture2DVulkan.cpp"
	"src/Rendering/Vulkan/vkinit.cpp"
	"src/Rendering/Vulkan/VulkanEngine.cpp"
	"src/Rendering/Vulkan/volk/volk.cpp" 
	"src/Rendering/Vulkan/vk_mem_alloc/Common.cpp"
	"src/Rendering/Vulkan/vk_mem_alloc/VmaUsage.cpp"
	"src/Rendering/Vulkan/VkBootstrap/VkBootstrap.cpp"

	#Scenes
	"src/Scenes/SceneManager.cpp"
	"src/Scenes/Scene.cpp"
	"src/Scenes/MsgBox.cpp"
	"src/Scenes/Overlay.cpp"
	
	#Texture
	"src/Texture/Color3.cpp"
	"src/Texture/NumericTexture.cpp"
	"src/Texture/ResizableImage.cpp"
	"src/Texture/Sprite2D.cpp"
	"src/Texture/Text.cpp"
	"src/Texture/Texture2D.cpp"
	"src/Texture/UDim.cpp"
	"src/Texture/UDim2.cpp"
	"src/Texture/Vector2.cpp"
	"src/Texture/ImageGenerator.cpp"

	#UI
	# "src/UI/UIBase.cpp" 
	# "src/UI/Quad.cpp" 
	# "src/UI/Circle.cpp" 

	#Threading
	"src/Threading/GameThread.cpp"

	#Main
	"src/Configuration.cpp"
	"src/Game.cpp"
)

if (WIN32)
	set(glslangValidator "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
elseif (UNIX AND NOT APPLE)
	set(glslangValidator "$ENV{VULKAN_SDK}/Bin/glslangValidator")
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

execute_process(
	COMMAND "${glslangValidator}" -gVS
		-V -o "${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Vulkan/Shaders/position.spv.h" 
		--vn __glsl_vert "${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Vulkan/Shaders/position.vert"
	COMMAND "${glslangValidator}" -gVS
		-V -o "${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Vulkan/Shaders/color.spv.h" 
		--vn __glsl_frag "${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Vulkan/Shaders/color.frag"
)

if (MSVC)
	target_compile_options(EstEngine PUBLIC "/MP")
endif()
target_include_directories(EstEngine PUBLIC ./include/)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})