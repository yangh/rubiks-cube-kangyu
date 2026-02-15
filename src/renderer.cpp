#include "renderer.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// Global flag to enable/disable cube dump (defined in main.cpp)
extern bool g_enableDump;

CubeRenderer::CubeRenderer()
    : cube_()
{
}

void CubeRenderer::executeMove(Move move) {
    moveQueue_.push(move);

    if (!isAnimating_ && moveQueue_.size() == 1) {
        startNextAnimation();
    }

    if (g_enableDump) {
        std::cout << "\n=== Queued " << moveToString(move) << " ===" << std::endl;
    }
}

void CubeRenderer::reset() {
    cube_.reset();
    isAnimating_ = false;
    animationProgress_ = 0.0f;
    while (!moveQueue_.empty()) {
        moveQueue_.pop();
    }
}

void CubeRenderer::resetView() {
    rotationX = -160.0f;
    rotationY = 15.0f;
    rotationZ = 0.0f;
    scale = 3.1f;
    scale2D = 0.8f;
}

bool CubeRenderer::isSolved() const {
    return cube_.isSolved();
}

void CubeRenderer::dump() const {
    cube_.dump();
}

void CubeRenderer::draw2D(ImDrawList* drawList, ImVec2 offset, float scale) {
    // Draw cube net (2D unfolded view)
    float stickerSize = 30.0f * scale;
    float gap = 3.0f * scale;
    float faceSize = stickerSize * 3.0f + gap * 2.0f;

    // Standard cross-shaped layout: Up above Front, Left beside Front
    // Face spacing
    float spacing = faceSize + gap;
    offset.x -= spacing * 0.5;
    //offset.y += spacing * 0.5;

    // Draw in order: Up, Left, Front, Right, Down, Back
    drawFace(drawList, cube_.getUp(),
            ImVec2(offset.x, offset.y - spacing),
            stickerSize, gap);

    drawFace(drawList, cube_.getLeft(),
            ImVec2(offset.x - spacing, offset.y),
            stickerSize, gap);
    drawFace(drawList, cube_.getFront(),
            ImVec2(offset.x + spacing * 0, offset.y),
            stickerSize, gap);
    drawFace(drawList, cube_.getRight(),
            ImVec2(offset.x + spacing * 1, offset.y),
            stickerSize, gap);
    drawFace(drawList, cube_.getBack(),
            ImVec2(offset.x + spacing * 2, offset.y),
            stickerSize, gap);

    drawFace(drawList, cube_.getDown(),
            ImVec2(offset.x, offset.y + spacing),
            stickerSize, gap);
}

