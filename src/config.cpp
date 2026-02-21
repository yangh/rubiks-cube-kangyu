#include "config.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

// Get default color configuration
ColorConfig getDefaultColorConfig() {
    ColorConfig config;
    return config;  // Constructor already sets default colors
}

// Get config directory path
static std::string getConfigDirPath() {
    const char* homeDir = getenv("HOME");
#ifdef _WIN32
    if (!homeDir) {
        homeDir = getenv("USERPROFILE");
    }
#endif

    if (!homeDir) {
        std::cerr << "Warning: Could not determine home directory" << std::endl;
        return "";
    }

    return std::string(homeDir) + "/.rubiks-cube";
}

// Get config file path (for informational purposes)
std::string getConfigFilePath() {
    std::string configDir = getConfigDirPath();
    if (configDir.empty()) {
        return "";
    }
    return configDir + "/config.json";
}

// Create directory if it doesn't exist
static bool ensureDirectoryExists(const std::string& path) {
    struct stat info;

    if (stat(path.c_str(), &info) == 0) {
        // Directory exists
        if ((info.st_mode & S_IFDIR) != 0) {
            return true;
        } else {
            std::cerr << "Error: " << path << " exists but is not a directory" << std::endl;
            return false;
        }
    }

    // Try to create directory
#ifdef _WIN32
    int result = mkdir(path.c_str());
#else
    int result = mkdir(path.c_str(), 0755);
#endif

    if (result != 0) {
        std::cerr << "Error: Failed to create directory " << path << std::endl;
        return false;
    }

    return true;
}

