#!/bin/bash

set echo on

mkdir -p ../bin

CFILES=$(find . -type f -name "*.c")

OUTPUT="engine"

CFLAGS="-g -shared -fdeclspec -fPIC"
LINKERFLAGS="-lSDL2 -lGLEW -lGL -lSDL2_ttf -lSDL2main -lSDL2_image"

INCLUDES="-Isrc"
DEFINITIONS="-D_DEBUG -DKEXPORT -DGLEW_STATIC"

echo "building $OUTPUT ... "
clang $CFILES $CFLAGS -o ../bin/lib$OUTPUT.so $DEFINITIONS $INCLUDES $LINKERFLAGS
