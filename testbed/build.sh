#!/bin/bash

set echo on

mkdir -p ../bin

CFILES=$(find . -type f -name "*.c")

OUTPUT="testbed"
CFLAGS="-g -fdeclspec -fPIC -Wall -Werror"

INCLUDES="-I../engine/src -I../engine/src/core -I../engine/src/renderer -Isrc"
LINKERFLAGS="-L../bin/ -lengine -lm -Wl,-rpath,."
DEFINITIONS="-D_DEBUG -DKIMPORT"

echo "building $OUTPUT"

clang $CFILES $CFLAGS -o ../bin/$OUTPUT $DEFINITIONS $INCLUDES $LINKERFLAGS

