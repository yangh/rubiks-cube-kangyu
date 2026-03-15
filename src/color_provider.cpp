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

ImU32 ColorProvider::getFaceColor(Color color) const {
    std::array<float, 3> rgb;

    if (useCustomColors) {
        switch (color) {
            case Color::GREEN:  rgb = customFront; break;
            case Color::BLUE:   rgb = customBack; break;
            case Color::ORANGE: rgb = customLeft; break;
            case Color::RED:    rgb = customRight; break;
            case Color::WHITE:  rgb = customUp; break;
            case Color::YELLOW: rgb = customDown; break;
            default:            rgb = colorToRgb(color); break;
        }
    } else {
        rgb = colorToRgb(color);
    }

    return IM_COL32(
        static_cast<int>(rgb[0] * 255),
        static_cast<int>(rgb[1] * 255),
        static_cast<int>(rgb[2] * 255),
        255
    );
}

std::array<float, 3> ColorProvider::getFaceColorRgb(Color color) const {
    std::array<float, 3> rgb;

    if (useCustomColors) {
        switch (color) {
            case Color::GREEN:  rgb = customFront; break;
            case Color::BLUE:   rgb = customBack; break;
            case Color::ORANGE: rgb = customLeft; break;
            case Color::RED:    rgb = customRight; break;
            case Color::WHITE:  rgb = customUp; break;
            case Color::YELLOW: rgb = customDown; break;
            default:            rgb = colorToRgb(color); break;
        }
    } else {
        rgb = colorToRgb(color);
    }

    return rgb;
}
