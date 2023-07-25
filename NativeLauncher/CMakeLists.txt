# CMakeList.txt : CMake project for NativeLauncher, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (NativeLauncher "NativeLauncher.cpp" "NativeLauncher.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NativeLauncher PROPERTY CXX_STANDARD 20)
endif()

# if linux add -ldl
if (UNIX AND NOT APPLE)
  target_link_libraries(NativeLauncher dl)

# TODO: Add tests and install targets if needed.
