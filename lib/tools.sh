#!/bin/bash
#
# ollamaCode - Tool Functions Library
# Allows AI to execute commands on the local machine
#

# Tool execution safety
SAFE_MODE="${SAFE_MODE:-true}"
ALLOWED_COMMANDS=("ls" "cat" "grep" "find" "git" "docker" "kubectl" "systemctl" "journalctl")

# Execute shell command with safety checks
tool_bash() {
    local command="$1"
    local description="${2:-Execute command}"

    echo -e "${YELLOW}🔧 Tool: Bash${NC}"
    echo -e "${CYAN}Description: ${description}${NC}"
    echo -e "${MAGENTA}Command: ${command}${NC}"

    if [[ "${SAFE_MODE}" == "true" ]]; then
        # Check if command is in allowed list
        local cmd_name=$(echo "${command}" | awk '{print $1}')
        local is_allowed=false

        for allowed in "${ALLOWED_COMMANDS[@]}"; do
            if [[ "${cmd_name}" == "${allowed}" ]] || [[ "${cmd_name}" == *"${allowed}"* ]]; then
                is_allowed=true
                break
            fi
        done

        if [[ "${is_allowed}" == "false" ]]; then
            echo -e "${RED}✗ Command '${cmd_name}' not in allowed list${NC}"
            echo -e "${YELLOW}Run in unsafe mode: SAFE_MODE=false ollamacode${NC}"
            return 1
        fi
    fi

    echo -e "${YELLOW}Execute this command? (y/n/always)${NC}"
    read -r confirm

    case "${confirm}" in
        y|Y|yes|Yes)
            echo -e "${GREEN}Executing...${NC}"
            eval "${command}"
            local exit_code=$?
            if [[ ${exit_code} -eq 0 ]]; then
                echo -e "${GREEN}✓ Command completed successfully${NC}"
            else
                echo -e "${RED}✗ Command failed with exit code: ${exit_code}${NC}"
            fi
            return ${exit_code}
            ;;
        a|A|always|Always)
            SAFE_MODE="false"
            echo -e "${YELLOW}⚠ Safe mode disabled for this session${NC}"
            eval "${command}"
            return $?
            ;;
        *)
            echo -e "${RED}✗ Command execution cancelled${NC}"
            return 1
            ;;
    esac
}

# Read file with syntax highlighting (if bat is available)
tool_read() {
    local file_path="$1"

    echo -e "${YELLOW}🔧 Tool: Read${NC}"
    echo -e "${CYAN}File: ${file_path}${NC}"

    if [[ ! -f "${file_path}" ]]; then
        echo -e "${RED}✗ File not found: ${file_path}${NC}"
        return 1
    fi

    if command -v bat &> /dev/null; then
        bat --style=numbers --color=always "${file_path}"
    else
        cat -n "${file_path}"
    fi
}

# Write content to file
tool_write() {
    local file_path="$1"
    local content="$2"

    echo -e "${YELLOW}🔧 Tool: Write${NC}"
    echo -e "${CYAN}File: ${file_path}${NC}"

    if [[ -f "${file_path}" ]]; then
        echo -e "${YELLOW}⚠ File exists. Overwrite? (y/n)${NC}"
        read -r confirm
        if [[ ! "${confirm}" =~ ^[Yy]$ ]]; then
            echo -e "${RED}✗ Write cancelled${NC}"
            return 1
        fi
    fi

    echo "${content}" > "${file_path}"
    if [[ $? -eq 0 ]]; then
        echo -e "${GREEN}✓ File written successfully${NC}"
    else
        echo -e "${RED}✗ Failed to write file${NC}"
        return 1
    fi
}

# Search for files
tool_glob() {
    local pattern="$1"
    local path="${2:-.}"

    echo -e "${YELLOW}🔧 Tool: Glob${NC}"
    echo -e "${CYAN}Pattern: ${pattern}${NC}"
    echo -e "${CYAN}Path: ${path}${NC}"

    find "${path}" -name "${pattern}" -type f 2>/dev/null | head -50
}

