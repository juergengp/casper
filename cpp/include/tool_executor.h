#ifndef OLLAMACODE_TOOL_EXECUTOR_H
#define OLLAMACODE_TOOL_EXECUTOR_H

#include <string>
#include <functional>
#include "tool_parser.h"

namespace ollamacode {

class Config; // Forward declaration

struct ToolResult {
    bool success;
    int exit_code;
    std::string output;
    std::string error;
};

class ToolExecutor {
public:
    explicit ToolExecutor(Config& config);

    // Execute a single tool call
    ToolResult execute(const ToolCall& tool_call);

    // Execute multiple tool calls
    std::vector<ToolResult> executeAll(const std::vector<ToolCall>& tool_calls);

    // Set confirmation callback (for interactive mode)
    using ConfirmCallback = std::function<bool(const std::string&, const std::string&)>;
    void setConfirmCallback(ConfirmCallback callback);

private:
    Config& config_;
    ConfirmCallback confirm_callback_;

    // Tool implementations
    ToolResult executeBash(const ToolCall& tool_call);
    ToolResult executeRead(const ToolCall& tool_call);
    ToolResult executeWrite(const ToolCall& tool_call);
    ToolResult executeEdit(const ToolCall& tool_call);
    ToolResult executeGlob(const ToolCall& tool_call);
    ToolResult executeGrep(const ToolCall& tool_call);

    // Helpers
    bool isCommandSafe(const std::string& command);
    bool requestConfirmation(const std::string& tool_name, const std::string& description);
    std::string executeCommand(const std::string& command, int& exit_code);
};

} // namespace ollamacode

#endif // OLLAMACODE_TOOL_EXECUTOR_H
