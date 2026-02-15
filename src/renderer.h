#ifndef RENDERER_H
#define RENDERER_H

#include "cube.h"
#include <imgui.h>
#include <queue>

class CubeRenderer {
public:
    CubeRenderer();
    ~CubeRenderer() = default;

    // Render 2D unfolded cube net view
    void draw2D(ImDrawList* drawList, ImVec2 offset, float scale);

    // Render 3D isometric view
    void draw3D(ImDrawList* drawList, ImVec2 offset, float scale);

    void executeMove(Move move);
    void updateAnimation(float deltaTime);
    void reset();
    void resetView();  // Reset 3D view parameters to defaults
    bool isSolved() const;
    void dump() const;  // Dump cube state to console

    float rotationX = -160.0f;
    float rotationY = 15.0f;
    float rotationZ = 0.0f;
    float scale = 3.1f;
    float scale2D = 0.8f;  // 2D view zoom level

private:
    RubiksCube cube_;

    // Animation state
    bool isAnimating_ = false;
    float animationProgress_ = 0.0f;  // 0.0 to 1.0
    Move currentMove_ = Move::U;
    RubiksCube preAnimationCube_;  // Cube state before animation
    std::queue<Move> moveQueue_;

    // Project 3D point to 2D screen coordinates
    ImVec2 project(float x, float y, float z, ImVec2 center, float scale);

    // Draw a 2D face (unfolded cube net view)
    // flipVertical: flip the face vertically (for Back face in unfolded view)
    void drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                 ImVec2 offset, float size, float gap, bool flipVertical = false);

    // Draw a 3D face
    void draw3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                    const ImVec2 (&vertices)[4], ImVec2 center, float scale);

    // Draw a 3D face with animation transformation applied
    void drawAnimated3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                            const ImVec2 (&faceVerts)[4], ImVec2 center, float size,
                            int faceIndex);

    // Get face color for 3D drawing
    ImU32 getFaceColor(Color color);

    // Animation helpers
    void startNextAnimation();
    bool isStickerAnimating(Move move, int faceIndex, int stickerIndex) const;
    std::array<float, 3> rotateSticker(const std::array<float, 3>& pos, Move move, float angle) const;
};

#endif // RENDERER_H
