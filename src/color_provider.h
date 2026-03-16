#ifndef COLOR_PROVIDER_H
#define COLOR_PROVIDER_H

#include <array>
#include <cstdint>
#include "config.h"
#include "cube.h"

namespace DefaultColors {
    constexpr std::array<float, 3> WHITE  = {1.0f, 1.0f, 1.0f};
    constexpr std::array<float, 3> YELLOW = {1.0f, 1.0f, 0.0f};
    constexpr std::array<float, 3> RED    = {1.0f, 0.0f, 0.0f};
    constexpr std::array<float, 3> ORANGE = {1.0f, 0.55f, 0.0f};
    constexpr std::array<float, 3> GREEN  = {0.0f, 1.0f, 0.0f};
    constexpr std::array<float, 3> BLUE   = {0.0f, 0.4f, 1.0f};
    constexpr std::array<float, 3> BLACK  = {0.0f, 0.0f, 0.0f};
    
    inline const std::array<float, 3>& forFace(Face face) {
        switch (face) {
            case Face::UP: return WHITE;
            case Face::DOWN: return YELLOW;
            case Face::FRONT: return GREEN;
            case Face::BACK: return BLUE;
            case Face::LEFT: return ORANGE;
            case Face::RIGHT: return RED;
            default: return BLACK;
        }
    }
}

class ColorProvider {
public:
    ColorProvider();
    
    void setCustomColors(const ColorConfig& config);
    void resetToDefaults();
    
    uint32_t getFaceColor(Color color) const;
    std::array<float, 3> getFaceColorRgb(Color color) const;
    
    std::array<float, 3> customFront = DefaultColors::GREEN;
    std::array<float, 3> customBack = DefaultColors::BLUE;
    std::array<float, 3> customLeft = DefaultColors::ORANGE;
    std::array<float, 3> customRight = DefaultColors::RED;
    std::array<float, 3> customUp = DefaultColors::WHITE;
    std::array<float, 3> customDown = DefaultColors::YELLOW;
    bool useCustomColors = false;

private:
    std::array<float, 3> getRgbForColor(Color color) const;
};

#endif // COLOR_PROVIDER_H
