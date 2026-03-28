#include "../src/config.h"
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <filesystem>

const std::string RESET = "\033[0m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string CYAN = "\033[36m";
const std::string BOLD = "\033[1m";

int testsPassed = 0;
int testsFailed = 0;

void assertTest(const std::string& name, bool condition, const std::string& details = "") {
    if (condition) {
        std::cout << GREEN << "[PASS] " << name << RESET << std::endl;
        testsPassed++;
    } else {
        std::cout << RED << "[FAIL] " << name << RESET;
        if (!details.empty()) std::cout << " - " << details;
        std::cout << std::endl;
        testsFailed++;
    }
}

static bool floatEq(float a, float b) {
    return std::fabs(a - b) < 0.0001f;
}

void testDefaultConfig() {
    std::cout << CYAN << BOLD << "\n=== Test 1: Default Config Values ===" << RESET << std::endl;

    CubeConfig config;
    assertTest("Default config uses defaults", config.isUsingDefaults());
    assertTest("Default front is green",
               floatEq(config.front().r, DefaultColorRGB::GREEN.r) &&
               floatEq(config.front().g, DefaultColorRGB::GREEN.g));
    assertTest("Default back is blue",
               floatEq(config.back().r, DefaultColorRGB::BLUE.r));
    assertTest("Default animation enabled", config.getEnableAnimation());
    assertTest("Default animation speed is 1.0", floatEq(config.getAnimationSpeed(), 1.0f));
    assertTest("Default easing type is 0", config.getEasingType() == 0);
    assertTest("Default renderer type is OpenGL", config.getRendererType() == RendererType::OpenGL);
}

void testSaveLoadRoundTrip() {
    std::cout << CYAN << BOLD << "\n=== Test 2: Save/Load Round Trip ===" << RESET << std::endl;

    std::string configPath = getConfigFilePath();
    std::string backupPath = configPath + ".test_backup";

    if (std::filesystem::exists(configPath)) {
        std::filesystem::copy(configPath, backupPath, std::filesystem::copy_options::overwrite_existing);
    }

    CubeConfig original;
    original.setUsingDefaults(false);
    original.setFront({0.1f, 0.2f, 0.3f});
    original.setBack({0.4f, 0.5f, 0.6f});
    original.setEnableAnimation(false);
    original.setAnimationSpeed(2.5f);
    original.setEasingType(1);
    original.setRendererType(RendererType::Shader);

    bool saved = saveCubeConfig(original);
    assertTest("Save config returns true", saved);

    CubeConfig loaded = loadCubeConfig();
    assertTest("Loaded config not using defaults", !loaded.isUsingDefaults());
    assertTest("Loaded front matches",
               floatEq(loaded.front().r, 0.1f) &&
               floatEq(loaded.front().g, 0.2f) &&
               floatEq(loaded.front().b, 0.3f));
    assertTest("Loaded back matches",
               floatEq(loaded.back().r, 0.4f) &&
               floatEq(loaded.back().g, 0.5f) &&
               floatEq(loaded.back().b, 0.6f));
    assertTest("Loaded animation disabled", !loaded.getEnableAnimation());
    assertTest("Loaded animation speed is 2.5", floatEq(loaded.getAnimationSpeed(), 2.5f));
    assertTest("Loaded easing type is 1", loaded.getEasingType() == 1);
    assertTest("Loaded renderer type is Shader", loaded.getRendererType() == RendererType::Shader);

    if (std::filesystem::exists(backupPath)) {
        std::filesystem::rename(backupPath, configPath);
    } else {
        std::filesystem::remove(configPath);
    }
}

void testConfigFromMalformedFile() {
    std::cout << CYAN << BOLD << "\n=== Test 3: Malformed Config ===" << RESET << std::endl;

    std::string configPath = getConfigFilePath();
    std::string backupPath = configPath + ".test_backup";

    if (std::filesystem::exists(configPath)) {
        std::filesystem::copy(configPath, backupPath, std::filesystem::copy_options::overwrite_existing);
    }

    {
        std::ofstream f(configPath);
        f << "front = 0.5, 0.5\n";
        f << "back = invalid_color\n";
        f << "animationSpeed = not_a_number\n";
        f << "easingType = 999\n";
        f << "rendererType = -1\n";
        f << "enableAnimation = true\n";
    }

    CubeConfig loaded = loadCubeConfig();
    assertTest("Malformed file: front falls back to default (partial parse fails)",
               floatEq(loaded.front().r, DefaultColorRGB::GREEN.r));
    assertTest("Malformed file: back falls back to default",
               floatEq(loaded.back().r, DefaultColorRGB::BLUE.r));
    assertTest("Malformed file: animationSpeed uses default",
               floatEq(loaded.getAnimationSpeed(), 1.0f));
    assertTest("Malformed file: easingType out of range uses default",
               loaded.getEasingType() == 0);
    assertTest("Malformed file: rendererType out of range uses default",
               loaded.getRendererType() == RendererType::OpenGL);
    assertTest("Malformed file: enableAnimation loads correctly",
               loaded.getEnableAnimation());

    if (std::filesystem::exists(backupPath)) {
        std::filesystem::rename(backupPath, configPath);
    } else {
        std::filesystem::remove(configPath);
    }
}

void testMissingConfigFile() {
    std::cout << CYAN << BOLD << "\n=== Test 4: Missing Config File ===" << RESET << std::endl;

    std::string configPath = getConfigFilePath();
    std::string backupPath = configPath + ".test_backup";

    if (std::filesystem::exists(configPath)) {
        std::filesystem::copy(configPath, backupPath, std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove(configPath);
    }

    CubeConfig loaded = loadCubeConfig();
    assertTest("Missing config: uses defaults", loaded.isUsingDefaults());
    assertTest("Missing config: default animation enabled", loaded.getEnableAnimation());

    if (std::filesystem::exists(backupPath)) {
        std::filesystem::rename(backupPath, configPath);
    }
}

void testRgbClamping() {
    std::cout << CYAN << BOLD << "\n=== Test 5: RGB Clamping ===" << RESET << std::endl;

    std::string configPath = getConfigFilePath();
    std::string backupPath = configPath + ".test_backup";

    if (std::filesystem::exists(configPath)) {
        std::filesystem::copy(configPath, backupPath, std::filesystem::copy_options::overwrite_existing);
    }

    {
        std::ofstream f(configPath);
        f << "front = -0.5, 1.5, 0.5\n";
        f << "back = 2.0, -1.0, 0.0\n";
    }

    CubeConfig loaded = loadCubeConfig();
    assertTest("RGB clamped to [0,1]: front.r",
               floatEq(loaded.front().r, 0.0f));
    assertTest("RGB clamped to [0,1]: front.g",
               floatEq(loaded.front().g, 1.0f));
    assertTest("RGB clamped to [0,1]: back.r",
               floatEq(loaded.back().r, 1.0f));
    assertTest("RGB clamped to [0,1]: back.g",
               floatEq(loaded.back().g, 0.0f));

    if (std::filesystem::exists(backupPath)) {
        std::filesystem::rename(backupPath, configPath);
    } else {
        std::filesystem::remove(configPath);
    }
}

int main() {
    std::cout << CYAN << BOLD << "Config Tests" << RESET << std::endl;
    std::cout << "=============" << std::endl;

    testDefaultConfig();
    testSaveLoadRoundTrip();
    testConfigFromMalformedFile();
    testMissingConfigFile();
    testRgbClamping();

    std::cout << "\n" << CYAN << BOLD << "=============" << RESET << std::endl;
    std::cout << GREEN << "  Passed: " << testsPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << testsFailed << RESET << std::endl;

    if (testsFailed == 0) {
        std::cout << GREEN << BOLD << "ALL TESTS PASSED!" << RESET << std::endl;
        return 0;
    }
    return 1;
}
