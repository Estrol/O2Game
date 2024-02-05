#!/bin/bash

if [ -z "$VULKAN_SDK" ]
then
    echo "VULKAN_SDK is not defined. Please set it to the Vulkan SDK directory"
    exit 1
fi

GLSLC=$VULKAN_SDK/Bin/glslangValidator
SHADER_DIR=lib/src/Graphics/Shaders
SHADER_GLSL_DIR=$SHADER_DIR/GLSL

for f in $SHADER_GLSL_DIR/*.vert $SHADER_GLSL_DIR/*.frag
do
    filename=$(basename -- "$f")
    filename="${filename%.*}"
    $GLSLC -V -o $SHADER_DIR/SPV/$filename.spv.h --vn "__glsl_$filename" $f
done