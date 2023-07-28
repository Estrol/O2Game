cmake_minimum_required(VERSION 3.0.0)
project(Game VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB_RECURSE MAIN "./src/*.cpp" "./src/*.h")
add_executable(Game ${MAIN})

if (WIN32)
    target_compile_definitions(EstEngine PRIVATE EST_PLATFORM_WINDOWS)
    file(GLOB WIN32_LIBRARIES "../third-party/lib/x64-windows/Debug/*.lib")

    add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/third-party/bin/x64-windows/Debug
        $<TARGET_FILE_DIR:Game>)
elseif (UNIX NOT APPLE)
    target_compile_definitions(EstEngine PRIVATE EST_PLATFORM_LINUX)
	file(GLOB LINUX_LIBRARIES "../third-party/lib/x64-linux/Debug/*.a")
	message("LINUX_LIBRARIES: ${LINUX_LIBRARIES}")

	add_custom_command(TARGET Game POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/third-party/bin/x64-linux/Debug
		$<TARGET_FILE_DIR:Game>)
else()
    message(FATAL_ERROR "Not supported platform")
endif()

#include "../Engine/include"
target_include_directories(Game PRIVATE "../Engine/include")

# set preprocessor MEM_LEAK_DEBUG
target_compile_definitions(Game PRIVATE MEM_LEAK_DEBUG=1 _CRT_SECURE_NO_WARNINGS)

target_link_libraries(Game PRIVATE EstEngine 
    ${WIN32_LIBRARIES} 
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${Vulkan_LIBRARIES} 
    ${FREETYPE_LIB_PATH})