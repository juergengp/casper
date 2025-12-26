#include "license_client.h"
#include "utils.h"
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/utsname.h>

#ifdef __APPLE__
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef __linux__
#include <fstream>
#endif

// Simple JSON parsing (minimal implementation)
namespace {
    std::string getJsonString(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return "";

        pos += search.length();
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

        if (pos >= json.length()) return "";

        if (json[pos] == '"') {
            pos++;
            size_t end = json.find('"', pos);
            if (end == std::string::npos) return "";
            return json.substr(pos, end - pos);
        } else if (json[pos] == '[') {
            // Return the array as string for now
            size_t end = json.find(']', pos);
            if (end == std::string::npos) return "";
            return json.substr(pos, end - pos + 1);
        }

        // Number or boolean
        size_t end = pos;
        while (end < json.length() && json[end] != ',' && json[end] != '}' && json[end] != ' ') end++;
        return json.substr(pos, end - pos);
    }

    bool getJsonBool(const std::string& json, const std::string& key) {
        std::string val = getJsonString(json, key);
        return val == "true";
    }

    int getJsonInt(const std::string& json, const std::string& key) {
        std::string val = getJsonString(json, key);
        if (val.empty()) return 0;
        return std::stoi(val);
    }

    std::vector<std::string> parseJsonArray(const std::string& arr) {
        std::vector<std::string> result;
        size_t pos = 0;
        while ((pos = arr.find('"', pos)) != std::string::npos) {
            pos++;
            size_t end = arr.find('"', pos);
            if (end == std::string::npos) break;
            result.push_back(arr.substr(pos, end - pos));
            pos = end + 1;
        }
        return result;
    }

    size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
}

namespace casper {

LicenseClient::LicenseClient()
    : server_url_("http://10.19.0.128:5000")
    , cache_loaded_(false) {
    machine_id_ = generateMachineId();
    machine_name_ = getMachineName();
    cached_info_.valid = false;
    cached_info_.last_validated = 0;
    cached_info_.grace_expires = 0;
}

LicenseClient::~LicenseClient() {
    // Check in floating license if active
    if (!session_token_.empty()) {
        checkin();
    }
}

void LicenseClient::setServerUrl(const std::string& url) {
    server_url_ = url;
}

void LicenseClient::setLicenseKey(const std::string& key) {
    license_key_ = key;
}

std::string LicenseClient::getLicenseKey() const {
    return license_key_;
}

std::string LicenseClient::generateMachineId() {
#ifdef __APPLE__
    io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMainPortDefault, "IOService:/");
    CFStringRef uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
    IOObjectRelease(ioRegistryRoot);

    if (uuidCf) {
        char buffer[128];
        CFStringGetCString(uuidCf, buffer, sizeof(buffer), kCFStringEncodingUTF8);
        CFRelease(uuidCf);
        return std::string(buffer);
    }
    return "unknown-mac";
#elif __linux__
    std::ifstream file("/etc/machine-id");
    if (file.is_open()) {
        std::string id;
        std::getline(file, id);
        return id;
    }

    // Fallback to DMI product UUID
    std::ifstream dmi("/sys/class/dmi/id/product_uuid");
    if (dmi.is_open()) {
        std::string id;
        std::getline(dmi, id);
        return id;
    }
    return "unknown-linux";
#else
    return "unknown-platform";
#endif
}

std::string LicenseClient::getMachineId() const {
    return machine_id_;
}

std::string LicenseClient::getMachineName() const {
    struct utsname buf;
    if (uname(&buf) == 0) {
        return std::string(buf.nodename);
    }
    return "unknown";
}

std::string LicenseClient::httpPost(const std::string& endpoint, const std::string& json_body) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    std::string response;
    std::string url = server_url_ + endpoint;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }

    return response;
}

ServerLicenseInfo LicenseClient::validate() {
    ServerLicenseInfo info;
    info.valid = false;
    info.license_key = license_key_;

    if (license_key_.empty()) {
        info.error = "No license key configured";
        return info;
    }

    std::ostringstream json;
    json << "{\"license_key\":\"" << license_key_ << "\","
         << "\"machine_id\":\"" << machine_id_ << "\","
         << "\"machine_name\":\"" << machine_name_ << "\"}";

    std::string response = httpPost("/api/validate", json.str());

    if (response.empty()) {
        info.error = "Cannot connect to license server";
        // Check if we have a valid cache within grace period
        if (cache_loaded_ && cached_info_.valid && cached_info_.grace_expires > time(nullptr)) {
            info = cached_info_;
            info.error = "Using cached license (offline mode)";
            return info;
        }
        return info;
    }

    info.valid = getJsonBool(response, "valid");

    if (!info.valid) {
        info.error = getJsonString(response, "error");
        return info;
    }

    info.license_type = getJsonString(response, "license_type");
    info.owner = getJsonString(response, "owner");
    info.expires_at = getJsonString(response, "expires_at");
    info.grace_period_days = getJsonInt(response, "grace_period_days");

    std::string features_str = getJsonString(response, "features");
    info.features = parseJsonArray(features_str);

    info.last_validated = time(nullptr);
    info.grace_expires = info.last_validated + (info.grace_period_days * 24 * 60 * 60);

    // Update cache
    cached_info_ = info;
    saveCache();

    return info;
}