// Minimal JSON parsing utilities
namespace {
    // Skip whitespace
    static void skipWhitespace(const std::string& json, size_t& pos) {
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' ||
                                       json[pos] == '\n' || json[pos] == '\r')) {
            pos++;
        }
    }

    // Parse a JSON string (unquoted)
    static bool parseStringValue(const std::string& json, size_t& pos, std::string& value) {
        skipWhitespace(json, pos);
        if (pos >= json.length() || json[pos] != '"') {
            return false;
        }
        pos++; // Skip opening quote

        value.clear();
        while (pos < json.length() && json[pos] != '"') {
            if (json[pos] == '\\' && pos + 1 < json.length()) {
                pos++; // Skip escape character
                value += json[pos];
            } else {
                value += json[pos];
            }
            pos++;
        }

        if (pos >= json.length()) {
            return false;
        }
        pos++; // Skip closing quote
        return true;
    }

    // Parse a JSON number
    static bool parseNumberValue(const std::string& json, size_t& pos, float& value) {
        skipWhitespace(json, pos);
        if (pos >= json.length()) {
            return false;
        }

        size_t start = pos;
        bool hasDecimal = false;

        if (json[pos] == '-') {
            pos++;
        }

        while (pos < json.length() && (isdigit(json[pos]) || json[pos] == '.')) {
            if (json[pos] == '.') {
                if (hasDecimal) {
                    return false; // Multiple decimal points
                }
                hasDecimal = true;
            }
            pos++;
        }

        if (pos == start) {
            return false;
        }

        std::string numStr = json.substr(start, pos - start);
        value = std::stof(numStr);
        return true;
    }

    // Parse a JSON boolean
    static bool parseBoolValue(const std::string& json, size_t& pos, bool& value) {
        skipWhitespace(json, pos);
        if (pos >= json.length()) {
            return false;
        }

        if (json.substr(pos, 4) == "true") {
            value = true;
            pos += 4;
            return true;
        } else if (json.substr(pos, 5) == "false") {
            value = false;
            pos += 5;
            return true;
        }

        return false;
    }

    // Parse a JSON object and extract RGB values for a color
    static bool parseColorObject(const std::string& json, size_t& pos, RgbColor& color) {
        skipWhitespace(json, pos);
        if (pos >= json.length() || json[pos] != '{') {
            return false;
        }
        pos++; // Skip opening brace

        bool hasR = false, hasG = false, hasB = false;

        while (pos < json.length() && json[pos] != '}') {
            skipWhitespace(json, pos);

            std::string key;
            if (!parseStringValue(json, pos, key)) {
                return false;
            }

            skipWhitespace(json, pos);
            if (pos >= json.length() || json[pos] != ':') {
                return false;
            }
            pos++; // Skip colon

            float value;
            if (!parseNumberValue(json, pos, value)) {
                return false;
            }

            // Assign to appropriate color component
            if (key == "r") {
                color.r = value;
                hasR = true;
            } else if (key == "g") {
                color.g = value;
                hasG = true;
            } else if (key == "b") {
                color.b = value;
                hasB = true;
            }

            skipWhitespace(json, pos);
            if (pos < json.length() && json[pos] == ',') {
                pos++; // Skip comma
            }
        }

        if (pos >= json.length()) {
            return false;
        }
        pos++; // Skip closing brace

        return hasR && hasG && hasB;
    }

    // Find and extract a value from a JSON object by key
    static bool extractObjectByKey(const std::string& json, size_t& pos,
                                   const std::string& key, std::string& valueStr) {
        skipWhitespace(json, pos);
        if (pos >= json.length() || json[pos] != '{') {
            return false;
        }
        pos++; // Skip opening brace

        size_t startPos = pos;

        // Find the matching closing brace
        int braceCount = 1;
        while (pos < json.length() && braceCount > 0) {
            if (json[pos] == '{') {
                braceCount++;
            } else if (json[pos] == '}') {
                braceCount--;
            } else if (json[pos] == '"' && braceCount == 1) {
                // Check for key
                pos++;
                std::string currentKey;
                while (pos < json.length() && json[pos] != '"') {
                    if (json[pos] == '\\' && pos + 1 < json.length()) {
                        pos++;
                    }
                    currentKey += json[pos];
                    pos++;
                }

                if (pos >= json.length() || json[pos] != '"') {
                    return false;
                }
                pos++; // Skip closing quote

                skipWhitespace(json, pos);
                if (pos >= json.length() || json[pos] != ':') {
                    return false;
                }
                pos++; // Skip colon

                // If this is our key, find the value object
                if (currentKey == key) {
                    skipWhitespace(json, pos);
                    size_t valueStart = pos;

                    // Find the matching brace for the value object
                    braceCount = 1;
                    if (json[pos] != '{') {
                        // Not an object, just extract until comma or closing brace
                        while (pos < json.length() && json[pos] != ',' && json[pos] != '}') {
                            pos++;
                        }
                        valueStr = json.substr(valueStart, pos - valueStart);
                        return true;
                    }
                    pos++; // Skip opening brace of value object

                    while (pos < json.length() && braceCount > 0) {
                        if (json[pos] == '{') {
                            braceCount++;
                        } else if (json[pos] == '}') {
                            braceCount--;
                        }
                        pos++;
                    }
                    valueStr = json.substr(valueStart, pos - valueStart);
                    return true;
                }
            }
            pos++;
        }

        pos = startPos;
        return false;
    }

    // Extract a boolean value from JSON by key
    static bool extractBoolByKey(const std::string& json, size_t& pos,
                               const std::string& key, bool& value) {
        skipWhitespace(json, pos);
        if (pos >= json.length() || json[pos] != '{') {
            return false;
        }
        pos++; // Skip opening brace

        size_t startPos = pos;
        int braceCount = 1;
        bool skippingNestedObject = false;

        std::cout << "  extractBoolByKey: searching for '" << key << "' from pos " << startPos << std::endl;

        while (pos < json.length() && braceCount > 0) {
            if (json[pos] == '{') {
                braceCount++;
                std::cout << "  extractBoolByKey: found '{' at pos " << pos << ", braceCount=" << braceCount << std::endl;
            } else if (json[pos] == '}') {
                braceCount--;
                std::cout << "  extractBoolByKey: found '}' at pos " << pos << ", braceCount=" << braceCount << std::endl;
            } else if (json[pos] == '"' && braceCount == 1) {
                // Check for key at current level
                pos++;
                std::string currentKey;
                while (pos < json.length() && json[pos] != '"') {
                    if (json[pos] == '\\' && pos + 1 < json.length()) {
                        pos++;
                    }
                    currentKey += json[pos];
                    pos++;
                }

                if (pos >= json.length() || json[pos] != '"') {
                    std::cout << "  extractBoolByKey: ERROR: no closing quote at pos " << pos << std::endl;
                    return false;
                }
                pos++; // Skip closing quote

                skipWhitespace(json, pos);
                if (pos >= json.length() || json[pos] != ':') {
                    std::cout << "  extractBoolByKey: ERROR: no colon at pos " << pos << std::endl;
                    return false;
                }
                pos++; // Skip colon

                std::cout << "  extractBoolByKey: found key='" << currentKey << "' at pos " << pos << std::endl;

                // Check if this is a nested object we should skip
                skipWhitespace(json, pos);
                if (pos < json.length() && json[pos] == '{') {
                    // This is a nested object (like "colors"), skip its entire value
                    std::cout << "  extractBoolByKey: skipping nested object at pos " << pos << std::endl;
                    skippingNestedObject = true;
                    int nestedBraceCount = 1;
                    pos++; // Skip opening brace of nested object
                    while (pos < json.length() && nestedBraceCount > 0) {
                        if (json[pos] == '{') {
                            nestedBraceCount++;
                        } else if (json[pos] == '}') {
                            nestedBraceCount--;
                        }
                        pos++;
                    }
                    skippingNestedObject = false;
                    std::cout << "  extractBoolByKey: done skipping nested object, now at pos " << pos << std::endl;
                    continue;
                }

                // If this is our key, parse the boolean value
                if (currentKey == key) {
                    std::cout << "  extractBoolByKey: MATCH! parsing at pos " << pos << std::endl;
                    return parseBoolValue(json, pos, value);
                }
            }
            pos++;
        }

        std::cout << "  extractBoolByKey: NOT FOUND, returning false" << std::endl;
        pos = startPos;
        return false;
    }

    // Extract a number value from JSON by key
    static bool extractFloatByKey(const std::string& json, size_t& pos,
                               const std::string& key, float& value) {
        skipWhitespace(json, pos);
        if (pos >= json.length() || json[pos] != '{') {
            return false;
        }
        pos++; // Skip opening brace

        size_t startPos = pos;
        int braceCount = 1;
        bool skippingNestedObject = false;

        while (pos < json.length() && braceCount > 0) {
            if (json[pos] == '{') {
                braceCount++;
            } else if (json[pos] == '}') {
                braceCount--;
            } else if (json[pos] == '"' && braceCount == 1) {
                // Check for key at current level
                pos++;
                std::string currentKey;
                while (pos < json.length() && json[pos] != '"') {
                    if (json[pos] == '\\' && pos + 1 < json.length()) {
                        pos++;
                    }
                    currentKey += json[pos];
                    pos++;
                }

                if (pos >= json.length() || json[pos] != '"') {
                    return false;
                }
                pos++; // Skip closing quote

                skipWhitespace(json, pos);
                if (pos >= json.length() || json[pos] != ':') {
                    return false;
                }
                pos++; // Skip colon

                // Check if this is a nested object we should skip
                skipWhitespace(json, pos);
                if (pos < json.length() && json[pos] == '{') {
                    // This is a nested object (like "colors"), skip its entire value
                    skippingNestedObject = true;
                    int nestedBraceCount = 1;
                    pos++; // Skip opening brace of nested object
                    while (pos < json.length() && nestedBraceCount > 0) {
                        if (json[pos] == '{') {
                            nestedBraceCount++;
                        } else if (json[pos] == '}') {
                            nestedBraceCount--;
                        }
                        pos++;
                    }
                    skippingNestedObject = false;
                    continue;
                }

                // If this is our key, parse the number value
                if (currentKey == key) {
                    return parseNumberValue(json, pos, value);
                }
            }
            pos++;
        }

        pos = startPos;
        return false;
    }
}

