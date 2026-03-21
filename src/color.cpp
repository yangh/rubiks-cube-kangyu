#include "color.h"
#include "config.h"

RgbColor colorToRgb(Color color) {
    switch (color) {
        case Color::WHITE:  return DefaultColorRGB::WHITE;
        case Color::YELLOW: return DefaultColorRGB::YELLOW;
        case Color::RED:    return DefaultColorRGB::RED;
        case Color::ORANGE: return DefaultColorRGB::ORANGE;
        case Color::GREEN:  return DefaultColorRGB::GREEN;
        case Color::BLUE:   return DefaultColorRGB::BLUE;
        default:            return DefaultColorRGB::BLACK;
    }
}

std::string colorToString(Color color) {
    static const char* names[] = {"W", "Y", "R", "O", "G", "B"};
    int idx = static_cast<int>(color);
    return (idx >= 0 && idx < 6) ? names[idx] : "?";
}

bool isOppositeColor(Color color1, Color color2) {
    return (color1 == Color::WHITE  && color2 == Color::YELLOW)  ||
           (color1 == Color::YELLOW && color2 == Color::WHITE)   ||
           (color1 == Color::RED    && color2 == Color::ORANGE)  ||
           (color1 == Color::ORANGE && color2 == Color::RED)     ||
           (color1 == Color::GREEN  && color2 == Color::BLUE)    ||
           (color1 == Color::BLUE   && color2 == Color::GREEN);
}

ColorProvider::ColorProvider() = default;

void ColorProvider::setCustomColors(const CubeConfig& config) {
    customFront = config.getFrontColor();
    customBack  = config.getBackColor();
    customLeft  = config.getLeftColor();
    customRight = config.getRightColor();
    customUp    = config.getUpColor();
    customDown  = config.getDownColor();
    useCustomColors = !config.isUsingDefaults();
}

void ColorProvider::resetToDefaults() {
    customFront = DefaultColorRGB::GREEN;
    customBack  = DefaultColorRGB::BLUE;
    customLeft  = DefaultColorRGB::ORANGE;
    customRight = DefaultColorRGB::RED;
    customUp    = DefaultColorRGB::WHITE;
    customDown  = DefaultColorRGB::YELLOW;
    useCustomColors = false;
}

RgbColor ColorProvider::getRgbForColor(Color color) const {
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
    RgbColor rgb = getRgbForColor(color);
    return static_cast<uint32_t>(255) << 24
         | static_cast<uint32_t>(rgb.b * 255) << 16
         | static_cast<uint32_t>(rgb.g * 255) << 8
         | static_cast<uint32_t>(rgb.r * 255);
}

RgbColor ColorProvider::getFaceColorRgb(Color color) const {
    return getRgbForColor(color);
}
