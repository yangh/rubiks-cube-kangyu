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
bool isOppositeColor(Color color1, Color color2);

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

    RgbColor& front() { return customFront_; }
    RgbColor& back()  { return customBack_; }
    RgbColor& left()  { return customLeft_; }
    RgbColor& right() { return customRight_; }
    RgbColor& up()    { return customUp_; }
    RgbColor& down()  { return customDown_; }
    const RgbColor& front() const { return customFront_; }
    const RgbColor& back()  const { return customBack_; }
    const RgbColor& left()  const { return customLeft_; }
    const RgbColor& right() const { return customRight_; }
    const RgbColor& up()    const { return customUp_; }
    const RgbColor& down()  const { return customDown_; }

    bool useCustomColors() const { return useCustomColors_; }
    void setUseCustomColors(bool v) { useCustomColors_ = v; }

private:
    RgbColor getRgbForColor(Color color) const;

    RgbColor customFront_ = DefaultColorRGB::GREEN;
    RgbColor customBack_  = DefaultColorRGB::BLUE;
    RgbColor customLeft_  = DefaultColorRGB::ORANGE;
    RgbColor customRight_ = DefaultColorRGB::RED;
    RgbColor customUp_    = DefaultColorRGB::WHITE;
    RgbColor customDown_  = DefaultColorRGB::YELLOW;
    bool useCustomColors_ = false;
};

#endif // COLOR_H
