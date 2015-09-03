#!/bin/sh

rm -f pbtheme

cmake \
    -D CMAKE_TOOLCHAIN_FILE=toolchain-arm-obreey-linux-gnueabi-pocketbook.cmake \
    -D TARGET_TYPE=ARM \
    -D CMAKE_BUILD_TYPE=Release \
    .
make

../SDK_481/bin/arm-obreey-linux-gnueabi-strip pbtheme
