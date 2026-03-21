#ifndef COLOR_H
#define COLOR_H

#include <array>
#include <cstdint>
#include <string>

// Color enum
enum class Color {
    WHITE,
    YELLOW,
    RED,
    ORANGE,
    GREEN,
    BLUE
};

struct RgbColor {
    float r;
    float g;
    float b;
};

// Color utility functions
RgbColor colorToRgb(Color color);
std::string colorToString(Color color);
bool isOppositeColor(Color a, Color b);

// DefaultColorRGB namespace
namespace DefaultColorRGB {
    constexpr RgbColor WHITE  = {1.0f, 1.0f, 1.0f};
    constexpr RgbColor YELLOW = {1.0f, 1.0f, 0.0f};
    constexpr RgbColor RED    = {1.0f, 0.0f, 0.0f};
    constexpr RgbColor ORANGE = {1.0f, 0.55f, 0.0f};
    constexpr RgbColor GREEN  = {0.0f, 1.0f, 0.0f};
    constexpr RgbColor BLUE   = {0.0f, 0.4f, 1.0f};
    constexpr RgbColor BLACK  = {0.0f, 0.0f, 0.0f};
}

class CubeConfig;

class ColorProvider {
public:
    ColorProvider();
    
    void setCustomColors(const CubeConfig& config);
    void resetToDefaults();
    
    uint32_t getFaceColor(Color color) const;
    RgbColor getFaceColorRgb(Color color) const;
    
    RgbColor customFront = DefaultColorRGB::GREEN;
    RgbColor customBack  = DefaultColorRGB::BLUE;
    RgbColor customLeft  = DefaultColorRGB::ORANGE;
    RgbColor customRight = DefaultColorRGB::RED;
    RgbColor customUp    = DefaultColorRGB::WHITE;
    RgbColor customDown  = DefaultColorRGB::YELLOW;
    bool useCustomColors = false;

private:
    RgbColor getRgbForColor(Color color) const;
};

#endif // COLOR_H
