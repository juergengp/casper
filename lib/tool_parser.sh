#!/bin/bash
#
# ollamaCode - Tool Call Parser
# Parses AI responses for tool calls
#

# Extract tool calls from AI response
parse_tool_calls() {
    local response="$1"
    local temp_file=$(mktemp)

    # Check if response contains tool calls
    if ! echo "$response" | grep -q "<tool_calls>"; then
        echo "[]"
        return 0
    fi

    # Extract the tool_calls section
    echo "$response" | sed -n '/<tool_calls>/,/<\/tool_calls>/p' > "$temp_file"

    # Count tool calls
    local tool_count=$(grep -c "<tool_call>" "$temp_file")

    if [[ $tool_count -eq 0 ]]; then
        echo "[]"
        rm -f "$temp_file"
        return 0
    fi

    # Output path to temp file for processing
    echo "$temp_file"
}

# Extract a specific tool call by index (0-based)
extract_tool_call() {
    local temp_file="$1"
    local index="$2"

    # Extract all tool_call blocks
    local current=0
    local in_tool_call=false
    local tool_call_content=""

    while IFS= read -r line; do
        if [[ "$line" =~ \<tool_call\> ]]; then
            in_tool_call=true
            tool_call_content=""
            continue
        elif [[ "$line" =~ \</tool_call\> ]]; then
            if [[ $current -eq $index ]]; then
                echo "$tool_call_content"
                return 0
            fi
            current=$((current + 1))
            in_tool_call=false
            continue
        fi

        if [[ "$in_tool_call" == "true" ]]; then
            tool_call_content+="$line"$'\n'
        fi
    done < "$temp_file"

    return 1
}

# Parse tool name from tool call
parse_tool_name() {
    local tool_call="$1"
    echo "$tool_call" | sed -n 's/.*<tool_name>\(.*\)<\/tool_name>.*/\1/p' | tr -d '[:space:]'
}

# Parse parameter value from tool call
parse_parameter() {
    local tool_call="$1"
    local param_name="$2"

    # Extract parameter value, preserving newlines and content
    echo "$tool_call" | sed -n "/<${param_name}>/,/<\/${param_name}>/p" | \
        sed "1s/.*<${param_name}>//; \$s/<\/${param_name}>.*//" | \
        sed '1s/^[[:space:]]*//; $s/[[:space:]]*$//'
}

# Count total tool calls
count_tool_calls() {
    local temp_file="$1"

    if [[ ! -f "$temp_file" ]] || [[ "$temp_file" == "[]" ]]; then
        echo "0"
        return
    fi

    grep -c "<tool_call>" "$temp_file"
}

# Get non-tool response text (text outside of tool_calls blocks)
get_response_text() {
    local response="$1"

    # If no tool calls, return entire response
    if ! echo "$response" | grep -q "<tool_calls>"; then
        echo "$response"
        return 0
    fi

    # Extract text before tool_calls
    local before=$(echo "$response" | sed -n '1,/<tool_calls>/p' | sed '$d')

    # Extract text after tool_calls
    local after=$(echo "$response" | sed -n '/<\/tool_calls>/,$p' | sed '1d')

    # Combine and clean up
    printf "%s\n%s" "$before" "$after" | sed '/^[[:space:]]*$/d'
}

export -f parse_tool_calls
export -f extract_tool_call
export -f parse_tool_name
export -f parse_parameter
export -f count_tool_calls
export -f get_response_text
