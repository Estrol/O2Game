cmake_minimum_required(VERSION 3.0)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
if(WIN32)
	set(VCPKG_DEFAULT_TRIPLET "x64-windows" CACHE STRING "Vcpkg default triplet")
elseif(UNIX AND NOT APPLE)
	set(VCPKG_DEFAULT_TRIPLET "x64-linux" CACHE STRING "Vcpkg default triplet")
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# parallel build
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "Cmake generator platform")
project(O2Game)

include(${CMAKE_TOOLCHAIN_FILE})

find_package(Sdl2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(SDL2_image CONFIG REQUIRED)
set(SDL2_IMAGE_LIBRARIES $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

find_package(Freetype REQUIRED)

find_package(glm CONFIG REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

set("FREETYPE_LIB_PATH" "")
list(GET FREETYPE_LIBRARIES 1 FREETYPE_OPTIMIZED_LIB)
list(GET FREETYPE_LIBRARIES 3 FREETYPE_DEBUG_LIB)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set("FREETYPE_LIB_PATH" "${FREETYPE_DEBUG_LIB}")
else()
    set("FREETYPE_LIB_PATH" "${FREETYPE_OPTIMIZED_LIB}")
endif()

# include directories from: third-party/headers/*.h
include_directories(third-party/headers)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(Engine)
add_subdirectory(Game)