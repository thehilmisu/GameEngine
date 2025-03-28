#!/bin/bash

echo "Building testbed executable..."

# Compile testbed with rpath to include current directory for library lookup
clang src/*.c -I../engine/src -L../engine -lengine -I/usr/include/SDL2 -I/usr/include/freetype2 -D_GNU_SOURCE=1 -D_REENTRANT -lSDL2 -lGL -lGLEW -lfreetype -lm -Wl,-rpath='$ORIGIN' -o testbed

echo "Testbed build complete."