void CubeRenderer::draw3D(ImDrawList* drawList, ImVec2 offset, float scale) {
    float size = 40.0f * scale;

    // Define vertices for each face (before rotation)
    // Cube centered at origin, extending from -1 to +1
    struct FaceDrawInfo {
        int faceIndex;  // 0:Front, 1:Back, 2:Left, 3:Right, 4:Up, 5:Down
        float centerX, centerY, centerZ;  // Face center in 3D space
    };

    // Front face: z = +1
    ImVec2 frontVerts[4] = {
        ImVec2(-1, 1), ImVec2(1, 1), ImVec2(1, -1), ImVec2(-1, -1)
    };

    // Back face: z = -1
    ImVec2 backVerts[4] = {
        ImVec2(1, 1), ImVec2(-1, 1), ImVec2(-1, -1), ImVec2(1, -1)
    };

    // Left face: x = -1
    ImVec2 leftVerts[4] = {
        ImVec2(-1, 1), ImVec2(-1, -1), ImVec2(1, -1), ImVec2(1, 1)
    };

    // Right face: x = +1
    ImVec2 rightVerts[4] = {
        ImVec2(1, 1), ImVec2(1, -1), ImVec2(-1, -1), ImVec2(-1, 1)
    };

    // Up face: y = +1
    ImVec2 upVerts[4] = {
        ImVec2(-1, 1), ImVec2(-1, -1), ImVec2(1, -1), ImVec2(1, 1)
    };

    // Down face: y = -1
    ImVec2 downVerts[4] = {
        ImVec2(-1, -1), ImVec2(-1, 1), ImVec2(1, 1), ImVec2(1, -1)
    };

    // Calculate rotated face centers for depth sorting
    float angleX = rotationX * M_PI / 180.0f;
    float angleY = rotationY * M_PI / 180.0f;

    // Function to rotate a 3D point
    auto rotatePoint = [](float x, float y, float z, float ax, float ay) -> std::array<float, 3> {
        float y1 = y * cosf(ax) - z * sinf(ax);
        float z1 = y * sinf(ax) + z * cosf(ax);
        float x2 = x * cosf(ay) + z1 * sinf(ay);
        float z2 = -x * sinf(ay) + z1 * cosf(ay);
        return {x2, y1, z2};
    };

    auto getDepth = [&](float x, float y, float z) -> float {
        auto rotated = rotatePoint(x, y, z, angleX, angleY);
        return rotated[2];  // Depth is z after rotation
    };

    // Create face draw information
    FaceDrawInfo faces[6] = {
        {0, 0, 0, 1},
        {1, 0, 0, -1},
        {2, -1, 0, 0},
        {3, 1, 0, 0},
        {4, 0, 1, 0},
        {5, 0, -1, 0}
    };

    // Sort faces by depth (far to near)
    int indices[6] = {0, 1, 2, 3, 4, 5};
    for (int i = 0; i < 6; ++i) {
        for (int j = i + 1; j < 6; ++j) {
            float depthI = getDepth(faces[indices[i]].centerX, faces[indices[i]].centerY, faces[indices[i]].centerZ);
            float depthJ = getDepth(faces[indices[j]].centerX, faces[indices[j]].centerY, faces[indices[j]].centerZ);
            if (depthI < depthJ) {  // Smaller z = farther
                std::swap(indices[i], indices[j]);
            }
        }
    }

    // Draw faces in sorted order (far to near)
    for (int i = 0; i < 6; ++i) {
        int idx = indices[i];
        if (idx == 0) {
            if (isAnimating_) {
                drawAnimated3DFace(drawList, preAnimationCube_.getFront(), frontVerts, offset, size, 0);
            } else {
                draw3DFace(drawList, cube_.getFront(), frontVerts, offset, size);
            }
        }
        else if (idx == 1) {
            if (isAnimating_) {
                drawAnimated3DFace(drawList, preAnimationCube_.getBack(), backVerts, offset, size, 1);
            } else {
                draw3DFace(drawList, cube_.getBack(), backVerts, offset, size);
            }
        }
        else if (idx == 2) {
            if (isAnimating_) {
                drawAnimated3DFace(drawList, preAnimationCube_.getLeft(), leftVerts, offset, size, 2);
            } else {
                draw3DFace(drawList, cube_.getLeft(), leftVerts, offset, size);
            }
        }
        else if (idx == 3) {
            if (isAnimating_) {
                drawAnimated3DFace(drawList, preAnimationCube_.getRight(), rightVerts, offset, size, 3);
            } else {
                draw3DFace(drawList, cube_.getRight(), rightVerts, offset, size);
            }
        }
        else if (idx == 4) {
            if (isAnimating_) {
                drawAnimated3DFace(drawList, preAnimationCube_.getUp(), upVerts, offset, size, 4);
            } else {
                draw3DFace(drawList, cube_.getUp(), upVerts, offset, size);
            }
        }
        else if (idx == 5) {
            if (isAnimating_) {
                drawAnimated3DFace(drawList, preAnimationCube_.getDown(), downVerts, offset, size, 5);
            } else {
                draw3DFace(drawList, cube_.getDown(), downVerts, offset, size);
            }
        }
    }
}

ImVec2 CubeRenderer::project(float x, float y, float z, ImVec2 center, float scale) {
    // Isometric-style projection
    float angleX = rotationX * M_PI / 180.0f;
    float angleY = rotationY * M_PI / 180.0f;
    float angleZ = rotationZ * M_PI / 180.0f;

    // Rotate around X axis
    float y1 = y * cosf(angleX) - z * sinf(angleX);
    float z1 = y * sinf(angleX) + z * cosf(angleX);

    // Rotate around Y axis
    float x2 = x * cosf(angleY) + z1 * sinf(angleY);
    float z2 = -x * sinf(angleY) + z1 * cosf(angleY);

    // Rotate around Z axis (in the XY plane)
    float x3 = x2 * cosf(angleZ) - y1 * sinf(angleZ);
    float y3 = x2 * sinf(angleZ) + y1 * cosf(angleZ);

    return ImVec2(
        center.x + x3 * scale,
        center.y + y3 * scale
    );
}

