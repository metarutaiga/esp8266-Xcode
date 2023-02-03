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

if [ ! -f "patchsdk.ok" ]; then
  echo ok > patchsdk.ok
  cd ../ESP8266_RTOS_SDK
  patch < ../build/patchsdk
  cd ../build
  printf 'wifi'     | dd of=../ESP8266_RTOS_SDK/components/esp8266/lib/libpp.a bs=1 seek=0x2DE6 count=4 conv=notrunc
  printf '\x05\x00' | dd of=../ESP8266_RTOS_SDK/components/esp8266/lib/libpp.a bs=1 seek=0x36AA count=2 conv=notrunc
  printf '\x14\x00' | dd of=../ESP8266_RTOS_SDK/components/esp8266/lib/libpp.a bs=1 seek=0x3D46 count=2 conv=notrunc
  printf '\x32\x00' | dd of=../ESP8266_RTOS_SDK/components/esp8266/lib/libpp.a bs=1 seek=0x3D52 count=2 conv=notrunc
  printf '\x9C\x05' | dd of=../ESP8266_RTOS_SDK/components/esp8266/lib/libpp.a bs=1 seek=0xDAEA count=2 conv=notrunc
  printf '\x5B\x00' | dd of=../ESP8266_RTOS_SDK/components/esp8266/lib/libpp.a bs=1 seek=0xDAEE count=2 conv=notrunc
  printf '\xB2\x00' | dd of=../ESP8266_RTOS_SDK/components/esp8266/lib/libpp.a bs=1 seek=0x3BC5E count=2 conv=notrunc
fi