// Load color configuration from JSON file
ColorConfig loadColorConfig() {
    ColorConfig config = getDefaultColorConfig();
    std::string configPath = getConfigFilePath();

    if (configPath.empty()) {
        std::cerr << "Warning: Could not determine config file path, using defaults" << std::endl;
        return config;
    }

    std::ifstream file(configPath);
    if (!file.is_open()) {
        // File doesn't exist, return default config silently
        return config;
    }

    // Read entire file
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    file.close();

    if (json.empty()) {
        std::cerr << "Warning: Config file is empty, using defaults" << std::endl;
        return config;
    }

    size_t pos = 0;

    // Parse JSON structure
    skipWhitespace(json, pos);
    if (pos >= json.length() || json[pos] != '{') {
        std::cerr << "Warning: Invalid JSON format, using defaults" << std::endl;
        return config;
    }
    pos++; // Skip opening brace

    // Look for "colors" object - reset pos to 0 and search from beginning
    // extractObjectByKey expects to find the opening brace, so we need to start from 0
    pos = 0;
    std::string colorsObj;
    if (!extractObjectByKey(json, pos, "colors", colorsObj)) {
        std::cerr << "Warning: Could not find 'colors' object, using defaults" << std::endl;
        return config;
    }

    // Parse colors object
    pos = 0;
    skipWhitespace(colorsObj, pos);
    if (pos >= colorsObj.length() || colorsObj[pos] != '{') {
        return config;
    }
    pos++; // Skip opening brace

    bool success = true;
    while (pos < colorsObj.length() && colorsObj[pos] != '}' && success) {
        std::string key;
        if (!parseStringValue(colorsObj, pos, key)) {
            success = false;
            break;
        }

        skipWhitespace(colorsObj, pos);
        if (pos >= colorsObj.length() || colorsObj[pos] != ':') {
            success = false;
            break;
        }
        pos++; // Skip colon

        RgbColor color;
        if (parseColorObject(colorsObj, pos, color)) {
            if (key == "front") {
                config.setFront(color);
            } else if (key == "back") {
                config.setBack(color);
            } else if (key == "left") {
                config.setLeft(color);
            } else if (key == "right") {
                config.setRight(color);
            } else if (key == "up") {
                config.setUp(color);
            } else if (key == "down") {
                config.setDown(color);
            }
        } else {
            success = false;
            break;
        }

        skipWhitespace(colorsObj, pos);
        if (pos < colorsObj.length() && colorsObj[pos] == ',') {
            pos++; // Skip comma
        }
    }

    if (!success) {
        std::cerr << "Warning: Error parsing config file, using defaults" << std::endl;
        return getDefaultColorConfig();
    }

    // Mark that we're using custom colors
    config.setUsingDefaults(false);

    // Load animation settings using direct string search (simpler approach)
    // Search for "enableAnimation": in JSON
    std::string enableAnimKey = "\"enableAnimation\":";
    size_t enableAnimPos = json.find(enableAnimKey);
    if (enableAnimPos != std::string::npos) {
        size_t valStart = enableAnimPos + enableAnimKey.length();
        skipWhitespace(json, valStart);
        bool enableAnim;
        if (parseBoolValue(json, valStart, enableAnim)) {
            config.setEnableAnimation(enableAnim);
        }
    }

    // Search for "animationSpeed": in JSON
    std::string animSpeedKey = "\"animationSpeed\":";
    size_t animSpeedPos = json.find(animSpeedKey);
    if (animSpeedPos != std::string::npos) {
        size_t valStart = animSpeedPos + animSpeedKey.length();
        skipWhitespace(json, valStart);
        float animSpeed;
        if (parseNumberValue(json, valStart, animSpeed)) {
            config.setAnimationSpeed(animSpeed);
        }
    }

    return config;
}

