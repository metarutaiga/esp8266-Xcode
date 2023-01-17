#!/bin/bash

wget -N https://github.com/espressif/llvm-project/releases/download/esp-15.0.0-20221201/llvm-esp-15.0.0-20221201-macos.tar.xz
wget -N https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-macos.tar.gz

if [ ! -d "esp-clang" ]; then
  tar xvf llvm-esp-15.0.0-20221201-macos.tar.xz
fi

if [ ! -d "xtensa-lx106-elf" ]; then
  tar xvf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-macos.tar.gz
fi

if [ ! -d "esp-clang/xtensa-lx106-elf" ]; then
  ln -s ../xtensa-lx106-elf/xtensa-lx106-elf esp-clang/xtensa-lx106-elf
fi
