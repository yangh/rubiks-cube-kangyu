#include "renderer.h"
#include <cmath>
#include <algorithm>

CubeRenderer::CubeRenderer()
    : cube_()
{
}

void CubeRenderer::executeMove(Move move) {
    cube_.executeMove(move);
}

void CubeRenderer::reset() {
    cube_.reset();
}

bool CubeRenderer::isSolved() const {
    return cube_.isSolved();
}

void CubeRenderer::draw(ImDrawList* drawList, ImVec2 offset, float scale) {
    // Draw cube net (2D unfolded view)
    float stickerSize = 30.0f * scale;
    float gap = 3.0f * scale;
    float faceSize = stickerSize * 3.0f + gap * 2.0f;

    // Position offsets for each face in the net layout
    // Standard cube net: F in center, U above, D below, L left, R right, B to the right of R
    // Using explicit offsets to ensure no overlap
    float yOffsetUp = -(faceSize + gap * 1.5f);
    float yOffsetDown = (faceSize + gap * 1.5f);
    float xOffsetLeft = -(faceSize + gap * 1.5f);
    float xOffsetRight = (faceSize + gap * 1.5f);
    float xOffsetBack = (faceSize + gap * 1.5f) * 2;

    // Draw in order: Up, Left, Front, Right, Down, Back (to minimize overlap issues)
    drawFace(drawList, cube_.getUp(),
            ImVec2(offset.x + 0, offset.y + yOffsetUp),
            stickerSize, gap);
    drawFace(drawList, cube_.getLeft(),
            ImVec2(offset.x + xOffsetLeft, offset.y + 0),
            stickerSize, gap);
    drawFace(drawList, cube_.getFront(),
            ImVec2(offset.x + 0, offset.y + 0),
            stickerSize, gap);
    drawFace(drawList, cube_.getRight(),
            ImVec2(offset.x + xOffsetRight, offset.y + 0),
            stickerSize, gap);
    drawFace(drawList, cube_.getDown(),
            ImVec2(offset.x + 0, offset.y + yOffsetDown),
            stickerSize, gap);
    drawFace(drawList, cube_.getBack(),
            ImVec2(offset.x + xOffsetBack, offset.y + 0),
            stickerSize, gap);
}

ImVec2 CubeRenderer::project(float x, float y, float z, ImVec2 center, float scale) {
    // Simple isometric-style projection
    float angleX = rotationX * M_PI / 180.0f;
    float angleY = rotationY * M_PI / 180.0f;

    // Rotate around X axis
    float y1 = y * cosf(angleX) - z * sinf(angleX);
    float z1 = y * sinf(angleX) + z * cosf(angleX);

    // Rotate around Y axis
    float x2 = x * cosf(angleY) + z1 * sinf(angleY);
    float z2 = -x * sinf(angleY) + z1 * cosf(angleY);

    return ImVec2(
        center.x + x2 * scale,
        center.y + y1 * scale
    );
}

void CubeRenderer::drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                          ImVec2 offset, float size, float gap) {
    float totalSize = size * 3.0f + gap * 2.0f;
    float startX = offset.x - totalSize / 2.0f + size / 2.0f;
    float startY = offset.y - totalSize / 2.0f + size / 2.0f;

    // Draw black background for the face
    ImVec2 p1(offset.x - totalSize / 2.0f, offset.y + totalSize / 2.0f);
    ImVec2 p2(offset.x + totalSize / 2.0f, offset.y + totalSize / 2.0f);
    ImVec2 p3(offset.x + totalSize / 2.0f, offset.y - totalSize / 2.0f);
    ImVec2 p4(offset.x - totalSize / 2.0f, offset.y - totalSize / 2.0f);
    drawList->AddQuadFilled(p1, p2, p3, p4, IM_COL32(25, 25, 25, 255));

    // Draw 3x3 grid of stickers
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int index = row * 3 + col;
            std::array<float, 3> rgb = colorToRgb(face[index]);
            ImU32 color = IM_COL32(
                static_cast<int>(rgb[0] * 255),
                static_cast<int>(rgb[1] * 255),
                static_cast<int>(rgb[2] * 255),
                255
            );
            float x = startX + static_cast<float>(col) * (size + gap);
            float y = startY - static_cast<float>(row) * (size + gap);

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
