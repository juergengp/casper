#include "config.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include "json.hpp"

using json = nlohmann::json;

namespace ollamacode {

Config::Config()
    : db_(nullptr)
    , model_("llama3")
    , ollama_host_("http://localhost:11434")
    , temperature_(0.7)
    , max_tokens_(4096)
    , safe_mode_(true)
    , auto_approve_(false)
    , mcp_enabled_(false)
{
    // Default allowed commands
    allowed_commands_ = {
        "ls", "cat", "head", "tail", "grep", "find", "git", "docker",
        "kubectl", "systemctl", "journalctl", "pwd", "whoami", "date",
        "echo", "which", "ps", "df", "du", "wc", "sort", "uniq", "tree"
    };
}

Config::~Config() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool Config::initialize(const std::string& config_path) {
    if (config_path.empty()) {
        config_path_ = getConfigPath();
    } else {
        config_path_ = config_path;
    }

    // Create config directory if it doesn't exist
    std::string config_dir = getConfigDir();
    if (!utils::dirExists(config_dir)) {
        if (!utils::createDir(config_dir)) {
            std::cerr << "Failed to create config directory: " << config_dir << std::endl;
            return false;
        }
    }

    // Initialize database
    initializeDatabase();

    // Load configuration
    return load();
}

void Config::initializeDatabase() {
    int rc = sqlite3_open(config_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << std::endl;
        return;
    }

    // Create tables
    const char* create_config_table = R"(
        CREATE TABLE IF NOT EXISTS config (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        );
    )";

