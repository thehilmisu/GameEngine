#!/bin/bash

echo "Building everything ... "
echo "------------------------"

# Create bin directory if it doesn't exist
mkdir -p bin

# Build engine
echo "building engine ... "
cd engine
./build.sh
cd ..

# Build testbed
echo "building testbed"
cd testbed
./build.sh
cd ..

# Copy the built files to bin directory
cp engine/libengine.so bin/
cp testbed/testbed bin/

# Remove the built files after copying to bin directory
rm engine/libengine.so
rm testbed/testbed

# Copy testbed and engine assets to the bin directory
echo "Copying assets to bin directory"
mkdir -p bin/assets/fonts
if [ -d "testbed/assets" ]; then
  cp -r testbed/assets/* bin/assets/
fi

if [ -d "engine/assets" ]; then
  cp -r engine/assets/* bin/assets/
fi
echo "Successfully build all libs "
