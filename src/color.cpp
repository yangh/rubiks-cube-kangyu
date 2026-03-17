#include "color.h"

std::array<float, 3> colorToRgb(Color color) {
    switch (color) {
        case Color::WHITE:  return DefaultColors::WHITE;
        case Color::YELLOW: return DefaultColors::YELLOW;
        case Color::RED:    return DefaultColors::RED;
        case Color::ORANGE: return DefaultColors::ORANGE;
        case Color::GREEN:  return DefaultColors::GREEN;
        case Color::BLUE:   return DefaultColors::BLUE;
        default:            return DefaultColors::BLACK;
    }
}

std::string colorToString(Color color) {
    static const char* names[] = {"W", "Y", "R", "O", "G", "B"};
    int idx = static_cast<int>(color);
    return (idx >= 0 && idx < 6) ? names[idx] : "?";
}

bool isOppositeColor(Color a, Color b) {
    return (a == Color::WHITE  && b == Color::YELLOW)  ||
           (a == Color::YELLOW && b == Color::WHITE)   ||
           (a == Color::RED    && b == Color::ORANGE)  ||
           (a == Color::ORANGE && b == Color::RED)     ||
           (a == Color::GREEN  && b == Color::BLUE)    ||
           (a == Color::BLUE   && b == Color::GREEN);
}

ColorProvider::ColorProvider() = default;

void ColorProvider::setCustomColors(const ColorConfig& config) {
    customFront = config.getFrontColor();
    customBack  = config.getBackColor();
    customLeft  = config.getLeftColor();
    customRight = config.getRightColor();
    customUp    = config.getUpColor();
    customDown  = config.getDownColor();
    useCustomColors = !config.isUsingDefaults();
}

void ColorProvider::resetToDefaults() {
    customFront = DefaultColors::GREEN;
    customBack  = DefaultColors::BLUE;
    customLeft  = DefaultColors::ORANGE;
    customRight = DefaultColors::RED;
    customUp    = DefaultColors::WHITE;
    customDown  = DefaultColors::YELLOW;
    useCustomColors = false;
}

std::array<float, 3> ColorProvider::getRgbForColor(Color color) const {
    if (!useCustomColors) {
        return colorToRgb(color);
    }
    
    switch (color) {
        case Color::GREEN:  return customFront;
        case Color::BLUE:   return customBack;
        case Color::ORANGE: return customLeft;
        case Color::RED:    return customRight;
        case Color::WHITE:  return customUp;
        case Color::YELLOW: return customDown;
        default:            return colorToRgb(color);
    }
}

uint32_t ColorProvider::getFaceColor(Color color) const {
    std::array<float, 3> rgb = getRgbForColor(color);
    return static_cast<uint32_t>(255) << 24
         | static_cast<uint32_t>(rgb[2] * 255) << 16
         | static_cast<uint32_t>(rgb[1] * 255) << 8
         | static_cast<uint32_t>(rgb[0] * 255);
}

std::array<float, 3> ColorProvider::getFaceColorRgb(Color color) const {
    return getRgbForColor(color);
}
