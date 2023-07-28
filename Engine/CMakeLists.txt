cmake_minimum_required(VERSION 3.0.0)

project(EstEngine VERSION 0.1.0 LANGUAGES C CXX)

# recursive search file in ./src
file(GLOB_RECURSE MAIN "./include/*.h" "./src/*.cpp")

# CXX20
set(CMAKE_CXX_STANDARD 20)

# _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

add_library(EstEngine ${MAIN})

target_include_directories(EstEngine PUBLIC ./include/)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})