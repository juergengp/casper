#!/bin/bash
#
# ollamaCode - Universal Installation Script
# Version: 1.0.0
# Supports: macOS, Fedora, RHEL, CentOS, Debian, Ubuntu
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
echo -e "${NC}${BLUE}Universal Installation Script - Version 1.0.0${NC}"
echo ""

# Detect OS
detect_os() {
    if [[ "$(uname)" == "Darwin" ]]; then
        OS="macos"
        PKG_MANAGER="brew"
    elif [[ -f /etc/fedora-release ]]; then
        OS="fedora"
        PKG_MANAGER="dnf"
    elif [[ -f /etc/redhat-release ]]; then
        OS="rhel"
        PKG_MANAGER="yum"
    elif [[ -f /etc/debian_version ]]; then
        OS="debian"
        PKG_MANAGER="apt"
    else
        OS="unknown"
        PKG_MANAGER="unknown"
    fi

    echo -e "${GREEN}Detected OS: ${OS}${NC}"
}

# Install dependencies
install_dependencies() {
    echo -e "${YELLOW}Installing dependencies...${NC}"

    case "${OS}" in
        macos)
            # Check for Homebrew
            if ! command -v brew &> /dev/null; then
                echo -e "${YELLOW}Homebrew not found. Installing Homebrew...${NC}"
                /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
            fi

            # Install jq if not present
            if ! command -v jq &> /dev/null; then
                echo -e "${YELLOW}Installing jq...${NC}"
                brew install jq
            fi

            # Check for Ollama
            if ! command -v ollama &> /dev/null; then
                echo -e "${YELLOW}Ollama not found. Would you like to install it? (y/n)${NC}"
                read -r response
                if [[ "$response" =~ ^[Yy]$ ]]; then
                    brew install ollama
                else
                    echo -e "${RED}✗ Ollama is required. Install from: https://ollama.ai${NC}"
                    exit 1
                fi
            fi
            ;;

        fedora|rhel)
            # Install jq if not present
            if ! command -v jq &> /dev/null; then
                echo -e "${YELLOW}Installing jq...${NC}"
                sudo ${PKG_MANAGER} install -y jq
            fi

            # Check for Ollama
            if ! command -v ollama &> /dev/null; then
                echo -e "${YELLOW}Ollama not found. Installing Ollama...${NC}"
                curl -fsSL https://ollama.ai/install.sh | sh
            fi
            ;;

        debian)
            # Update package list
            sudo apt update

            # Install jq if not present
            if ! command -v jq &> /dev/null; then
                echo -e "${YELLOW}Installing jq...${NC}"
                sudo apt install -y jq
            fi

            # Check for Ollama
            if ! command -v ollama &> /dev/null; then
                echo -e "${YELLOW}Ollama not found. Installing Ollama...${NC}"
                curl -fsSL https://ollama.ai/install.sh | sh
            fi
            ;;

        *)
            echo -e "${RED}✗ Unsupported OS. Please install dependencies manually:${NC}"
            echo -e "  - jq"
            echo -e "  - ollama (https://ollama.ai)"
            exit 1
            ;;
    esac

    echo -e "${GREEN}✓ All dependencies satisfied${NC}"
}

# Install ollamacode
install_ollamacode() {
    echo -e "${YELLOW}Installing ollamaCode...${NC}"

    # Find the script directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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

    # Copy library files if they exist
    if [[ -d "${SCRIPT_DIR}/lib" ]]; then
        LIB_DIR="/usr/local/lib/ollamacode"
        if [[ -w /usr/local/lib ]]; then
            mkdir -p "${LIB_DIR}"
            cp -r "${SCRIPT_DIR}/lib/"* "${LIB_DIR}/"
        else
            sudo mkdir -p "${LIB_DIR}"
            sudo cp -r "${SCRIPT_DIR}/lib/"* "${LIB_DIR}/"
        fi
        echo -e "${GREEN}✓ Library files installed to ${LIB_DIR}${NC}"
    fi
}

# Create configuration directory
create_config() {
    CONFIG_DIR="${HOME}/.config/ollamacode"
    mkdir -p "${CONFIG_DIR}"
    echo -e "${GREEN}✓ Configuration directory created at ${CONFIG_DIR}${NC}"
}

# Test installation
test_installation() {
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
}

# Start Ollama service if needed
start_ollama() {
    if ! pgrep -x "ollama" > /dev/null; then
        echo -e "${YELLOW}Ollama is not running. Would you like to start it? (y/n)${NC}"
        read -r response
        if [[ "$response" =~ ^[Yy]$ ]]; then
            echo -e "${YELLOW}Starting Ollama...${NC}"
            if [[ "${OS}" == "macos" ]]; then
                brew services start ollama
            else
                systemctl --user start ollama || ollama serve &
            fi
            sleep 2
            echo -e "${GREEN}✓ Ollama started${NC}"
        fi
    fi
}

# Main installation flow
main() {
    detect_os
    echo ""
    install_dependencies
    echo ""
    install_ollamacode
    echo ""
    create_config
    test_installation
    start_ollama
    echo ""
    echo -e "${GREEN}${BOLD}Installation complete!${NC}"
    echo -e "${BLUE}Run 'ollamacode' to get started${NC}"
}

main
