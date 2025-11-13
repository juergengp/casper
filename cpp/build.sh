#!/bin/bash
#
# Build script for ollamaCode C++ version
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}Building ollamaCode C++ version${NC}"
echo ""

# Check dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"

check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}✗ $1 not found${NC}"
        echo -e "${YELLOW}Install with: $2${NC}"
        return 1
    else
        echo -e "${GREEN}✓ $1 found${NC}"
        return 0
    fi
}

MISSING_DEPS=0

check_command cmake "sudo dnf install cmake" || MISSING_DEPS=1
check_command g++ "sudo dnf install gcc-c++" || MISSING_DEPS=1

# Check for libraries
echo -e "${YELLOW}Checking libraries...${NC}"

check_lib() {
    if pkg-config --exists $1 2>/dev/null; then
        echo -e "${GREEN}✓ $1 found${NC}"
        return 0
    else
        echo -e "${RED}✗ $1 not found${NC}"
        echo -e "${YELLOW}Install with: $2${NC}"
        return 1
    fi
}

check_lib libcurl "sudo dnf install libcurl-devel" || MISSING_DEPS=1
check_lib sqlite3 "sudo dnf install sqlite-devel" || MISSING_DEPS=1

if [ $MISSING_DEPS -eq 1 ]; then
    echo ""
    echo -e "${RED}Please install missing dependencies and try again.${NC}"
    exit 1
fi

echo ""
echo -e "${YELLOW}Creating build directory...${NC}"
rm -rf build
mkdir -p build
cd build

echo -e "${YELLOW}Running CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE=Release ..

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ CMake configuration failed${NC}"
    exit 1
fi

echo ""
echo -e "${YELLOW}Building...${NC}"
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}✓ Build successful!${NC}"
echo ""
echo -e "${BLUE}Binary location: ${CYAN}$(pwd)/ollamacode${NC}"
echo ""
echo -e "${BLUE}To install:${NC}"
echo -e "  ${CYAN}sudo make install${NC}"
echo ""
echo -e "${BLUE}To test:${NC}"
echo -e "  ${CYAN}./ollamacode --version${NC}"
echo -e "  ${CYAN}./ollamacode --help${NC}"
echo ""
