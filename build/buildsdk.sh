#!/bin/bash

export BUILD_PATH=$PWD
export IDF_PATH=$PWD/../ESP8266_RTOS_SDK
export PATH=$PWD:$PWD/esp-clang/bin:$PWD/xtensa-lx106-elf/bin:$PATH

if [ ! -f "hello_world/README.md" ]; then
  cp -r $IDF_PATH/examples/get-started/hello_world .
fi

cd hello_world
make menuconfig
make -j
cd ..
