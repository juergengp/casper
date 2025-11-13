#include "tool_executor.h"
#include "config.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <array>

namespace ollamacode {

ToolExecutor::ToolExecutor(Config& config)
    : config_(config)
    , confirm_callback_(nullptr)
{
}

void ToolExecutor::setConfirmCallback(ConfirmCallback callback) {
    confirm_callback_ = callback;
}

bool ToolExecutor::isCommandSafe(const std::string& command) {
    if (!config_.getSafeMode()) {
        return true;
    }

    return config_.isCommandAllowed(command);
}

bool ToolExecutor::requestConfirmation(const std::string& tool_name, const std::string& description) {
    if (config_.getAutoApprove()) {
        return true;
    }

    if (confirm_callback_) {
        return confirm_callback_(tool_name, description);
    }

    // Default confirmation
    std::cout << utils::terminal::YELLOW << "Execute " << tool_name << "? (y/n): " << utils::terminal::RESET;
    char response;
    std::cin >> response;
    std::cin.ignore(); // Clear newline
    return (response == 'y' || response == 'Y');
}

std::string ToolExecutor::executeCommand(const std::string& command, int& exit_code) {
    std::array<char, 128> buffer;
    std::string result;

    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (!pipe) {
        exit_code = -1;
        return "Failed to execute command";
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    exit_code = pclose(pipe);
    exit_code = WEXITSTATUS(exit_code);

    return result;
}

ToolResult ToolExecutor::executeBash(const ToolCall& tool_call) {
    ToolResult result;

    auto it = tool_call.parameters.find("command");
    if (it == tool_call.parameters.end()) {
        result.success = false;
        result.error = "Missing 'command' parameter";
        return result;
    }

    std::string command = it->second;
    std::string description = "Execute command";

    auto desc_it = tool_call.parameters.find("description");
    if (desc_it != tool_call.parameters.end()) {
        description = desc_it->second;
    }

    utils::terminal::printInfo("[Tool: Bash]");
    std::cout << utils::terminal::CYAN << "Description: " << description << utils::terminal::RESET << "\n";
    std::cout << utils::terminal::MAGENTA << "Command: " << command << utils::terminal::RESET << "\n\n";

    // Safety check
    if (!isCommandSafe(command)) {
        result.success = false;
        result.error = "Command not allowed in safe mode";
        utils::terminal::printError(result.error);
        return result;
    }

    // Confirmation
    if (!requestConfirmation("Bash", description)) {
        result.success = false;
        result.error = "Cancelled by user";
        utils::terminal::printError("Cancelled");
        return result;
    }

    // Execute
    utils::terminal::printInfo("Executing...");
    result.output = executeCommand(command, result.exit_code);
    result.success = (result.exit_code == 0);

    if (result.success) {
        utils::terminal::printSuccess("Success");
    } else {
        utils::terminal::printError("Failed (exit code: " + std::to_string(result.exit_code) + ")");
    }

    std::cout << "\n=== Output ===\n" << result.output << "==============\n\n";

    return result;
}

ToolResult ToolExecutor::executeRead(const ToolCall& tool_call) {
    ToolResult result;

    auto it = tool_call.parameters.find("file_path");
    if (it == tool_call.parameters.end()) {
        result.success = false;
        result.error = "Missing 'file_path' parameter";
        return result;
    }

    std::string file_path = it->second;

    utils::terminal::printInfo("[Tool: Read]");
    std::cout << utils::terminal::CYAN << "File: " << file_path << utils::terminal::RESET << "\n\n";

    if (!utils::fileExists(file_path)) {
        result.success = false;
        result.error = "File not found: " + file_path;
        utils::terminal::printError(result.error);
        return result;
    }

    result.output = utils::readFile(file_path);
    if (result.output.empty()) {
        result.success = false;
        result.error = "Failed to read file or file is empty";
        utils::terminal::printError(result.error);
        return result;
    }

    result.success = true;
    result.exit_code = 0;

    std::cout << "=== File Contents ===\n" << result.output << "\n====================\n\n";

    return result;
}

ToolResult ToolExecutor::executeWrite(const ToolCall& tool_call) {
    ToolResult result;

    auto path_it = tool_call.parameters.find("file_path");
    auto content_it = tool_call.parameters.find("content");

    if (path_it == tool_call.parameters.end() || content_it == tool_call.parameters.end()) {
        result.success = false;
        result.error = "Missing 'file_path' or 'content' parameter";
        return result;
    }

    std::string file_path = path_it->second;
    std::string content = content_it->second;

    utils::terminal::printInfo("[Tool: Write]");
    std::cout << utils::terminal::CYAN << "File: " << file_path << utils::terminal::RESET << "\n\n";

    // Create parent directory if needed
    std::string dir = utils::getDirname(file_path);
    if (!utils::dirExists(dir)) {
        if (!utils::createDir(dir)) {
            result.success = false;
            result.error = "Failed to create directory: " + dir;
            utils::terminal::printError(result.error);
            return result;
        }
    }

    // Confirm if file exists
    if (utils::fileExists(file_path)) {
        if (!requestConfirmation("Write", "File exists. Overwrite?")) {
            result.success = false;
            result.error = "Cancelled by user";
            utils::terminal::printError("Cancelled");
            return result;
        }
    }

    // Write file
    if (!utils::writeFile(file_path, content)) {
        result.success = false;
        result.error = "Failed to write file";
        utils::terminal::printError(result.error);
        return result;
    }

    result.success = true;
    result.exit_code = 0;
    result.output = "File written successfully";

    utils::terminal::printSuccess(result.output);
    std::cout << utils::terminal::CYAN << "Lines written: " << std::count(content.begin(), content.end(), '\n') + 1 << utils::terminal::RESET << "\n\n";

    return result;
}

ToolResult ToolExecutor::executeEdit(const ToolCall& tool_call) {
    ToolResult result;

    auto path_it = tool_call.parameters.find("file_path");
    auto old_it = tool_call.parameters.find("old_string");
    auto new_it = tool_call.parameters.find("new_string");

    if (path_it == tool_call.parameters.end() || old_it == tool_call.parameters.end() || new_it == tool_call.parameters.end()) {
        result.success = false;
        result.error = "Missing required parameters";
        return result;
    }

    std::string file_path = path_it->second;
    std::string old_string = old_it->second;
    std::string new_string = new_it->second;

    utils::terminal::printInfo("[Tool: Edit]");
    std::cout << utils::terminal::CYAN << "File: " << file_path << utils::terminal::RESET << "\n\n";

    if (!utils::fileExists(file_path)) {
        result.success = false;
        result.error = "File not found: " + file_path;
        utils::terminal::printError(result.error);
        return result;
    }

    // Read file
    std::string content = utils::readFile(file_path);
    if (content.empty()) {
        result.success = false;
        result.error = "Failed to read file";
        utils::terminal::printError(result.error);
        return result;
    }

    // Check if old_string exists
    if (content.find(old_string) == std::string::npos) {
        result.success = false;
        result.error = "String not found in file";
        utils::terminal::printError(result.error);
        return result;
    }

    // Count occurrences
    size_t count = 0;
    size_t pos = 0;
    while ((pos = content.find(old_string, pos)) != std::string::npos) {
        count++;
        pos += old_string.length();
    }

    std::cout << utils::terminal::CYAN << "Found " << count << " occurrence(s)" << utils::terminal::RESET << "\n\n";

    // Confirm
    if (!requestConfirmation("Edit", "Apply changes?")) {
        result.success = false;
        result.error = "Cancelled by user";
        utils::terminal::printError("Cancelled");
        return result;
    }

    // Create backup
    std::string backup_path = file_path + ".bak";
    utils::writeFile(backup_path, content);

    // Perform replacement
    pos = 0;
    while ((pos = content.find(old_string, pos)) != std::string::npos) {
        content.replace(pos, old_string.length(), new_string);
        pos += new_string.length();
    }

    // Write modified content
    if (!utils::writeFile(file_path, content)) {
        result.success = false;
        result.error = "Failed to write file";
        utils::terminal::printError(result.error);
        return result;
    }

    result.success = true;
    result.exit_code = 0;
    result.output = "File edited successfully";

    utils::terminal::printSuccess(result.output);
    std::cout << utils::terminal::CYAN << "Backup saved: " << backup_path << utils::terminal::RESET << "\n\n";

    return result;
}

ToolResult ToolExecutor::executeGlob(const ToolCall& tool_call) {
    ToolResult result;

    auto pattern_it = tool_call.parameters.find("pattern");
    if (pattern_it == tool_call.parameters.end()) {
        result.success = false;
        result.error = "Missing 'pattern' parameter";
        return result;
    }

    std::string pattern = pattern_it->second;
    std::string path = ".";

    auto path_it = tool_call.parameters.find("path");
    if (path_it != tool_call.parameters.end()) {
        path = path_it->second;
    }

    utils::terminal::printInfo("[Tool: Glob]");
    std::cout << utils::terminal::CYAN << "Pattern: " << pattern << utils::terminal::RESET << "\n";
    std::cout << utils::terminal::CYAN << "Path: " << path << utils::terminal::RESET << "\n\n";

    // Use find command
    std::string command = "find " + path + " -name '" + pattern + "' -type f 2>/dev/null | head -100";
    result.output = executeCommand(command, result.exit_code);
    result.success = true;

    std::cout << "=== Matching Files ===\n" << result.output << "=====================\n\n";

    return result;
}

ToolResult ToolExecutor::executeGrep(const ToolCall& tool_call) {
    ToolResult result;

    auto pattern_it = tool_call.parameters.find("pattern");
    if (pattern_it == tool_call.parameters.end()) {
        result.success = false;
        result.error = "Missing 'pattern' parameter";
        return result;
    }

    std::string pattern = pattern_it->second;
    std::string path = ".";
    std::string output_mode = "files_with_matches";

    auto path_it = tool_call.parameters.find("path");
    if (path_it != tool_call.parameters.end()) {
        path = path_it->second;
    }

    auto mode_it = tool_call.parameters.find("output_mode");
    if (mode_it != tool_call.parameters.end()) {
        output_mode = mode_it->second;
    }

    utils::terminal::printInfo("[Tool: Grep]");
    std::cout << utils::terminal::CYAN << "Pattern: " << pattern << utils::terminal::RESET << "\n";
    std::cout << utils::terminal::CYAN << "Path: " << path << utils::terminal::RESET << "\n";
    std::cout << utils::terminal::CYAN << "Mode: " << output_mode << utils::terminal::RESET << "\n\n";

    // Build grep command
    std::string command;
    if (output_mode == "content") {
        command = "grep -r -n '" + pattern + "' " + path + " 2>/dev/null | head -100";
    } else {
        command = "grep -r -l '" + pattern + "' " + path + " 2>/dev/null | head -100";
    }

    result.output = executeCommand(command, result.exit_code);
    result.success = true;

    std::cout << "=== Search Results ===\n" << result.output << "=====================\n\n";

    return result;
}

ToolResult ToolExecutor::execute(const ToolCall& tool_call) {
    if (tool_call.name == "Bash") {
        return executeBash(tool_call);
    } else if (tool_call.name == "Read") {
        return executeRead(tool_call);
    } else if (tool_call.name == "Write") {
        return executeWrite(tool_call);
    } else if (tool_call.name == "Edit") {
        return executeEdit(tool_call);
    } else if (tool_call.name == "Glob") {
        return executeGlob(tool_call);
    } else if (tool_call.name == "Grep") {
        return executeGrep(tool_call);
    } else {
        ToolResult result;
        result.success = false;
        result.error = "Unknown tool: " + tool_call.name;
        utils::terminal::printError(result.error);
        return result;
    }
}

std::vector<ToolResult> ToolExecutor::executeAll(const std::vector<ToolCall>& tool_calls) {
    std::vector<ToolResult> results;

    for (size_t i = 0; i < tool_calls.size(); i++) {
        std::cout << utils::terminal::MAGENTA << "═══════════════════════════════════════" << utils::terminal::RESET << "\n";
        std::cout << utils::terminal::MAGENTA << "Tool " << (i+1) << "/" << tool_calls.size() << ": " << tool_calls[i].name << utils::terminal::RESET << "\n";
        std::cout << utils::terminal::MAGENTA << "═══════════════════════════════════════" << utils::terminal::RESET << "\n\n";

        results.push_back(execute(tool_calls[i]));
    }

    return results;
}

} // namespace ollamacode
