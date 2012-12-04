#!/bin/bash
cd "$(dirname "$0")"

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
mkdir -p ../bin
pushd ../bin > /dev/null
  cmake -D CMAKE_BUILD_TYPE="Debug" ../tools/view
  make -j4
popd > /dev/null

