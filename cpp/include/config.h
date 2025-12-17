#ifndef OLLAMACODE_CONFIG_H
#define OLLAMACODE_CONFIG_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sqlite3.h>

namespace ollamacode {

// MCP Server configuration structure
struct MCPServerConfig {
    std::string name;
    std::string command;
    std::vector<std::string> args;
    std::map<std::string, std::string> env;
    bool enabled;
    std::string transport;  // "stdio" or "http"
    std::string url;        // For HTTP transport
};

class Config {
public:
    Config();
    ~Config();

    // Initialize/load configuration
    bool initialize(const std::string& config_path = "");

    // Getters
    std::string getModel() const { return model_; }
    std::string getOllamaHost() const { return ollama_host_; }
    double getTemperature() const { return temperature_; }
    int getMaxTokens() const { return max_tokens_; }
    bool getSafeMode() const { return safe_mode_; }
    bool getAutoApprove() const { return auto_approve_; }
    bool getMCPEnabled() const { return mcp_enabled_; }

    // Setters
    void setModel(const std::string& model);
    void setOllamaHost(const std::string& host);
    void setTemperature(double temp);
    void setMaxTokens(int tokens);
    void setSafeMode(bool enabled);
    void setAutoApprove(bool enabled);
    void setMCPEnabled(bool enabled);

    // Persistence
    bool save();
    bool load();

    // Allowed commands for safe mode
    bool isCommandAllowed(const std::string& command) const;
    void addAllowedCommand(const std::string& command);

    // MCP Server management
    std::vector<MCPServerConfig> getMCPServers() const;
    bool addMCPServer(const MCPServerConfig& server);
    bool removeMCPServer(const std::string& name);
    bool enableMCPServer(const std::string& name, bool enabled);
    MCPServerConfig* getMCPServer(const std::string& name);

    // Get config directory path
    static std::string getConfigDir();
    static std::string getConfigPath();
    static std::string getHistoryPath();
    static std::string getMCPConfigPath();

private:
    void createDefaultConfig();
    void initializeDatabase();
    void loadAllowedCommands();
    void loadMCPServers();
    void saveMCPServers();

    sqlite3* db_;
    std::string config_path_;

    // Configuration values
    std::string model_;
    std::string ollama_host_;
    double temperature_;
    int max_tokens_;
    bool safe_mode_;
    bool auto_approve_;
    bool mcp_enabled_;

    // Allowed commands for safe mode
    std::vector<std::string> allowed_commands_;

    // MCP Servers
    std::vector<MCPServerConfig> mcp_servers_;
};

} // namespace ollamacode

#endif // OLLAMACODE_CONFIG_H
