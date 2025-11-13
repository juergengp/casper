#ifndef OLLAMACODE_CONFIG_H
#define OLLAMACODE_CONFIG_H

#include <string>
#include <vector>
#include <memory>
#include <sqlite3.h>

namespace ollamacode {

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

    // Setters
    void setModel(const std::string& model);
    void setOllamaHost(const std::string& host);
    void setTemperature(double temp);
    void setMaxTokens(int tokens);
    void setSafeMode(bool enabled);
    void setAutoApprove(bool enabled);

    // Persistence
    bool save();
    bool load();

    // Allowed commands for safe mode
    bool isCommandAllowed(const std::string& command) const;
    void addAllowedCommand(const std::string& command);

    // Get config directory path
    static std::string getConfigDir();
    static std::string getConfigPath();
    static std::string getHistoryPath();

private:
    void createDefaultConfig();
    void initializeDatabase();
    void loadAllowedCommands();

    sqlite3* db_;
    std::string config_path_;

    // Configuration values
    std::string model_;
    std::string ollama_host_;
    double temperature_;
    int max_tokens_;
    bool safe_mode_;
    bool auto_approve_;

    // Allowed commands for safe mode
    std::vector<std::string> allowed_commands_;
};

} // namespace ollamacode

#endif // OLLAMACODE_CONFIG_H
