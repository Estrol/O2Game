# Compiling shaders

By default cmake will compile these shaders into the SPIRV format. This is done by the `glangvalidator` tool located in your `VULKAN_SDK` directory.

To compile just refresh the cmake project and it will compile the shaders into this folder with prefix `.sph.h`

**Note: Do not upload the .spv.h to git, as it will be compiled at every CMake configure.*