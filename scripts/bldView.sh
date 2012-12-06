#!/bin/bash
cd "$(dirname "$0")"

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
mkdir -p ../bin/viewer
pushd ../bin/viewer
  cmake -D CMAKE_BUILD_TYPE="Debug" ../../tools/view
  make -j1
popd

