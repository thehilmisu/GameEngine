#!/bin/bash

set echo on

mkdir -p ../bin

CFILES=$(find . -type f -name "*.c")

OUTPUT="engine"

#CFLAGS="-g -shared -fdeclspec -fPIC -Wall -Werror"
CFLAGS="-g -shared -fdeclspec -fPIC"
LINKERFLAGS="-lvulkan -lSDL2 -L$VULKAN_SDK/lib -lSDL2_ttf -lSDL2main -lSDL2_image"

INCLUDES="-Isrc -I$VULKAN_SDK/include"
DEFINITIONS="-D_DEBUG -DKEXPORT"

echo "building $OUTPUT ... "
clang $CFILES $CFLAGS -o ../bin/lib$OUTPUT.so $DEFINITIONS $INCLUDES $LINKERFLAGS
