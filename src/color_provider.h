#ifndef COLOR_PROVIDER_H
#define COLOR_PROVIDER_H

#include <array>
#include "config.h"
#include <imgui.h>
#include "cube.h"

class ColorProvider {
public:
    ColorProvider();
    
    // Set custom colors from config
    void setCustomColors(const ColorConfig& config);
    void resetToDefaults();
    
    // Get colors
    ImU32 getFaceColor(Color color) const;
    std::array<float, 3> getFaceColorRgb(Color color) const;
    
    // Public state (for UI direct access)
    std::array<float, 3> customFront = {0.0f, 1.0f, 0.0f};
    std::array<float, 3> customBack = {0.0f, 0.0f, 1.0f};
    std::array<float, 3> customLeft = {1.0f, 0.5f, 0.0f};
    std::array<float, 3> customRight = {1.0f, 0.0f, 0.0f};
    std::array<float, 3> customUp = {1.0f, 1.0f, 1.0f};
    std::array<float, 3> customDown = {1.0f, 1.0f, 0.0f};
    bool useCustomColors = false;
};

#endif // COLOR_PROVIDER_H
