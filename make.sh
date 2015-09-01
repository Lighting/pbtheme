#!/bin/sh

rm -f pbtheme

cmake \
    -D CMAKE_TOOLCHAIN_FILE=toolchain-arm-obreey-linux-gnueabi-pocketbook.cmake \
    -D TARGET_TYPE=ARM \
    -D DEVICE_NAME=pb360 \
    -D MAX_IMAGE_SCALE_MUL=2 \
    -D CMAKE_BUILD_TYPE=Release \
    -D ENABLE_CHM=1 \
    -D ENABLE_ANTIWORD=1 \
    -D GUI=CRGUI_PB \
    -D ENABLE_PB_DB_STATE=1 \
    -D BACKGROUND_CACHE_FILE_CREATION=1 \
    -D POCKETBOOK_PRO=1 \
    -D POCKETBOOK_PRO_FW5=1 \
    .
make

../SDK_481/bin/arm-obreey-linux-gnueabi-strip pbtheme
