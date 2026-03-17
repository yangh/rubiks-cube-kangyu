#ifndef COLOR_H
#define COLOR_H

#include <array>
#include <cstdint>
#include <string>
#include "config.h"

// Color enum
enum class Color {
    WHITE,
    YELLOW,
    RED,
    ORANGE,
    GREEN,
    BLUE
};

// Color utility functions
std::array<Color, 9> fillFaceColor(Color color);
std::array<float, 3> colorToRgb(Color color);
std::string colorToString(Color color);
bool isOppositeColor(Color a, Color b);

// DefaultColorRGB namespace (from color_provider.h, WITHOUT forFace to avoid circular dependency)
namespace DefaultColorRGB {
    constexpr std::array<float, 3> WHITE  = {1.0f, 1.0f, 1.0f};
    constexpr std::array<float, 3> YELLOW = {1.0f, 1.0f, 0.0f};
    constexpr std::array<float, 3> RED    = {1.0f, 0.0f, 0.0f};
    constexpr std::array<float, 3> ORANGE = {1.0f, 0.55f, 0.0f};
    constexpr std::array<float, 3> GREEN  = {0.0f, 1.0f, 0.0f};
    constexpr std::array<float, 3> BLUE   = {0.0f, 0.4f, 1.0f};
    constexpr std::array<float, 3> BLACK  = {0.0f, 0.0f, 0.0f};
}

// ColorProvider class (from color_provider.h, unchanged)
class ColorProvider {
public:
    ColorProvider();
    
    void setCustomColors(const ColorConfig& config);
    void resetToDefaults();
    
    uint32_t getFaceColor(Color color) const;
    std::array<float, 3> getFaceColorRgb(Color color) const;
    
    std::array<float, 3> customFront = DefaultColorRGB::GREEN;
    std::array<float, 3> customBack  = DefaultColorRGB::BLUE;
    std::array<float, 3> customLeft  = DefaultColorRGB::ORANGE;
    std::array<float, 3> customRight = DefaultColorRGB::RED;
    std::array<float, 3> customUp    = DefaultColorRGB::WHITE;
    std::array<float, 3> customDown  = DefaultColorRGB::YELLOW;
    bool useCustomColors = false;

private:
    std::array<float, 3> getRgbForColor(Color color) const;
};

#endif // COLOR_H
