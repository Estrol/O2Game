#!/bin/bash

# check if VCPKG_ROOT exist
if [ -z "$VCPKG_ROOT" ]; then
  echo "VCPKG_ROOT is not set, please install vcpkg and set VCPKG_ROOT to vcpkg root directory"
  exit 1
fi

# if debug
if [ "$1" == "debug" ]; then
  echo "debug mode"
  BUILD_TYPE="x64-linux-debug"
else
  BUILD_TYPE="x64-linux"
fi

cmake . --preset=$BUILD_TYPE