void CubeRenderer::drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                          ImVec2 offset, float size, float gap, bool flipVertical) {
    float totalSize = size * 3.0f + gap * 2.0f;
    float startX = offset.x - totalSize / 2.0f + size / 2.0f;
    float startY = offset.y - totalSize / 2.0f + size / 2.0f;

    // Draw black background for face
    ImVec2 p1(offset.x - totalSize / 2.0f, offset.y + totalSize / 2.0f);
    ImVec2 p2(offset.x + totalSize / 2.0f, offset.y + totalSize / 2.0f);
    ImVec2 p3(offset.x + totalSize / 2.0f, offset.y - totalSize / 2.0f);
    ImVec2 p4(offset.x - totalSize / 2.0f, offset.y - totalSize / 2.0f);
    drawList->AddQuadFilled(p1, p2, p3, p4, IM_COL32(25, 25, 25, 255));

    // Draw 3x3 grid of stickers
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            // Map visual position to face index, with optional vertical flip
            // Normal: row 0 -> indices 0,1,2 (top)
            // Flipped: row 0 -> indices 6,7,8 (bottom)
            int index;
            if (flipVertical) {
                // Flip vertically: top visually = bottom in face indices
                index = (2 - row) * 3 + col;
            } else {
                index = row * 3 + col;
            }

            std::array<float, 3> rgb = colorToRgb(face[index]);
            ImU32 color = IM_COL32(
                static_cast<int>(rgb[0] * 255),
                static_cast<int>(rgb[1] * 255),
                static_cast<int>(rgb[2] * 255),
                255
            );
            float x = startX + static_cast<float>(col) * (size + gap);
            float y = startY + static_cast<float>(row) * (size + gap);

            // Draw sticker
            ImVec2 s1(x - size / 2.0f, y + size / 2.0f);
            ImVec2 s2(x + size / 2.0f, y + size / 2.0f);
            ImVec2 s3(x + size / 2.0f, y - size / 2.0f);
            ImVec2 s4(x - size / 2.0f, y - size / 2.0f);
            drawList->AddQuadFilled(s1, s2, s3, s4, color);

            // Draw black border
            ImU32 black = IM_COL32(0, 0, 0, 255);
            drawList->AddQuad(s1, s2, s3, s4, black, 2.0f);
        }
    }
}

