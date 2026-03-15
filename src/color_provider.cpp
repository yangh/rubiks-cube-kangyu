#include "color_provider.h"

ColorProvider::ColorProvider() = default;

void ColorProvider::setCustomColors(const ColorConfig& config) {
    customFront = config.getFrontColor();
    customBack = config.getBackColor();
    customLeft = config.getLeftColor();
    customRight = config.getRightColor();
    customUp = config.getUpColor();
    customDown = config.getDownColor();
    useCustomColors = !config.isUsingDefaults();
}

void ColorProvider::resetToDefaults() {
    customFront = DefaultColors::GREEN;
    customBack = DefaultColors::BLUE;
    customLeft = DefaultColors::ORANGE;
    customRight = DefaultColors::RED;
    customUp = DefaultColors::WHITE;
    customDown = DefaultColors::YELLOW;
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

ImU32 ColorProvider::getFaceColor(Color color) const {
    std::array<float, 3> rgb = getRgbForColor(color);
    return IM_COL32(
        static_cast<int>(rgb[0] * 255),
        static_cast<int>(rgb[1] * 255),
        static_cast<int>(rgb[2] * 255),
        255
    );
}

std::array<float, 3> ColorProvider::getFaceColorRgb(Color color) const {
    return getRgbForColor(color);
}
