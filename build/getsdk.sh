wget -N https://github.com/espressif/llvm-project/releases/download/esp-15.0.0-20221201/llvm-esp-15.0.0-20221201-macos.tar.xz
wget -N https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.1.0-gcc10.3/x86_64-apple-darwin14.xtensa-lx106-elf-e5f9fec.220621.tar.gz

if [ ! -d "esp-clang" ]; then
  tar xvf llvm-esp-15.0.0-20221201-macos.tar.xz
fi

if [ ! -d "xtensa-lx106-elf" ]; then
  tar xvf x86_64-apple-darwin14.xtensa-lx106-elf-e5f9fec.220621.tar.gz
fi

if [ ! -d "esp-clang/xtensa-lx106-elf" ]; then
  ln -s ../xtensa-lx106-elf/xtensa-lx106-elf esp-clang/xtensa-lx106-elf
fi

if [ ! -d "../lwip" ]; then
  mkdir ../lwip
  ln -s ../ESP8266_NONOS_SDK/third_party/include ../lwip/include
  ln -s ../ESP8266_NONOS_SDK/third_party/lwip ../lwip/src
fi

if [ ! -d "../mqtt" ]; then
  ln -s ESP8266_NONOS_SDK/examples/esp_mqtt_proj/mqtt ../mqtt
fi
