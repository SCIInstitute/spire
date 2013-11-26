#!/bin/bash
mkdir bin
cd bin
cmake -DUSE_OS_MESA=ON ../batch
make
./spirebatch
