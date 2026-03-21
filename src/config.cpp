#include "config.h"
#include "color.h"
#include "renderer.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

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
    return configDir + "/config.ini";
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

// Trim whitespace from string
static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Parse RGB color from value string (handles "r, g, b" or "r g b")
static bool parseColorValue(const std::string& value, RgbColor& color) {
    std::string v = value;
    // Replace commas with spaces for uniform parsing
    std::replace(v.begin(), v.end(), ',', ' ');
    
    std::istringstream iss(v);
    float r, g, b;
    if (!(iss >> r >> g >> b)) {
        return false;
    }
    color = RgbColor(r, g, b);
    return true;
}

// Load color configuration from INI file
CubeConfig loadCubeConfig() {
    CubeConfig config;
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

    bool foundAnyColor = false;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Find the equals sign
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, eqPos));
        std::string value = trim(line.substr(eqPos + 1));

        // Parse color keys
        if (key == "front" || key == "back" || key == "left" || 
            key == "right" || key == "up" || key == "down") {
            RgbColor color;
            if (parseColorValue(value, color)) {
                foundAnyColor = true;
                if (key == "front") config.setFront(color);
                else if (key == "back") config.setBack(color);
                else if (key == "left") config.setLeft(color);
                else if (key == "right") config.setRight(color);
                else if (key == "up") config.setUp(color);
                else if (key == "down") config.setDown(color);
            }
        }
        // Parse animation settings
        else if (key == "enableAnimation") {
            std::string val = value;
            std::transform(val.begin(), val.end(), val.begin(), ::tolower);
            config.setEnableAnimation(val == "true");
        }
        else if (key == "animationSpeed") {
            try {
                config.setAnimationSpeed(std::stof(value));
            } catch (...) {}
        }
        else if (key == "easingType") {
            try {
                config.setEasingType(std::stoi(value));
            } catch (...) {}
        }
        else if (key == "rendererType") {
            try {
                config.setRendererType(static_cast<RendererType>(std::stoi(value)));
            } catch (...) {}
        }
        // Unknown keys are silently ignored
    }

    file.close();

    if (foundAnyColor) {
        config.setUsingDefaults(false);
    }

    return config;
}

// Save color configuration to INI file
bool saveCubeConfig(const CubeConfig& config) {
    std::string configDir = getConfigDirPath();
    if (configDir.empty()) {
        std::cerr << "Error: Could not determine config directory" << std::endl;
        return false;
    }

    // Create directory if it doesn't exist
    if (!ensureDirectoryExists(configDir)) {
        return false;
    }

    std::string configPath = configDir + "/config.ini";

    std::ofstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open config file for writing: " << configPath << std::endl;
        return false;
    }

    file << std::fixed << std::setprecision(6);

    // Write header comment
    file << "# Rubik's Cube Configuration\n";
    file << "\n";
    file << "# Colors (r, g, b)\n";
    
    // Write colors
    file << "front = " << config.front().r << ", " << config.front().g << ", " << config.front().b << "\n";
    file << "back = " << config.back().r << ", " << config.back().g << ", " << config.back().b << "\n";
    file << "left = " << config.left().r << ", " << config.left().g << ", " << config.left().b << "\n";
    file << "right = " << config.right().r << ", " << config.right().g << ", " << config.right().b << "\n";
    file << "up = " << config.up().r << ", " << config.up().g << ", " << config.up().b << "\n";
    file << "down = " << config.down().r << ", " << config.down().g << ", " << config.down().b << "\n";
    
    // Write animation settings
    file << "\n";
    file << "# Animation\n";
    file << "enableAnimation = " << (config.getEnableAnimation() ? "true" : "false") << "\n";
    file << "animationSpeed = " << config.getAnimationSpeed() << "\n";
    file << "easingType = " << config.getEasingType() << "\n";
    file << "rendererType = " << static_cast<int>(config.getRendererType()) << "\n";

    file.close();

    std::cout << "Configuration saved to: " << configPath << std::endl;
    return true;
}

CubeConfig::CubeConfig() :
    usingDefaults_(true),
    enableAnimation_(true),
    animationSpeed_(1.0f),
    easingType_(0),
    rendererType_(RendererType::OpenGL)
{
        front_ = RgbColor(DefaultColorRGB::GREEN);
        back_  = RgbColor(DefaultColorRGB::BLUE);
        left_  = RgbColor(DefaultColorRGB::ORANGE);
        right_ = RgbColor(DefaultColorRGB::RED);
        up_    = RgbColor(DefaultColorRGB::WHITE);
        down_  = RgbColor(DefaultColorRGB::YELLOW);
}
