cmake_minimum_required(VERSION 3.0)
message("Unnamed O2 Game Clone Project - CMake Build System")

# check if CXX_COMPILER is not cl.exe then set 

project(O2Game)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(O2GAME_PLATFORM x64)
else()
	set(O2GAME_PLATFORM x86)
endif()

message("Platform: ${O2GAME_PLATFORM} - ${CMAKE_SIZEOF_VOID_P}")

set(CMAKE_CXX_STANDARD 20)
if(MSVC)
    add_compile_options(/utf-8 /D_CRT_SECURE_NO_WARNINGS)
else()
	set(CMAKE_C_FLAGS -m64)
	set(CMAKE_CXX_FLAGS -m64)
	add_definitions(-D__cpp_lib_char8_t)
	add_definitions(-D__cpp_lib_clamp)
    add_compile_options(-finput-charset=UTF-8)
endif()

set(GFLAGS_USE_TARGET_NAMESPACE ON)

# BEGIN PACKAGES
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
set(SDL2_IMAGE_LIBRARIES $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
set(SQLite3_LIBRARIES unofficial::sqlite3::sqlite3)

find_package(Freetype REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)
find_package(BZip2 REQUIRED)
find_package(Vulkan)

if (NOT DEFINED Vulkan_INCLUDE_DIRS)
    if (NOT DEFINED ENV{VULKAN_SDK})
		message(FATAL_ERROR "Cannot find Vulkan and VULKAN_SDK environment variable not defined")
	endif()

	set(Vulkan_INCLUDE_DIRS $ENV{VULKAN_SDK}/include)
	set(Vulkan_LIBRARIES $ENV{VULKAN_SDK}/lib/vulkan-1.lib)
endif()
# END PACKAGES

include_directories(
	third-party/headers 
	${Vulkan_INCLUDE_DIRS} 
	${SDL2_INCLUDE_DIRS}
	${GLM_INCLUDE_DIRS}
)

# third-party libraries
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
set(LIB_INDEX 1)
else()
set(LIB_INDEX 3)
endif()


if (WIN32)
	set(THIRD_PARTY_LIBS
		"${PROJECT_SOURCE_DIR}/third-party/lib/${O2GAME_PLATFORM}-windows/${CMAKE_BUILD_TYPE}/bass.lib" 
		"${PROJECT_SOURCE_DIR}/third-party/lib/${O2GAME_PLATFORM}-windows/${CMAKE_BUILD_TYPE}/bass_fx.lib" 
	)

elseif (UNIX AND NOT APPLE)
    set(THIRD_PARTY_LIBS 
        "${PROJECT_SOURCE_DIR}/third-party/lib/${O2GAME_PLATFORM}-linux/${CMAKE_BUILD_TYPE}/libbass.so"
        "${PROJECT_SOURCE_DIR}/third-party/lib/${O2GAME_PLATFORM}-linux/${CMAKE_BUILD_TYPE}/libbass_fx.so"
        dl 
        pthread 
        m 
    )
endif()

# check if FREETYPE_LIBRAIES is a list then check first index if optimized
if (FREETYPE_LIBRARIES MATCHES ";")
    list(GET FREETYPE_LIBRARIES ${LIB_INDEX} LIB_FREETYPE)
else()
    set(LIB_FREETYPE ${FREETYPE_LIBRARIES})
endif()

if (JPEG_LIBRARIES MATCHES ";")
    list(GET JPEG_LIBRARIES ${LIB_INDEX} LIB_JPEG)
else()
    set(LIB_JPEG ${JPEG_LIBRARIES})
endif()

list(GET PNG_LIBRARIES ${LIB_INDEX} LIB_PNG)

if (PNG_LIBRARIES MATCHES ";")
    list(GET PNG_LIBRARIES ${LIB_INDEX} LIB_PNG)
else()
    set(LIB_PNG ${PNG_LIBRARIES})
endif()

if (BZIP2_LIBRARIES MATCHES ";")
    list(GET BZIP2_LIBRARIES ${LIB_INDEX} LIB_BZIP2)
else()
    set(LIB_BZIP2 ${BZIP2_LIBRARIES})
endif()

set(O2GAME_LIBRARIES 
    ${Vulkan_LIBRARIES} 
    ${SDL2_LIBRARIES} 
    ${SDL2_IMAGE_LIBRARIES} 
    ${SQLite3_LIBRARIES} 
    ${LIB_FREETYPE} 
    ${LIB_PNG} 
    ${LIB_JPEG} 
    ${LIB_BZIP2} 
    ${THIRD_PARTY_LIBS} 
)

add_subdirectory(Engine)
add_subdirectory(Game)