bool LicenseClient::isValid() {
    // First try to load cache if not loaded
    if (!cache_loaded_) {
        loadCache();
    }

    // Try to validate with server
    ServerLicenseInfo info = validate();

    if (info.valid) {
        cached_info_ = info;
        return true;
    }

    // Check grace period
    if (cached_info_.valid && cached_info_.grace_expires > time(nullptr)) {
        return true;
    }

    return false;
}

ServerLicenseInfo LicenseClient::getLicenseInfo() const {
    return cached_info_;
}

bool LicenseClient::hasFeature(const std::string& feature) const {
    if (!cached_info_.valid) return false;

    for (const auto& f : cached_info_.features) {
        if (f == "all" || f == feature) return true;
    }
    return false;
}

bool LicenseClient::deactivate() {
    if (license_key_.empty()) return false;

    std::ostringstream json;
    json << "{\"license_key\":\"" << license_key_ << "\","
         << "\"machine_id\":\"" << machine_id_ << "\"}";

    std::string response = httpPost("/api/deactivate", json.str());

    if (response.empty()) return false;

    bool success = getJsonBool(response, "success");
    if (success) {
        cached_info_.valid = false;
        saveCache();
    }
    return success;
}

bool LicenseClient::checkout() {
    if (license_key_.empty()) return false;

    std::ostringstream json;
    json << "{\"license_key\":\"" << license_key_ << "\","
         << "\"machine_id\":\"" << machine_id_ << "\"}";

    std::string response = httpPost("/api/checkout", json.str());

    if (response.empty()) return false;

    bool success = getJsonBool(response, "success");
    if (success) {
        session_token_ = getJsonString(response, "session_token");
    }
    return success;
}

bool LicenseClient::checkin() {
    if (session_token_.empty()) return false;

    std::ostringstream json;
    json << "{\"session_token\":\"" << session_token_ << "\"}";

    std::string response = httpPost("/api/checkin", json.str());

    if (!response.empty() && getJsonBool(response, "success")) {
        session_token_.clear();
        return true;
    }
    return false;
}

bool LicenseClient::heartbeat() {
    if (session_token_.empty()) return false;

    std::ostringstream json;
    json << "{\"session_token\":\"" << session_token_ << "\"}";

    std::string response = httpPost("/api/heartbeat", json.str());

    return !response.empty() && getJsonBool(response, "success");
}

std::string LicenseClient::getCachePath() const {
    std::string home = utils::getHomeDir();
    return utils::joinPath(home, ".casper_license_cache");
}

bool LicenseClient::loadCache() {
    std::string path = getCachePath();
    std::ifstream file(path);

    if (!file.is_open()) {
        cache_loaded_ = true;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (key == "license_key") cached_info_.license_key = value;
        else if (key == "license_type") cached_info_.license_type = value;
        else if (key == "owner") cached_info_.owner = value;
        else if (key == "expires_at") cached_info_.expires_at = value;
        else if (key == "grace_period_days") cached_info_.grace_period_days = std::stoi(value);
        else if (key == "last_validated") cached_info_.last_validated = std::stol(value);
        else if (key == "grace_expires") cached_info_.grace_expires = std::stol(value);
        else if (key == "valid") cached_info_.valid = (value == "1");
        else if (key == "features") {
            cached_info_.features.clear();
            std::stringstream ss(value);
            std::string feature;
            while (std::getline(ss, feature, ',')) {
                cached_info_.features.push_back(feature);
            }
        }
    }

    // Also restore license_key_ if it was cached
    if (!cached_info_.license_key.empty() && license_key_.empty()) {
        license_key_ = cached_info_.license_key;
    }

    cache_loaded_ = true;
    return true;
}

bool LicenseClient::saveCache() {
    std::string path = getCachePath();
    std::ofstream file(path);

    if (!file.is_open()) return false;

    file << "license_key=" << cached_info_.license_key << "\n";
    file << "license_type=" << cached_info_.license_type << "\n";
    file << "owner=" << cached_info_.owner << "\n";
    file << "expires_at=" << cached_info_.expires_at << "\n";
    file << "grace_period_days=" << cached_info_.grace_period_days << "\n";
    file << "last_validated=" << cached_info_.last_validated << "\n";
    file << "grace_expires=" << cached_info_.grace_expires << "\n";
    file << "valid=" << (cached_info_.valid ? "1" : "0") << "\n";

    std::string features_str;
    for (size_t i = 0; i < cached_info_.features.size(); i++) {
        if (i > 0) features_str += ",";
        features_str += cached_info_.features[i];
    }
    file << "features=" << features_str << "\n";

    return true;
}

int LicenseClient::getGraceDaysRemaining() const {
    if (!cached_info_.valid || cached_info_.grace_expires == 0) return 0;

    time_t now = time(nullptr);
    if (now >= cached_info_.grace_expires) return 0;

    return (cached_info_.grace_expires - now) / (24 * 60 * 60);
}

bool LicenseClient::isInGracePeriod() const {
    if (!cached_info_.valid) return false;

    time_t now = time(nullptr);
    // In grace period if we haven't validated recently but grace hasn't expired
    return (now - cached_info_.last_validated > 24 * 60 * 60) &&
           (now < cached_info_.grace_expires);
}

} // namespace casper
