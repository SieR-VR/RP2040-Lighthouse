#!/bin/bash

[ ! -d pico-sdk ] && \
  git clone --recursive https://github.com/raspberrypi/pico-sdk

docker run --rm -it \
  -v $(pwd)/pico-sdk:/pico-sdk \
  -v $(pwd):/project \
  osumaet/build-rp2040 \
  sh -c 'mkdir -p build && cd build && cmake .. && make'
