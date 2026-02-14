#ifndef RENDERER_H
#define RENDERER_H

#include "cube.h"
#include <imgui.h>

class CubeRenderer {
public:
    CubeRenderer();
    ~CubeRenderer() = default;

    // Render cube using ImGui draw list
    void draw(ImDrawList* drawList, ImVec2 offset, float scale);

    void executeMove(Move move);
    void reset();
    bool isSolved() const;

    float rotationX = -30.0f;
    float rotationY = 45.0f;
    float scale = 0.7f;

private:
    RubiksCube cube_;

    // Project 3D point to 2D screen coordinates
    ImVec2 project(float x, float y, float z, ImVec2 center, float scale);

    // Draw a 2D face (unfolded cube net view)
    void drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                 ImVec2 offset, float size, float gap);
};

#endif // RENDERER_H
