#!/bin/bash
#
# ollamaCode - Tool Executor
# Executes tool calls and returns results
#

# Source the tool parser
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/tool_parser.sh"

# Tool execution safety
SAFE_MODE="${SAFE_MODE:-true}"
AUTO_APPROVE="${AUTO_APPROVE:-false}"
ALLOWED_COMMANDS=("ls" "cat" "head" "tail" "grep" "find" "git" "docker" "kubectl" "systemctl" "journalctl" "pwd" "whoami" "date" "echo" "which" "ps" "df" "du" "wc" "sort" "uniq")

# Execute Bash tool
execute_bash_tool() {
    local command="$1"
    local description="${2:-Execute command}"

    echo -e "${YELLOW}[Tool: Bash]${NC}"
    echo -e "${CYAN}Description: ${description}${NC}"
    echo -e "${MAGENTA}Command: ${command}${NC}"
    echo ""

    # Safety checks
    if [[ "${SAFE_MODE}" == "true" ]]; then
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
            echo -e "${YELLOW}To allow: Set SAFE_MODE=false${NC}"
            return 1
        fi
    fi

    # Confirmation
    if [[ "${AUTO_APPROVE}" != "true" ]]; then
        echo -e "${YELLOW}Execute? (y/n/always): ${NC}"
        read -r confirm
        case "${confirm}" in
            a|A|always|Always)
                AUTO_APPROVE="true"
                ;;
            y|Y|yes|Yes)
                ;;
            *)
                echo -e "${RED}✗ Cancelled${NC}"
                return 1
                ;;
        esac
    fi

    # Execute
    echo -e "${GREEN}Executing...${NC}"
    local output
    local exit_code

    output=$(eval "${command}" 2>&1)
    exit_code=$?

    if [[ ${exit_code} -eq 0 ]]; then
        echo -e "${GREEN}✓ Success${NC}"
    else
        echo -e "${RED}✗ Failed (exit code: ${exit_code})${NC}"
    fi

    echo ""
    echo "=== Output ==="
    echo "$output"
    echo "=============="
    echo ""

    return ${exit_code}
}

# Execute Read tool
execute_read_tool() {
    local file_path="$1"

    echo -e "${YELLOW}[Tool: Read]${NC}"
    echo -e "${CYAN}File: ${file_path}${NC}"
    echo ""

    if [[ ! -f "${file_path}" ]]; then
        echo -e "${RED}✗ File not found: ${file_path}${NC}"
        return 1
    fi

    if [[ ! -r "${file_path}" ]]; then
        echo -e "${RED}✗ Permission denied: ${file_path}${NC}"
        return 1
    fi

    echo "=== File Contents ==="
    if command -v bat &> /dev/null; then
        bat --style=numbers --color=always "${file_path}"
    else
        cat -n "${file_path}"
    fi
    echo "===================="
    echo ""

    return 0
}

# Execute Write tool
execute_write_tool() {
    local file_path="$1"
    local content="$2"

    echo -e "${YELLOW}[Tool: Write]${NC}"
    echo -e "${CYAN}File: ${file_path}${NC}"
    echo ""

    # Create parent directory if needed
    local parent_dir=$(dirname "${file_path}")
    if [[ ! -d "${parent_dir}" ]]; then
        echo -e "${YELLOW}Creating directory: ${parent_dir}${NC}"
        mkdir -p "${parent_dir}"
    fi

    # Confirm if file exists
    if [[ -f "${file_path}" ]]; then
        echo -e "${YELLOW}⚠ File exists. Overwrite? (y/n): ${NC}"
        if [[ "${AUTO_APPROVE}" != "true" ]]; then
            read -r confirm
            if [[ ! "${confirm}" =~ ^[Yy]$ ]]; then
                echo -e "${RED}✗ Cancelled${NC}"
                return 1
            fi
        fi
    fi

    # Write file
    echo "$content" > "${file_path}"
    if [[ $? -eq 0 ]]; then
        echo -e "${GREEN}✓ File written successfully${NC}"
        echo -e "${CYAN}Lines written: $(wc -l < "${file_path}")${NC}"
        echo ""
        return 0
    else
        echo -e "${RED}✗ Failed to write file${NC}"
        return 1
    fi
}

