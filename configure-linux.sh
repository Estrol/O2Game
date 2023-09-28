#!/bin/bash

# check if VCPKG_ROOT exist
if [ -z "$VCPKG_ROOT" ]; then
  echo "VCPKG_ROOT is not set, please install vcpkg and set VCPKG_ROOT to vcpkg root directory"
  exit 1
fi

# set default triplet
if [ -z "$VCPKG_DEFAULT_TRIPLET" ]; then
  export VCPKG_DEFAULT_TRIPLET=x64-linux
fi
# build x64 cmake
cmake . -G "Unix Makefiles" -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=$VCPKG_DEFAULT_TRIPLET -DCMAKE_MAKE_PROGRAM=/usr/bin/make -DCMAKE_BUILD_TYPE=Debug -B build