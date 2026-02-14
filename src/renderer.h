#ifndef RENDERER_H
#define RENDERER_H

#include "cube.h"
#include <imgui.h>

class CubeRenderer {
public:
    CubeRenderer();
    ~CubeRenderer() = default;

    // Render 2D unfolded cube net view
    void draw2D(ImDrawList* drawList, ImVec2 offset, float scale);

    // Render 3D isometric view
    void draw3D(ImDrawList* drawList, ImVec2 offset, float scale);

    void executeMove(Move move);
    void reset();
    void resetView();  // Reset 3D view parameters to defaults
    bool isSolved() const;

    float rotationX = -30.0f;
    float rotationY = 45.0f;
    float rotationZ = 0.0f;
    float scale = 3.1f;
    float scale2D = 1.0f;  // 2D view zoom level

private:
    RubiksCube cube_;

    // Project 3D point to 2D screen coordinates
    ImVec2 project(float x, float y, float z, ImVec2 center, float scale);

    // Draw a 2D face (unfolded cube net view)
    void drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                 ImVec2 offset, float size, float gap);

    // Draw a 3D face
    void draw3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                    const ImVec2 (&vertices)[4], ImVec2 center, float scale);

    // Get face color for 3D drawing
    ImU32 getFaceColor(Color color);
};

#endif // RENDERER_H