// Write RGB color as JSON object
static std::string writeRgbColor(const RgbColor& color) {
    std::ostringstream oss;
    oss << "{\"r\":" << color.r << ",\"g\":" << color.g << ",\"b\":" << color.b << "}";
    return oss.str();
}

// Save color configuration to JSON file
bool saveColorConfig(const ColorConfig& config) {
    std::string configDir = getConfigDirPath();
    if (configDir.empty()) {
        std::cerr << "Error: Could not determine config directory" << std::endl;
        return false;
    }

    // Create directory if it doesn't exist
    if (!ensureDirectoryExists(configDir)) {
        return false;
    }

    std::string configPath = configDir + "/config.json";

    std::ofstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open config file for writing: " << configPath << std::endl;
        return false;
    }

    // Write JSON
    file << "{\n";
    file << "  \"colors\": {\n";
    file << "    \"front\": " << writeRgbColor(config.front()) << ",\n";
    file << "    \"back\": " << writeRgbColor(config.back()) << ",\n";
    file << "    \"left\": " << writeRgbColor(config.left()) << ",\n";
    file << "    \"right\": " << writeRgbColor(config.right()) << ",\n";
    file << "    \"up\": " << writeRgbColor(config.up()) << ",\n";
    file << "    \"down\": " << writeRgbColor(config.down()) << "\n";
    file << "  },\n";
    file << "  \"enableAnimation\": " << (config.getEnableAnimation() ? "true" : "false") << ",\n";
    file << "  \"animationSpeed\": " << config.getAnimationSpeed() << "\n";
    file << "}\n";

    file.close();

    std::cout << "Configuration saved to: " << configPath << std::endl;
    return true;
}
