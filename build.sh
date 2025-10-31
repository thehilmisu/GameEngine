#!/bin/bash
# Build script for GameEngine

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building GameEngine...${NC}"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir build
fi

cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake ..

# Build
echo -e "${YELLOW}Building...${NC}"
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    make -j$(sysctl -n hw.ncpu)
else
    # Linux
    make -j$(nproc)
fi

# Copy compile_commands.json to root for IDE support
if [ -f "compile_commands.json" ]; then
    cp compile_commands.json ..
fi

echo -e "${GREEN}Build successful!${NC}"
echo -e "${YELLOW}Run with: ./run.sh${NC}"
echo -e "${YELLOW}Or navigate to build/bin and run: ./testbed${NC}"

