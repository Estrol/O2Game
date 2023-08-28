cmake_minimum_required(VERSION 3.0)

# check if VCPKG_ROOT environment is defined, if not and it's linux, check if ~/.vcpkg exist and define it
if(NOT DEFINED ENV{VCPKG_ROOT})
	if(UNIX AND NOT APPLE)
		if(EXISTS "$ENV{HOME}/.vcpkg")
			set(ENV{VCPKG_ROOT} "$ENV{HOME}/.vcpkg")
		endif()
	endif()
endif()

if(NOT DEFINED CMAKE_MAKE_PROGRAM)
	if(UNIX AND NOT APPLE)
		set(CMAKE_MAKE_PROGRAM "/usr/bin/make")
	endif()
endif()

if(NOT DEFINED CMAKE_CXX_COMPILER)
	if (WIN32)
		set(CMAKE_CXX_COMPILER "cl.exe")
		set(CMAKE_C_COMPILER "cl.exe")
	elseif (UNIX AND NOT APPLE)
		set(CMAKE_CXX_COMPILER "g++.exe")
		set(CMAKE_C_COMPILER "gcc.exe")
	else ()
		message(FATAL_ERROR "Not supported platform")
	endif ()
endif()

set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "Cmake generator platform")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

if (CMAKE_CXX_COMPILER STREQUAL "cl.exe")
	set(VCPKG_HOST_TRIPLET "x64-windows" CACHE STRING "Vcpkg host triplet")
	set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg target triplet")
else ()
	if (UNIX AND NOT APPLE)
		set(VCPKG_HOST_TRIPLET "x64-linux" CACHE STRING "Vcpkg host triplet")
		set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg target triplet")
	elseif (WIN32)
		set(VCPKG_HOST_TRIPLET "x64-mingw-static" CACHE STRING "Vcpkg host triplet")
		set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "Vcpkg target triplet")
	else ()
		message(FATAL_ERROR "Not supported platform")
	endif()
endif()

project(O2Game)

set(CMAKE_CXX_STANDARD 20)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
	set(CMAKE_C_FLAGS -m64)
	set(CMAKE_CXX_FLAGS -m64)
	add_definitions(-D__cpp_lib_char8_t)
	add_definitions(-D__cpp_lib_clamp)
endif()

set(GFLAGS_USE_TARGET_NAMESPACE ON)
include(${CMAKE_TOOLCHAIN_FILE})
find_package(Freetype REQUIRED)

find_package(glm CONFIG REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(SDL2 CONFIG REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

if (MSVC)
	set(SDL2_LIBRARIES SDL2::SDL2)
else ()
	set(SDL2_LIBRARIES SDL2)
endif()

message("TEST: ${SDL2_LIBRARIES}")

find_package(SDL2_image CONFIG REQUIRED)
if (MSVC)
	message("[SDL2_Image] using MSVC")
	set(SDL2_IMAGE_LIBRARIES SDL2_image::SDL2_image)
else ()
	message("[SDL2_Image] using GNU GCC")
	set(SDL2_IMAGE_LIBRARIES SDL2_Image)
endif()

set("FREETYPE_LIB_PATH" "")
list(GET FREETYPE_LIBRARIES 1 FREETYPE_OPTIMIZED_LIB)
list(GET FREETYPE_LIBRARIES 3 FREETYPE_DEBUG_LIB)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set("FREETYPE_LIB_PATH" "${FREETYPE_DEBUG_LIB}")
else()
    set("FREETYPE_LIB_PATH" "${FREETYPE_OPTIMIZED_LIB}")
endif()

if (NOT MSVC)
	find_package(PNG REQUIRED)
	set("PNG_LIB" png)

	find_package(ZLIB REQUIRED)
	set("ZLIB_LIB" z)

	find_package(JPEG REQUIRED)
	set("JPEG_LIB" jpeg)

	find_package(BZip2 REQUIRED)
	set("BZIP2_LIB" bz2)
endif()

find_package(Vulkan)
if (NOT DEFINED Vulkan_INCLUDE_DIRS)
	if (NOT DEFINED ENV{VULKAN_SDK})
		message(FATAL_ERROR "Cannot find Vulkan and VULKAN_SDK environment variable not defined")
	endif()

	set(Vulkan_INCLUDE_DIRS $ENV{VULKAN_SDK}/include)
	set(Vulkan_LIBRARIES $ENV{VULKAN_SDK}/lib/vulkan-1.lib)
endif()

# include directories from: third-party/headers/*.h
include_directories(third-party/headers ${Vulkan_INCLUDE_DIRS})

if (WIN32)
    file(GLOB THIRD_PARTY "./third-party/lib/x64-windows/Debug/*.lib")
elseif (UNIX AND NOT APPLE)
	file(GLOB THIRD_PARTY "./third-party/lib/x64-linux/Debug/*.a")
else ()
    message(FATAL_ERROR "Not supported platform")
endif()

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

set(O2GAME_LIBRARIES
	${THIRD_PARTY} 
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${Vulkan_LIBRARIES} 
    ${FREETYPE_LIB_PATH} 
	${PNG_LIB} 
	${JPEG_LIB} 
	${ZLIB_LIB} 
	${BZIP2_LIB})

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(Engine)
add_subdirectory(Game)