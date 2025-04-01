#!/bin/bash

echo "Building engine shared library..."

# Create object directory if it doesn't exist
mkdir -p obj

# Compile each source file
for file in src/*.c src/core/*.c src/platform/*.c src/renderer/*.c src/renderer/opengl/*.c src/containers/*.c src/shaders/*.c; do
    if [ -f "$file" ]; then
        obj_file="obj/$(basename ${file%.c}.o)"
        clang -g -fPIC -c "$file" -o "$obj_file" -I/usr/include/SDL2 -I/usr/include/freetype2 -Isrc -D_GNU_SOURCE=1 -D_REENTRANT
    fi
done

# Create shared library
clang -g -shared -o libengine.so obj/*.o -lSDL2 -lGL -lGLEW -lfreetype -lm

# Clean up object files
rm -rf obj

echo "Engine build complete."
