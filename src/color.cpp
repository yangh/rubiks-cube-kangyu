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

bool isOppositeColor(Color a, Color b) {
    static const Color opposite[] = {
        Color::YELLOW,  // WHITE → opposite is YELLOW
        Color::WHITE,   // YELLOW → opposite is WHITE
        Color::ORANGE,  // RED → opposite is ORANGE
        Color::RED,     // ORANGE → opposite is RED
        Color::BLUE,    // GREEN → opposite is BLUE
        Color::GREEN    // BLUE → opposite is GREEN
    };
    return opposite[static_cast<int>(a)] == b;
}

ColorProvider::ColorProvider() = default;

void ColorProvider::setCustomColors(const CubeConfig& config) {
    customFront_ = config.front();
    customBack_  = config.back();
    customLeft_  = config.left();
    customRight_ = config.right();
    customUp_    = config.up();
    customDown_  = config.down();
    useCustomColors_ = !config.isUsingDefaults();
}

void ColorProvider::resetToDefaults() {
    customFront_ = DefaultColorRGB::GREEN;
    customBack_  = DefaultColorRGB::BLUE;
    customLeft_  = DefaultColorRGB::ORANGE;
    customRight_ = DefaultColorRGB::RED;
    customUp_    = DefaultColorRGB::WHITE;
    customDown_  = DefaultColorRGB::YELLOW;
    useCustomColors_ = false;
}

uint32_t ColorProvider::getFaceColor(Color color) const {
    RgbColor rgb = getFaceColorRgb(color);
    return static_cast<uint32_t>(255) << 24
         | static_cast<uint32_t>(rgb.b * 255) << 16
         | static_cast<uint32_t>(rgb.g * 255) << 8
         | static_cast<uint32_t>(rgb.r * 255);
}

RgbColor ColorProvider::getFaceColorRgb(Color color) const {
    int idx = static_cast<int>(color);
    if (idx < 0 || idx >= 6) return DefaultColorRGB::BLACK;

    static const RgbColor defaults[] = {
        DefaultColorRGB::WHITE, DefaultColorRGB::YELLOW,
        DefaultColorRGB::RED,   DefaultColorRGB::ORANGE,
        DefaultColorRGB::GREEN, DefaultColorRGB::BLUE
    };

    if (!useCustomColors_) return defaults[idx];

    const RgbColor* custom[] = {
        &customUp_, &customDown_, &customRight_,
        &customLeft_, &customFront_, &customBack_
    };
    return *custom[idx];
}
