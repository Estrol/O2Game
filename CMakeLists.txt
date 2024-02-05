cmake_minimum_required(VERSION 3.0)
message("EstEngine Project - CMake Build System")

# check if Environment VULKAN_SDK is set
if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "Environment VULKAN_SDK is required to compile the shaders!")
endif()

# check additional parameters

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(EstEngine)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/utf-8 /D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/WX)
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    add_definitions(-D__cpp_lib_char8_t)
    add_definitions(-D__cpp_lib_clamp)
    add_compile_options(-finput-charset=UTF-8)
    add_compile_options(-Wall -Wextra -pedantic)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    if (MSVC)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /DVK_NO_PROTOTYPES")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /DVK_NO_PROTOTYPES")
    else()
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -DVK_NO_PROTOTYPES")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DVK_NO_PROTOTYPES")
    endif()
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    if (MSVC)
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GL /DVK_NO_PROTOTYPES")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GL /DVK_NO_PROTOTYPES")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
    else()
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DVK_NO_PROTOTYPES")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DVK_NO_PROTOTYPES")
    endif()
endif()

if (MSVC)
    add_compile_options(/MP)
endif()

set(GFLAGS_USE_TARGET_NAMESPACE ON)

# BEGIN PACKAGES
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(Freetype REQUIRED)
find_package(spirv_cross_core REQUIRED)
find_package(spirv_cross_glsl REQUIRED)
find_package(Iconv REQUIRED)
find_package(sol2 REQUIRED)
find_package(lua REQUIRED)
find_package(lodepng REQUIRED)
find_package(unofficial-sqlite3 REQUIRED)
# END PACKAGES

# set TRACY_ENABLE
include_directories(
	${CMAKE_SOURCE_DIR}/third-party
    ${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-windows/include
)

add_subdirectory(third-party)
add_subdirectory(lib)
add_subdirectory(src)