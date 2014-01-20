#!/bin/sh

if [ ! -d build.debug ] ; then 
    mkdir build.debug
fi

cd build.debug
cmake -DCMAKE_BUILD_TYPE=Debug ../ 
cmake --build . --target install
if [ "$(uname)" == "Darwin" ] ; then 
    cd ./../../install/mac-clang-x86_64d/bin/
    lldb swnt_debug
else
    cd ./../../install/linux-gcc-x86_64d/bin/
    gdb swnt_debug
fi


