#ifndef RENDERER_2D_H
#define RENDERER_2D_H

#include <array>
#include <imgui.h>
#include "cube.h"
#include "color.h"

class Renderer2D {
public:
    Renderer2D();
    
    // Render 2D unfolded view
    void draw(ImDrawList* drawList, ImVec2 center, float scale,
              const RubiksCube& cube, const ColorProvider& colors);
    
private:
    void drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                 ImVec2 offset, float size, float gap,
                 const ColorProvider& colors);
};

#endif // RENDERER_2D_H
