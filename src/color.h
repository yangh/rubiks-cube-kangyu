#ifndef COLOR_H
#define COLOR_H

#include <array>
#include <cstdint>
#include <string>

class CubeConfig;

typedef std::array<float, 3> ColorRGB;

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
ColorRGB colorToRgb(Color color);
std::string colorToString(Color color);
bool isOppositeColor(Color a, Color b);

// DefaultColorRGB namespace (from color_provider.h, WITHOUT forFace to avoid circular dependency)
namespace DefaultColorRGB {
    constexpr ColorRGB WHITE  = {1.0f, 1.0f, 1.0f};
    constexpr ColorRGB YELLOW = {1.0f, 1.0f, 0.0f};
    constexpr ColorRGB RED    = {1.0f, 0.0f, 0.0f};
    constexpr ColorRGB ORANGE = {1.0f, 0.55f, 0.0f};
    constexpr ColorRGB GREEN  = {0.0f, 1.0f, 0.0f};
    constexpr ColorRGB BLUE   = {0.0f, 0.4f, 1.0f};
    constexpr ColorRGB BLACK  = {0.0f, 0.0f, 0.0f};
}

// ColorProvider class (from color_provider.h, unchanged)
class ColorProvider {
public:
    ColorProvider();
    
    void setCustomColors(const CubeConfig& config);
    void resetToDefaults();
    
    uint32_t getFaceColor(Color color) const;
    ColorRGB getFaceColorRgb(Color color) const;
    
    ColorRGB customFront = DefaultColorRGB::GREEN;
    ColorRGB customBack  = DefaultColorRGB::BLUE;
    ColorRGB customLeft  = DefaultColorRGB::ORANGE;
    ColorRGB customRight = DefaultColorRGB::RED;
    ColorRGB customUp    = DefaultColorRGB::WHITE;
    ColorRGB customDown  = DefaultColorRGB::YELLOW;
    bool useCustomColors = false;

private:
    ColorRGB getRgbForColor(Color color) const;
};

#endif // COLOR_H
