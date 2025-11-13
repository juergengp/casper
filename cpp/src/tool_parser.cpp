#include "tool_parser.h"
#include "utils.h"
#include <regex>
#include <iostream>
#include <sstream>

namespace ollamacode {

std::string ToolParser::extractTag(const std::string& text, const std::string& tag) {
    std::string openTag = "<" + tag + ">";
    std::string closeTag = "</" + tag + ">";

    size_t start = text.find(openTag);
    if (start == std::string::npos) return "";

    start += openTag.length();
    size_t end = text.find(closeTag, start);
    if (end == std::string::npos) return "";

    return text.substr(start, end - start);
}

std::vector<std::string> ToolParser::extractAllTags(const std::string& text, const std::string& tag) {
    std::vector<std::string> results;
    std::string openTag = "<" + tag + ">";
    std::string closeTag = "</" + tag + ">";

    size_t pos = 0;
    while (true) {
        size_t start = text.find(openTag, pos);
        if (start == std::string::npos) break;

        start += openTag.length();
        size_t end = text.find(closeTag, start);
        if (end == std::string::npos) break;

        results.push_back(text.substr(start, end - start));
        pos = end + closeTag.length();
    }

    return results;
}

std::string ToolParser::extractParameter(const std::string& text, const std::string& param) {
    std::string openTag = "<" + param + ">";
    std::string closeTag = "</" + param + ">";

    size_t start = text.find(openTag);
    if (start == std::string::npos) return "";

    start += openTag.length();
    size_t end = text.find(closeTag, start);
    if (end == std::string::npos) return "";

    std::string value = text.substr(start, end - start);
    return utils::trim(value);
}

bool ToolParser::hasToolCalls(const std::string& response) {
    return response.find("<tool_calls>") != std::string::npos;
}

std::vector<ToolCall> ToolParser::parseToolCalls(const std::string& response) {
    std::vector<ToolCall> toolCalls;

    if (!hasToolCalls(response)) {
        return toolCalls;
    }

    // Extract the tool_calls block
    std::string toolCallsBlock = extractTag(response, "tool_calls");
    if (toolCallsBlock.empty()) {
        return toolCalls;
    }

    // Extract all individual tool_call blocks
    std::vector<std::string> toolCallBlocks = extractAllTags(toolCallsBlock, "tool_call");

    for (const auto& block : toolCallBlocks) {
        ToolCall toolCall;

        // Extract tool name
        toolCall.name = extractTag(block, "tool_name");
        if (toolCall.name.empty()) {
            continue; // Skip invalid tool calls
        }

        // Extract parameters block
        std::string parametersBlock = extractTag(block, "parameters");
        if (!parametersBlock.empty()) {
            // Common parameters to extract
            std::vector<std::string> paramNames = {
                "command", "description", "file_path", "content",
                "old_string", "new_string", "pattern", "path", "output_mode"
            };

            for (const auto& paramName : paramNames) {
                std::string value = extractParameter(parametersBlock, paramName);
                if (!value.empty()) {
                    toolCall.parameters[paramName] = value;
                }
            }
        }

        toolCalls.push_back(toolCall);
    }

    return toolCalls;
}

std::string ToolParser::extractResponseText(const std::string& response) {
    if (!hasToolCalls(response)) {
        return response;
    }

    // Find the tool_calls block
    size_t toolCallsStart = response.find("<tool_calls>");
    size_t toolCallsEnd = response.find("</tool_calls>");

    if (toolCallsStart == std::string::npos || toolCallsEnd == std::string::npos) {
        return response;
    }

    toolCallsEnd += std::string("</tool_calls>").length();

    // Get text before and after tool_calls
    std::string before = response.substr(0, toolCallsStart);
    std::string after = response.substr(toolCallsEnd);

    // Combine and trim
    std::string result = utils::trim(before + "\n" + after);

    // Remove empty lines
    std::string cleaned;
    std::istringstream stream(result);
    std::string line;
    while (std::getline(stream, line)) {
        line = utils::trim(line);
        if (!line.empty()) {
            cleaned += line + "\n";
        }
    }

    return utils::trim(cleaned);
}

} // namespace ollamacode
