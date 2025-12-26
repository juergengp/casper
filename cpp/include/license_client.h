#ifndef CASPER_LICENSE_CLIENT_H
#define CASPER_LICENSE_CLIENT_H

#include <string>
#include <vector>
#include <ctime>

namespace casper {

struct ServerLicenseInfo {
    bool valid;
    std::string license_key;
    std::string license_type;  // perpetual, subscription, floating
    std::string owner;
    std::string expires_at;
    std::vector<std::string> features;
    int grace_period_days;
    std::string error;
    time_t last_validated;
    time_t grace_expires;
};

class LicenseClient {
public:
    LicenseClient();
    ~LicenseClient();

    // Set the license server URL
    void setServerUrl(const std::string& url);

    // Set the license key
    void setLicenseKey(const std::string& key);

    // Get the license key
    std::string getLicenseKey() const;

    // Validate license with server
    ServerLicenseInfo validate();

    // Check if license is valid (uses cache + grace period)
    bool isValid();

    // Get current license info
    ServerLicenseInfo getLicenseInfo() const;

    // Check if a specific feature is enabled
    bool hasFeature(const std::string& feature) const;

    // Get machine ID
    std::string getMachineId() const;

    // Get machine name
    std::string getMachineName() const;

    // Deactivate license on this machine
    bool deactivate();

    // For floating licenses
    bool checkout();
    bool checkin();
    bool heartbeat();

    // Load/save license cache for grace period
    bool loadCache();
    bool saveCache();

    // Get days remaining in grace period
    int getGraceDaysRemaining() const;

    // Check if in grace period mode
    bool isInGracePeriod() const;

private:
    std::string server_url_;
    std::string license_key_;
    std::string machine_id_;
    std::string machine_name_;
    std::string session_token_;  // For floating licenses
    ServerLicenseInfo cached_info_;
    bool cache_loaded_;

    std::string getCachePath() const;
    std::string httpPost(const std::string& endpoint, const std::string& json_body);
    std::string generateMachineId();
};

} // namespace casper

#endif // CASPER_LICENSE_CLIENT_H