void CubeRenderer::draw3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                            const ImVec2 (&faceVerts)[4], ImVec2 center, float size) {
    // Project vertices to screen space
    ImVec2 projected[4];
    for (int i = 0; i < 4; ++i) {
        float x = 0, y = 0, z = 0;

        // Map face vertices to 3D positions
        if (face == cube_.getFront()) { z = 1.0f; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (face == cube_.getBack()) { z = -1.0f; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (face == cube_.getLeft()) { x = -1.0f; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (face == cube_.getRight()) { x = 1.0f; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (face == cube_.getUp()) { y = 1.0f; x = faceVerts[i].x; z = faceVerts[i].y; }
        else if (face == cube_.getDown()) { y = -1.0f; x = faceVerts[i].x; z = faceVerts[i].y; }

        projected[i] = project(x * 1.1f, y * 1.1f, z * 1.1f, center, size);
    }

    // Draw black background for face
    drawList->AddQuadFilled(projected[0], projected[1], projected[2], projected[3],
                          IM_COL32(20, 20, 20, 255));

    // Draw 3x3 stickers on the face
    // Each sticker is at a position in the face's local coordinate system
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int index = row * 3 + col;
            float u = (col - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67
            float v = (row - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67

            // Get face color
            ImU32 stickerColor = getFaceColor(face[index]);

            // Calculate sticker center position in 3D space
            // u = x offset (left to right), v = y offset (top to bottom) in face-local space
            float x = 0, y = 0, z = 0;

            if (face == cube_.getFront()) {
                // Front face (z=+1): col -> x, row -> -y
                x = u; y = -v; z = 1.0f;
            } else if (face == cube_.getBack()) {
                // Back face (z=-1): col -> -x, row -> -y (mirrored when viewing from front)
                x = -u; y = -v; z = -1.0f;
            } else if (face == cube_.getLeft()) {
                // Left face (x=-1): col -> z, row -> -y
                // col 0 (left in 2D) should be at negative z (back), col 2 at positive z (front)
                x = -1.0f; y = -v; z = u;
            } else if (face == cube_.getRight()) {
                // Right face (x=+1): col -> -z, row -> -y
                // col 0 (left in 2D) should be at positive z (front), col 2 at negative z (back)
                x = 1.0f; y = -v; z = -u;
            } else if (face == cube_.getUp()) {
                // Up face (y=+1): col -> x, row -> z
                // row 0 (top in 2D) should be at positive z (back), row 2 at negative z (front)
                x = u; y = 1.0f; z = v;
            } else if (face == cube_.getDown()) {
                // Down face (y=-1): col -> x, row -> -z
                // row 0 (top in 2D) should be at negative z (front), row 2 at positive z (back)
                x = u; y = -1.0f; z = -v;
            }

            // Calculate sticker corners relative to sticker center
            float stickerSize = 0.6f;
            float halfSize = stickerSize / 2.0f;

            // Sticker corners in face-local coordinates (dx, dy offset from center)
            // dx = horizontal offset (left/right), dy = vertical offset (up/down)
            float cornersLocal[4][2] = {
                {-halfSize, halfSize},  // top-left: dx negative, dy positive
                {halfSize, halfSize},   // top-right: dx positive, dy positive
                {halfSize, -halfSize},  // bottom-right: dx positive, dy negative
                {-halfSize, -halfSize}  // bottom-left: dx negative, dy negative
            };

            // Project all 4 corners to screen space
            ImVec2 corners[4];
            for (int c = 0; c < 4; ++c) {
                float dx = cornersLocal[c][0];  // horizontal offset
                float dy = cornersLocal[c][1];  // vertical offset
                float cx = 0, cy = 0, cz = 0;

                // Map local offsets to 3D space based on face orientation
                if (face == cube_.getFront()) {
                    cx = x + dx; cy = y + dy; cz = 1.0f;
                } else if (face == cube_.getBack()) {
                    cx = x - dx; cy = y + dy; cz = -1.0f;
                } else if (face == cube_.getLeft()) {
                    cx = -1.0f; cy = y + dy; cz = z + dx;
                } else if (face == cube_.getRight()) {
                    cx = 1.0f; cy = y + dy; cz = z - dx;
                } else if (face == cube_.getUp()) {
                    cx = x + dx; cy = 1.0f; cz = z + dy;
                } else if (face == cube_.getDown()) {
                    cx = x + dx; cy = -1.0f; cz = z - dy;
                }

                corners[c] = project(cx * 1.1f, cy * 1.1f, cz * 1.1f, center, size);
            }

            // Draw sticker
            drawList->AddQuadFilled(corners[0], corners[1], corners[2], corners[3], stickerColor);
            // Draw black border around sticker to reduce aliasing
            ImU32 black = IM_COL32(0, 0, 0, 255);
            drawList->AddQuad(corners[0], corners[1], corners[2], corners[3], black, 1.5f);
        }
    }
}

void CubeRenderer::drawAnimated3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                                      const ImVec2 (&faceVerts)[4], ImVec2 center, float size,
                                      int faceIndex) {
    // Quadratic ease-in-out: 3t^2 - 2t^3
    float easeProgress = animationProgress_ * animationProgress_ * (3.0f - 2.0f * animationProgress_);
    float currentAngle = 90.0f * easeProgress;  // Always use positive 90 degrees, rotateSticker handles direction

    // Draw black background for face
    ImVec2 projected[4];
    for (int i = 0; i < 4; ++i) {
        float x = 0, y = 0, z = 0;

        // Map face vertices to 3D positions
        if (faceIndex == 0) { z = 1.0f; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (faceIndex == 1) { z = -1.0f; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (faceIndex == 2) { x = -1.0f; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (faceIndex == 3) { x = 1.0f; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (faceIndex == 4) { y = 1.0f; x = faceVerts[i].x; z = faceVerts[i].y; }
        else if (faceIndex == 5) { y = -1.0f; x = faceVerts[i].x; z = faceVerts[i].y; }

        projected[i] = project(x * 1.1f, y * 1.1f, z * 1.1f, center, size);
    }

    drawList->AddQuadFilled(projected[0], projected[1], projected[2], projected[3],
                          IM_COL32(20, 20, 20, 255));

    // Draw 3x3 stickers on the face
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int stickerIndex = row * 3 + col;
            float u = (col - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67
            float v = (row - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67

            // Get face color
            ImU32 stickerColor = getFaceColor(face[stickerIndex]);

            // Calculate sticker center position in 3D space
            float x = 0, y = 0, z = 0;

            if (faceIndex == 0) { x = u; y = -v; z = 1.0f; }
            else if (faceIndex == 1) { x = -u; y = -v; z = -1.0f; }
            else if (faceIndex == 2) { x = -1.0f; y = -v; z = u; }
            else if (faceIndex == 3) { x = 1.0f; y = -v; z = -u; }
            else if (faceIndex == 4) { x = u; y = 1.0f; z = v; }
            else if (faceIndex == 5) { x = u; y = -1.0f; z = -v; }

            // Calculate sticker corners relative to sticker center
            float stickerSize = 0.6f;
            float halfSize = stickerSize / 2.0f;

            // Sticker corners in face-local coordinates (dx, dy offset from center)
            float cornersLocal[4][2] = {
                {-halfSize, halfSize},
                {halfSize, halfSize},
                {halfSize, -halfSize},
                {-halfSize, -halfSize}
            };

            // Project all 4 corners to screen space
            ImVec2 corners[4];
            for (int c = 0; c < 4; ++c) {
                float dx = cornersLocal[c][0];
                float dy = cornersLocal[c][1];
                float cx = 0, cy = 0, cz = 0;

                // Map local offsets to 3D space based on face orientation
                if (faceIndex == 0) { cx = x + dx; cy = y + dy; cz = 1.0f; }
                else if (faceIndex == 1) { cx = x - dx; cy = y + dy; cz = -1.0f; }
                else if (faceIndex == 2) { cx = -1.0f; cy = y + dy; cz = z + dx; }
                else if (faceIndex == 3) { cx = 1.0f; cy = y + dy; cz = z - dx; }
                else if (faceIndex == 4) { cx = x + dx; cy = 1.0f; cz = z + dy; }
                else if (faceIndex == 5) { cx = x + dx; cy = -1.0f; cz = z - dy; }

                // Apply animation rotation if this sticker is part of the rotating slice
                if (isStickerAnimating(currentMove_, faceIndex, stickerIndex)) {
                    std::array<float, 3> rotated = rotateSticker({cx, cy, cz}, currentMove_, currentAngle);
                    cx = rotated[0];
                    cy = rotated[1];
                    cz = rotated[2];
                }

                corners[c] = project(cx * 1.1f, cy * 1.1f, cz * 1.1f, center, size);
            }

            // Draw sticker
            drawList->AddQuadFilled(corners[0], corners[1], corners[2], corners[3], stickerColor);
            ImU32 black = IM_COL32(0, 0, 0, 255);
            drawList->AddQuad(corners[0], corners[1], corners[2], corners[3], black, 1.5f);
        }
    }
}

ImU32 CubeRenderer::getFaceColor(Color color) {
    std::array<float, 3> rgb = colorToRgb(color);
    return IM_COL32(
        static_cast<int>(rgb[0] * 255),
        static_cast<int>(rgb[1] * 255),
        static_cast<int>(rgb[2] * 255),
        255
    );
}

// Animation implementation
void CubeRenderer::updateAnimation(float deltaTime) {
    if (!isAnimating_) return;

    const float ANIMATION_DURATION = 0.2f;  // 200ms
    animationProgress_ += deltaTime / ANIMATION_DURATION;

    if (animationProgress_ >= 1.0f) {
        animationProgress_ = 1.0f;
        cube_.executeMove(currentMove_);  // Apply actual move
        isAnimating_ = false;

        if (g_enableDump) {
            std::cout << "\n=== Completed " << moveToString(currentMove_) << " ===" << std::endl;
            cube_.dump();
        }

        startNextAnimation();  // Start next queued move
    }
}

void CubeRenderer::startNextAnimation() {
    if (moveQueue_.empty()) {
        isAnimating_ = false;
        return;
    }

    isAnimating_ = true;
    animationProgress_ = 0.0f;
    currentMove_ = moveQueue_.front();
    moveQueue_.pop();
    preAnimationCube_ = cube_;  // Save current state

    if (g_enableDump) {
        std::cout << "\n=== Starting " << moveToString(currentMove_) << " ===" << std::endl;
    }
}

// Check if a specific sticker is part of the rotating slice for the current move
bool CubeRenderer::isStickerAnimating(Move move, int faceIndex, int stickerIndex) const {
    // faceIndex: 0=Front, 1=Back, 2=Left, 3=Right, 4=Up, 5=Down
    // stickerIndex: 0-8 (row*3 + col)

    switch (move) {
        // U move: Up face (all 9) + Front/Left/Back/Right top row (0,1,2)
        case Move::U:
        case Move::UP:
            if (faceIndex == 4) return true;  // Up face
            if (stickerIndex >= 0 && stickerIndex <= 2) {  // Top row
                return faceIndex == 0 || faceIndex == 2 || faceIndex == 1 || faceIndex == 3;  // F, L, B, R
            }
            return false;

        // D move: Down face (all 9) + Front/Left/Back/Right bottom row (6,7,8)
        case Move::D:
        case Move::DP:
            if (faceIndex == 5) return true;  // Down face
            if (stickerIndex >= 6 && stickerIndex <= 8) {  // Bottom row
                return faceIndex == 0 || faceIndex == 2 || faceIndex == 1 || faceIndex == 3;  // F, L, B, R
            }
            return false;

        // L move: Left face (all 9) + Up/Front/Down/Back left column (0,3,6)
        case Move::L:
        case Move::LP:
            if (faceIndex == 2) return true;  // Left face
            if (stickerIndex == 0 || stickerIndex == 3 || stickerIndex == 6) {  // Left column
                return faceIndex == 4 || faceIndex == 0 || faceIndex == 5 || faceIndex == 1;  // U, F, D, B
            }
            return false;

        // R move: Right face (all 9) + Up/Front/Down/Back right column (2,5,8)
        case Move::R:
        case Move::RP:
            if (faceIndex == 3) return true;  // Right face
            if (stickerIndex == 2 || stickerIndex == 5 || stickerIndex == 8) {  // Right column
                return faceIndex == 4 || faceIndex == 0 || faceIndex == 5 || faceIndex == 1;  // U, F, D, B
            }
            return false;

        // F move: Front face (all 9) + Up[6,7,8] + Left[2,5,8] + Down[0,1,2] + Right[0,3,6]
        case Move::F:
        case Move::FP:
            if (faceIndex == 0) return true;  // Front face
            if (faceIndex == 4 && stickerIndex >= 6 && stickerIndex <= 8) return true;  // Up bottom row
            if (faceIndex == 2 && (stickerIndex == 2 || stickerIndex == 5 || stickerIndex == 8)) return true;  // Left right col
            if (faceIndex == 5 && stickerIndex >= 0 && stickerIndex <= 2) return true;  // Down top row
            if (faceIndex == 3 && (stickerIndex == 0 || stickerIndex == 3 || stickerIndex == 6)) return true;  // Right left col
            return false;

        // B move: Back face (all 9) + Up[0,1,2] + Left[0,3,6] + Down[6,7,8] + Right[2,5,8]
        case Move::B:
        case Move::BP:
            if (faceIndex == 1) return true;  // Back face
            if (faceIndex == 4 && stickerIndex >= 0 && stickerIndex <= 2) return true;  // Up top row
            if (faceIndex == 2 && (stickerIndex == 0 || stickerIndex == 3 || stickerIndex == 6)) return true;  // Left left col
            if (faceIndex == 5 && stickerIndex >= 6 && stickerIndex <= 8) return true;  // Down bottom row
            if (faceIndex == 3 && (stickerIndex == 2 || stickerIndex == 5 || stickerIndex == 8)) return true;  // Right right col
            return false;

        // M move: Middle slice - Up[1,4,7] + Down[1,4,7] + Front[1,4,7] + Back[1,4,7]
        case Move::M:
        case Move::MP:
            if (stickerIndex == 1 || stickerIndex == 4 || stickerIndex == 7) {
                return faceIndex == 4 || faceIndex == 5 || faceIndex == 0 || faceIndex == 1;  // U, D, F, B
            }
            return false;

        // E move: Equator slice - Front[3,4,5] + Back[3,4,5] + Left[3,4,5] + Right[3,4,5]
        case Move::E:
        case Move::EP:
            if (stickerIndex >= 3 && stickerIndex <= 5) {
                return faceIndex == 0 || faceIndex == 1 || faceIndex == 2 || faceIndex == 3;  // F, B, L, R
            }
            return false;

        // S move: Standing slice - Up[3,4,5] + Down[3,4,5] + Left[1,4,7] + Right[1,4,7]
        case Move::S:
        case Move::SP:
            if (faceIndex == 4 || faceIndex == 5) {  // U or D
                return stickerIndex >= 3 && stickerIndex <= 5;  // Middle row
            }
            if (faceIndex == 2 || faceIndex == 3) {  // L or R
                return stickerIndex == 1 || stickerIndex == 4 || stickerIndex == 7;  // Middle column
            }
            return false;

        default:
            return false;
    }
}

// Apply rotation transformation to a 3D position based on the current move
std::array<float, 3> CubeRenderer::rotateSticker(const std::array<float, 3>& pos, Move move, float angle) const {
    // Convert angle to radians
    float rad = angle * M_PI / 180.0f;
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    float x = pos[0];
    float y = pos[1];
    float z = pos[2];

    switch (move) {
        // U moves rotate around Y axis (looking down)
        case Move::U:
            return {x * cosA - z * sinA, y, x * sinA + z * cosA};
        case Move::UP:
            return {x * cosA + z * sinA, y, -x * sinA + z * cosA};

        // D moves rotate around Y axis (looking up - opposite direction of U)
        case Move::D:
            return {x * cosA + z * sinA, y, -x * sinA + z * cosA};
        case Move::DP:
            return {x * cosA - z * sinA, y, x * sinA + z * cosA};

        // L moves rotate around X axis (looking from left)
        case Move::L:
            return {x, y * cosA - z * sinA, y * sinA + z * cosA};
        case Move::LP:
            return {x, y * cosA + z * sinA, -y * sinA + z * cosA};

        // R moves rotate around X axis (looking from right - opposite direction of L)
        case Move::R:
            return {x, y * cosA + z * sinA, -y * sinA + z * cosA};
        case Move::RP:
            return {x, y * cosA - z * sinA, y * sinA + z * cosA};

        // F moves rotate around Z axis (looking from front)
        case Move::F:
            return {x * cosA + y * sinA, -x * sinA + y * cosA, z};
        case Move::FP:
            return {x * cosA - y * sinA, x * sinA + y * cosA, z};

        // B moves rotate around Z axis (looking from back - opposite direction of F)
        case Move::B:
            return {x * cosA - y * sinA, x * sinA + y * cosA, z};
        case Move::BP:
            return {x * cosA + y * sinA, -x * sinA + y * cosA, z};

        // M moves rotate around X axis (same as L)
        case Move::M:
            return {x, y * cosA - z * sinA, y * sinA + z * cosA};
        case Move::MP:
            return {x, y * cosA + z * sinA, -y * sinA + z * cosA};

        // E moves rotate around Y axis (same as D)
        case Move::E:
            return {x * cosA + z * sinA, y, -x * sinA + z * cosA};
        case Move::EP:
            return {x * cosA - z * sinA, y, x * sinA + z * cosA};

        // S moves rotate around Z axis (same as F)
        case Move::S:
            return {x * cosA + y * sinA, -x * sinA + y * cosA, z};
        case Move::SP:
            return {x * cosA - y * sinA, x * sinA + y * cosA, z};

        default:
            return pos;
    }
}