    const char* create_commands_table = R"(
        CREATE TABLE IF NOT EXISTS allowed_commands (
            command TEXT PRIMARY KEY
        );
    )";

    const char* create_history_table = R"(
        CREATE TABLE IF NOT EXISTS history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            prompt TEXT NOT NULL,
            response TEXT
        );
    )";

    char* err_msg = nullptr;

    sqlite3_exec(db_, create_config_table, nullptr, nullptr, &err_msg);
    if (err_msg) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }

    sqlite3_exec(db_, create_commands_table, nullptr, nullptr, &err_msg);
    if (err_msg) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }

    sqlite3_exec(db_, create_history_table, nullptr, nullptr, &err_msg);
    if (err_msg) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }

    // Insert default allowed commands if table is empty
    sqlite3_stmt* stmt;
    const char* check_sql = "SELECT COUNT(*) FROM allowed_commands";
    int count = 0;

    if (sqlite3_prepare_v2(db_, check_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (count == 0) {
        for (const auto& cmd : allowed_commands_) {
            const char* insert_sql = "INSERT OR IGNORE INTO allowed_commands (command) VALUES (?)";
            if (sqlite3_prepare_v2(db_, insert_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, cmd.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
    }
}

bool Config::load() {
    if (!db_) return false;

    // Load configuration values
    sqlite3_stmt* stmt;
    const char* sql = "SELECT key, value FROM config";

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (key == "model") model_ = value;
        else if (key == "ollama_host") ollama_host_ = value;
        else if (key == "temperature") temperature_ = std::stod(value);
        else if (key == "max_tokens") max_tokens_ = std::stoi(value);
        else if (key == "safe_mode") safe_mode_ = (value == "true" || value == "1");
        else if (key == "auto_approve") auto_approve_ = (value == "true" || value == "1");
        else if (key == "mcp_enabled") mcp_enabled_ = (value == "true" || value == "1");
    }

    sqlite3_finalize(stmt);

    // Load allowed commands
    loadAllowedCommands();

    // Load MCP servers from JSON config
    loadMCPServers();

    return true;
}

bool Config::save() {
    if (!db_) return false;

    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO config (key, value) VALUES (?, ?)";

    auto saveValue = [&](const std::string& key, const std::string& value) {
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    };

    saveValue("model", model_);
    saveValue("ollama_host", ollama_host_);
    saveValue("temperature", std::to_string(temperature_));
    saveValue("max_tokens", std::to_string(max_tokens_));
    saveValue("safe_mode", safe_mode_ ? "true" : "false");
    saveValue("auto_approve", auto_approve_ ? "true" : "false");
    saveValue("mcp_enabled", mcp_enabled_ ? "true" : "false");

    return true;
}

void Config::loadAllowedCommands() {
    allowed_commands_.clear();

    sqlite3_stmt* stmt;
    const char* sql = "SELECT command FROM allowed_commands";

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string cmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            allowed_commands_.push_back(cmd);
        }
        sqlite3_finalize(stmt);
    }
}

bool Config::isCommandAllowed(const std::string& command) const {
    if (!safe_mode_) return true;

    // Extract command name (first word)
    std::string cmd_name = command.substr(0, command.find(' '));

    for (const auto& allowed : allowed_commands_) {
        if (cmd_name == allowed || cmd_name.find(allowed) != std::string::npos) {
            return true;
        }
    }

    return false;
}

void Config::addAllowedCommand(const std::string& command) {
    if (!db_) return;

    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR IGNORE INTO allowed_commands (command) VALUES (?)";

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, command.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    allowed_commands_.push_back(command);
}

// Setters
void Config::setModel(const std::string& model) {
    model_ = model;
    save();
}

void Config::setOllamaHost(const std::string& host) {
    ollama_host_ = host;
    save();
}

void Config::setTemperature(double temp) {
    temperature_ = temp;
    save();
}

void Config::setMaxTokens(int tokens) {
    max_tokens_ = tokens;
    save();
}

void Config::setSafeMode(bool enabled) {
    safe_mode_ = enabled;
    save();
}

void Config::setAutoApprove(bool enabled) {
    auto_approve_ = enabled;
    save();
}

void Config::setMCPEnabled(bool enabled) {
    mcp_enabled_ = enabled;
    save();
}

// MCP Server management
std::vector<MCPServerConfig> Config::getMCPServers() const {
    return mcp_servers_;
}

bool Config::addMCPServer(const MCPServerConfig& server) {
    // Check if server already exists
    for (const auto& s : mcp_servers_) {
        if (s.name == server.name) {
            return false;
        }
    }
    mcp_servers_.push_back(server);
    saveMCPServers();
    return true;
}

bool Config::removeMCPServer(const std::string& name) {
    auto it = std::find_if(mcp_servers_.begin(), mcp_servers_.end(),
        [&name](const MCPServerConfig& s) { return s.name == name; });

    if (it != mcp_servers_.end()) {
        mcp_servers_.erase(it);
        saveMCPServers();
        return true;
    }
    return false;
}

bool Config::enableMCPServer(const std::string& name, bool enabled) {
    for (auto& server : mcp_servers_) {
        if (server.name == name) {
            server.enabled = enabled;
            saveMCPServers();
            return true;
        }
    }
    return false;
}

MCPServerConfig* Config::getMCPServer(const std::string& name) {
    for (auto& server : mcp_servers_) {
        if (server.name == name) {
            return &server;
        }
    }
    return nullptr;
}

void Config::loadMCPServers() {
    mcp_servers_.clear();

    std::string mcp_config_path = getMCPConfigPath();
    std::ifstream file(mcp_config_path);
    if (!file.is_open()) {
        return;  // No MCP config file yet
    }

    try {
        json config = json::parse(file);

        if (config.contains("mcpServers") && config["mcpServers"].is_object()) {
            for (const auto& [name, server] : config["mcpServers"].items()) {
                MCPServerConfig sc;
                sc.name = name;
                sc.command = server.value("command", "");
                sc.enabled = server.value("enabled", true);
                sc.transport = server.value("transport", "stdio");
                sc.url = server.value("url", "");

                if (server.contains("args") && server["args"].is_array()) {
                    sc.args = server["args"].get<std::vector<std::string>>();
                }

                if (server.contains("env") && server["env"].is_object()) {
                    sc.env = server["env"].get<std::map<std::string, std::string>>();
                }

                mcp_servers_.push_back(sc);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading MCP config: " << e.what() << std::endl;
    }
}

void Config::saveMCPServers() {
    json config;
    config["mcpServers"] = json::object();

    for (const auto& sc : mcp_servers_) {
        json server;
        server["command"] = sc.command;
        server["args"] = sc.args;
        server["env"] = sc.env;
        server["enabled"] = sc.enabled;
        server["transport"] = sc.transport;
        if (!sc.url.empty()) {
            server["url"] = sc.url;
        }
        config["mcpServers"][sc.name] = server;
    }

    std::string mcp_config_path = getMCPConfigPath();
    std::ofstream file(mcp_config_path);
    if (file.is_open()) {
        file << config.dump(2);
    }
}

// Static methods
std::string Config::getConfigDir() {
    return utils::joinPath(utils::getHomeDir(), ".config/ollamacode");
}

std::string Config::getConfigPath() {
    return utils::joinPath(getConfigDir(), "config.db");
}

std::string Config::getHistoryPath() {
    return utils::joinPath(getConfigDir(), "history.txt");
}

std::string Config::getMCPConfigPath() {
    return utils::joinPath(getConfigDir(), "mcp_servers.json");
}

} // namespace ollamacode
