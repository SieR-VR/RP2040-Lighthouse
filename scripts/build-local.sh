#!/bin/bash

[ ! -d pico-sdk ] && \
 git clone --recursive https://github.com/raspberrypi/pico-sdk

export PICO_SDK_PATH=$(pwd)/pico-sdk

mkdir -p build && cd build && cmake .. -DPICO_SDK_PATH=$PICO_SDK_PATH
make
