#!/bin/bash
cd "$(dirname "$0")"

MY_CC="${HOME}/.vim/vim-addons/clang_complete/bin/cc_args.py /usr/bin/clang"
MY_CXX="${HOME}/.vim/vim-addons/clang_complete/bin/cc_args.py /usr/bin/clang++"
mkdir -p ../bin/viewer
pushd ../bin/viewer > /dev/null
  CC=${MY_CC} CXX=${MY_CXX} cmake -D CMAKE_BUILD_TYPE="Debug" ../../tools/view
  CC=${MY_CC} CXX=${MY_CXX} VERBOSE=1 make 
popd > /dev/null