# Execute Edit tool
execute_edit_tool() {
    local file_path="$1"
    local old_string="$2"
    local new_string="$3"

    echo -e "${YELLOW}[Tool: Edit]${NC}"
    echo -e "${CYAN}File: ${file_path}${NC}"
    echo ""

    if [[ ! -f "${file_path}" ]]; then
        echo -e "${RED}✗ File not found: ${file_path}${NC}"
        return 1
    fi

    if [[ ! -w "${file_path}" ]]; then
        echo -e "${RED}✗ Permission denied: ${file_path}${NC}"
        return 1
    fi

    # Check if old_string exists
    if ! grep -qF "$old_string" "${file_path}"; then
        echo -e "${RED}✗ String not found in file${NC}"
        echo -e "${YELLOW}Looking for: ${old_string}${NC}"
        return 1
    fi

    # Count occurrences
    local count=$(grep -cF "$old_string" "${file_path}")
    echo -e "${CYAN}Found ${count} occurrence(s)${NC}"

    # Show preview
    echo ""
    echo "=== Changes Preview ==="
    grep -nF "$old_string" "${file_path}" | head -5
    echo "======================="
    echo ""

    # Confirm
    if [[ "${AUTO_APPROVE}" != "true" ]]; then
        echo -e "${YELLOW}Apply changes? (y/n): ${NC}"
        read -r confirm
        if [[ ! "${confirm}" =~ ^[Yy]$ ]]; then
            echo -e "${RED}✗ Cancelled${NC}"
            return 1
        fi
    fi

    # Create backup
    cp "${file_path}" "${file_path}.bak"

    # Perform replacement
    # Use a temporary file for safe replacement
    local temp_file=$(mktemp)
    awk -v old="$old_string" -v new="$new_string" '{gsub(old, new); print}' "${file_path}" > "$temp_file"

    if [[ $? -eq 0 ]]; then
        mv "$temp_file" "${file_path}"
        echo -e "${GREEN}✓ File edited successfully${NC}"
        echo -e "${CYAN}Backup saved: ${file_path}.bak${NC}"
        echo ""
        return 0
    else
        echo -e "${RED}✗ Edit failed${NC}"
        rm -f "$temp_file"
        return 1
    fi
}

# Execute Glob tool
execute_glob_tool() {
    local pattern="$1"
    local path="${2:-.}"

    echo -e "${YELLOW}[Tool: Glob]${NC}"
    echo -e "${CYAN}Pattern: ${pattern}${NC}"
    echo -e "${CYAN}Path: ${path}${NC}"
    echo ""

    echo "=== Matching Files ==="
    find "${path}" -name "${pattern}" -type f 2>/dev/null | head -100
    echo "====================="
    echo ""

    return 0
}

# Execute Grep tool
execute_grep_tool() {
    local pattern="$1"
    local path="${2:-.}"
    local output_mode="${3:-files_with_matches}"

    echo -e "${YELLOW}[Tool: Grep]${NC}"
    echo -e "${CYAN}Pattern: ${pattern}${NC}"
    echo -e "${CYAN}Path: ${path}${NC}"
    echo -e "${CYAN}Mode: ${output_mode}${NC}"
    echo ""

    echo "=== Search Results ==="
    if [[ "$output_mode" == "content" ]]; then
        if command -v rg &> /dev/null; then
            rg --color=always -n "${pattern}" "${path}" 2>/dev/null | head -100
        else
            grep -r -n --color=always "${pattern}" "${path}" 2>/dev/null | head -100
        fi
    else
        # files_with_matches mode
        if command -v rg &> /dev/null; then
            rg -l "${pattern}" "${path}" 2>/dev/null | head -100
        else
            grep -r -l "${pattern}" "${path}" 2>/dev/null | head -100
        fi
    fi
    echo "====================="
    echo ""

    return 0
}

# Main tool executor
execute_tool() {
    local tool_name="$1"
    local tool_call_data="$2"

    case "${tool_name}" in
        Bash)
            local command=$(parse_parameter "$tool_call_data" "command")
            local description=$(parse_parameter "$tool_call_data" "description")
            execute_bash_tool "$command" "$description"
            ;;
        Read)
            local file_path=$(parse_parameter "$tool_call_data" "file_path")
            execute_read_tool "$file_path"
            ;;
        Write)
            local file_path=$(parse_parameter "$tool_call_data" "file_path")
            local content=$(parse_parameter "$tool_call_data" "content")
            execute_write_tool "$file_path" "$content"
            ;;
        Edit)
            local file_path=$(parse_parameter "$tool_call_data" "file_path")
            local old_string=$(parse_parameter "$tool_call_data" "old_string")
            local new_string=$(parse_parameter "$tool_call_data" "new_string")
            execute_edit_tool "$file_path" "$old_string" "$new_string"
            ;;
        Glob)
            local pattern=$(parse_parameter "$tool_call_data" "pattern")
            local path=$(parse_parameter "$tool_call_data" "path")
            execute_glob_tool "$pattern" "$path"
            ;;
        Grep)
            local pattern=$(parse_parameter "$tool_call_data" "pattern")
            local path=$(parse_parameter "$tool_call_data" "path")
            local output_mode=$(parse_parameter "$tool_call_data" "output_mode")
            execute_grep_tool "$pattern" "$path" "$output_mode"
            ;;
        *)
            echo -e "${RED}✗ Unknown tool: ${tool_name}${NC}"
            return 1
            ;;
    esac
}

export -f execute_bash_tool
export -f execute_read_tool
export -f execute_write_tool
export -f execute_edit_tool
export -f execute_glob_tool
export -f execute_grep_tool
export -f execute_tool
export AUTO_APPROVE
