#!/bin/bash

export IDF_PATH=$PWD/../ESP8266_RTOS_SDK
export PATH=$PWD/esp-clang:$PWD/xtensa-lx106-elf/bin:$PATH

if [ ! -d "hello_world" ]; then
  cp -r $IDF_PATH/examples/get-started/hello_world .
fi

cd hello_world
make menuconfig
make
cd ..