# Search in files
tool_grep() {
    local pattern="$1"
    local path="${2:-.}"

    echo -e "${YELLOW}🔧 Tool: Grep${NC}"
    echo -e "${CYAN}Pattern: ${pattern}${NC}"
    echo -e "${CYAN}Path: ${path}${NC}"

    if command -v rg &> /dev/null; then
        rg --color=always -n "${pattern}" "${path}" | head -50
    else
        grep -r -n --color=always "${pattern}" "${path}" 2>/dev/null | head -50
    fi
}

# Git operations
tool_git() {
    local operation="$1"
    shift
    local args="$@"

    echo -e "${YELLOW}🔧 Tool: Git${NC}"
    echo -e "${CYAN}Operation: ${operation}${NC}"

    case "${operation}" in
        status)
            git status
            ;;
        diff)
            git diff "${args}"
            ;;
        log)
            git log --oneline --graph --decorate -10 "${args}"
            ;;
        add)
            echo -e "${YELLOW}Add files: ${args}${NC}"
            echo -e "${YELLOW}Confirm? (y/n)${NC}"
            read -r confirm
            if [[ "${confirm}" =~ ^[Yy]$ ]]; then
                git add ${args}
                echo -e "${GREEN}✓ Files staged${NC}"
            fi
            ;;
        *)
            echo -e "${RED}✗ Unknown git operation: ${operation}${NC}"
            return 1
            ;;
    esac
}

# System information
tool_sysinfo() {
    echo -e "${YELLOW}🔧 Tool: System Information${NC}"
    echo ""
    echo -e "${CYAN}${BOLD}OS:${NC} $(uname -s)"
    echo -e "${CYAN}${BOLD}Kernel:${NC} $(uname -r)"
    echo -e "${CYAN}${BOLD}Architecture:${NC} $(uname -m)"
    echo -e "${CYAN}${BOLD}Hostname:${NC} $(hostname)"
    echo -e "${CYAN}${BOLD}CPU:${NC} $(nproc) cores"
    echo -e "${CYAN}${BOLD}Memory:${NC} $(free -h | awk '/^Mem:/ {print $2}' 2>/dev/null || echo 'N/A')"
    echo -e "${CYAN}${BOLD}Disk:${NC} $(df -h / | awk 'NR==2 {print $4 " free"}' 2>/dev/null || echo 'N/A')"
}

# Docker operations
tool_docker() {
    local operation="$1"
    shift
    local args="$@"

    echo -e "${YELLOW}🔧 Tool: Docker${NC}"
    echo -e "${CYAN}Operation: ${operation}${NC}"

    if ! command -v docker &> /dev/null; then
        echo -e "${RED}✗ Docker not installed${NC}"
        return 1
    fi

    case "${operation}" in
        ps)
            docker ps "${args}"
            ;;
        images)
            docker images "${args}"
            ;;
        logs)
            docker logs "${args}"
            ;;
        *)
            echo -e "${RED}✗ Unknown docker operation: ${operation}${NC}"
            return 1
            ;;
    esac
}

# Process information
tool_ps() {
    local pattern="${1:-}"

    echo -e "${YELLOW}🔧 Tool: Process List${NC}"

    if [[ -n "${pattern}" ]]; then
        ps aux | grep -v grep | grep -i "${pattern}"
    else
        ps aux | head -20
    fi
}

# Network information
tool_netstat() {
    echo -e "${YELLOW}🔧 Tool: Network Status${NC}"

    if command -v ss &> /dev/null; then
        ss -tulpn 2>/dev/null || ss -tuln
    elif command -v netstat &> /dev/null; then
        netstat -tulpn 2>/dev/null || netstat -tuln
    else
        echo -e "${RED}✗ No network tools available${NC}"
        return 1
    fi
}

# Export tools
export -f tool_bash
export -f tool_read
export -f tool_write
export -f tool_glob
export -f tool_grep
export -f tool_git
export -f tool_sysinfo
export -f tool_docker
export -f tool_ps
export -f tool_netstat
