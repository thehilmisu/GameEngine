#!/bin/bash

set echo on

mkdir -p ../bin

CFILES=$(find . -type f -name "*.c")

OUTPUT="testbed"
CFLAGS="-g -fdeclspec -fPIC -Wall -Werror"

INCLUDES="-Isrc -I../engine/src/"
LINKERFLAGS="-L../bin/ -lengine -Wl,-rpath,."
DEFINITIONS="-D_DEBUG -DKIMPORT"

echo "building $OUTPUT"

clang $CFILES $CFLAGS -o ../bin/$OUTPUT $DEFINITIONS $INCLUDES $LINKERFLAGS

