#!/bin/bash
#
# ollamaCode - macOS Installation Script
# Version: 1.0.0
#

set -euo pipefail

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'
BOLD='\033[1m'

# Installation paths
INSTALL_DIR="/usr/local/bin"
SCRIPT_NAME="ollamacode"

echo -e "${BLUE}${BOLD}"
cat << 'EOF'
   ____  _ _                       ____          _
  / __ \| | | __ _ _ __ ___   __ _/ ___|___   __| | ___
 | |  | | | |/ _` | '_ ` _ \ / _` | |   / _ \ / _` |/ _ \
 | |__| | | | (_| | | | | | | (_| | |__| (_) | (_| |  __/
  \____/|_|_|\__,_|_| |_| |_|\__,_|\____\___/ \__,_|\___|

EOF
echo -e "${NC}${BLUE}macOS Installation Script - Version 1.0.0${NC}"
echo ""

# Check if running on macOS
if [[ "$(uname)" != "Darwin" ]]; then
    echo -e "${RED}✗ Error: This script is for macOS only${NC}"
    exit 1
fi

# Check for required tools
echo -e "${YELLOW}Checking dependencies...${NC}"

# Check for Homebrew
if ! command -v brew &> /dev/null; then
    echo -e "${YELLOW}Homebrew not found. Installing Homebrew...${NC}"
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Check for jq
if ! command -v jq &> /dev/null; then
    echo -e "${YELLOW}Installing jq...${NC}"
    brew install jq
fi

# Check for Ollama
if ! command -v ollama &> /dev/null; then
    echo -e "${YELLOW}Ollama not found. Would you like to install it? (y/n)${NC}"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        echo -e "${YELLOW}Installing Ollama...${NC}"
        brew install ollama
    else
        echo -e "${RED}✗ Ollama is required. Please install it from: https://ollama.ai${NC}"
        exit 1
    fi
fi

echo -e "${GREEN}✓ All dependencies satisfied${NC}"
echo ""

# Install ollamacode
echo -e "${YELLOW}Installing ollamaCode...${NC}"

# Find the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ ! -f "${SCRIPT_DIR}/bin/ollamacode" ]]; then
    echo -e "${RED}✗ Error: ollamacode script not found in ${SCRIPT_DIR}/bin/${NC}"
    exit 1
fi

# Copy to installation directory
if [[ -w "${INSTALL_DIR}" ]]; then
    cp "${SCRIPT_DIR}/bin/ollamacode" "${INSTALL_DIR}/${SCRIPT_NAME}"
    chmod +x "${INSTALL_DIR}/${SCRIPT_NAME}"
else
    echo -e "${YELLOW}Administrator privileges required to install to ${INSTALL_DIR}${NC}"
    sudo cp "${SCRIPT_DIR}/bin/ollamacode" "${INSTALL_DIR}/${SCRIPT_NAME}"
    sudo chmod +x "${INSTALL_DIR}/${SCRIPT_NAME}"
fi

echo -e "${GREEN}✓ ollamaCode installed to ${INSTALL_DIR}/${SCRIPT_NAME}${NC}"
echo ""

# Create configuration directory
CONFIG_DIR="${HOME}/.config/ollamacode"
mkdir -p "${CONFIG_DIR}"
echo -e "${GREEN}✓ Configuration directory created at ${CONFIG_DIR}${NC}"

# Test installation
echo ""
echo -e "${YELLOW}Testing installation...${NC}"
if command -v ollamacode &> /dev/null; then
    echo -e "${GREEN}✓ ollamaCode is ready to use!${NC}"
    echo ""
    echo -e "${BLUE}${BOLD}Quick Start:${NC}"
    echo -e "  ollamacode              # Start interactive mode"
    echo -e "  ollamacode --help       # Show help"
    echo -e "  ollamacode --list       # List available models"
    echo -e "  ollamacode \"Hello\"      # Send a prompt"
    echo ""
else
    echo -e "${RED}✗ Installation failed. Please check ${INSTALL_DIR} is in your PATH${NC}"
    exit 1
fi

# Optional: Start Ollama service
if ! pgrep -x "ollama" > /dev/null; then
    echo -e "${YELLOW}Ollama is not running. Would you like to start it? (y/n)${NC}"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        echo -e "${YELLOW}Starting Ollama...${NC}"
        brew services start ollama
        sleep 2
        echo -e "${GREEN}✓ Ollama started${NC}"
    fi
fi

echo ""
echo -e "${GREEN}${BOLD}Installation complete!${NC}"
echo -e "${BLUE}Run 'ollamacode' to get started${NC}"
