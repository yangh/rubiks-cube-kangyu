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

void CubeRenderer::draw2D(ImDrawList* drawList, ImVec2 offset, float scale) {
    // Draw cube net (2D unfolded view)
    float stickerSize = 30.0f * scale;
    float gap = 3.0f * scale;
    float faceSize = stickerSize * 3.0f + gap * 2.0f;

    // Net layout: [Up] / [Left][Front][Right][Back] / [Down]
    // Face spacing
    float spacing = faceSize + gap;

    // Shift entire layout left by half a face width
    float xOffset = -0.5f * faceSize;

    // Draw in order: Up, Left, Front, Right, Down, Back
    drawFace(drawList, cube_.getUp(),
            ImVec2(offset.x + xOffset, offset.y - spacing),
            stickerSize, gap);
    drawFace(drawList, cube_.getLeft(),
            ImVec2(offset.x + xOffset - spacing, offset.y),
            stickerSize, gap);
    drawFace(drawList, cube_.getFront(),
            ImVec2(offset.x + xOffset, offset.y),
            stickerSize, gap);
    drawFace(drawList, cube_.getRight(),
            ImVec2(offset.x + xOffset + spacing, offset.y),
            stickerSize, gap);
    drawFace(drawList, cube_.getDown(),
            ImVec2(offset.x + xOffset, offset.y + spacing),
            stickerSize, gap);
    drawFace(drawList, cube_.getBack(),
            ImVec2(offset.x + xOffset + spacing * 2, offset.y),
            stickerSize, gap);
}

void CubeRenderer::draw3D(ImDrawList* drawList, ImVec2 offset, float scale) {
    float size = 40.0f * scale;

    // Define vertices for each face (before rotation)
    // Cube centered at origin, extending from -1 to +1
    struct FaceDef {
        const std::array<Color, 9>& face;
        const ImVec2 vertices[4];
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

    // Draw faces (order for simple depth sorting)
    // This is a simplified painter's algorithm - not perfect but works for this view
    draw3DFace(drawList, cube_.getBack(), backVerts, offset, size);
    draw3DFace(drawList, cube_.getDown(), downVerts, offset, size);
    draw3DFace(drawList, cube_.getLeft(), leftVerts, offset, size);
    draw3DFace(drawList, cube_.getRight(), rightVerts, offset, size);
    draw3DFace(drawList, cube_.getUp(), upVerts, offset, size);
    draw3DFace(drawList, cube_.getFront(), frontVerts, offset, size);
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
                          ImVec2 offset, float size, float gap) {
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

void CubeRenderer::draw3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                            const ImVec2 (&faceVerts)[4], ImVec2 center, float size) {
    // Project vertices to screen space
    ImVec2 projected[4];
    for (int i = 0; i < 4; ++i) {
        float x = 0, y = 0, z = 0;

        // Map face vertices to 3D positions
        if (face == cube_.getFront()) { z = 1; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (face == cube_.getBack()) { z = -1; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (face == cube_.getLeft()) { x = -1; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (face == cube_.getRight()) { x = 1; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (face == cube_.getUp()) { y = 1; x = faceVerts[i].x; z = faceVerts[i].y; }
        else if (face == cube_.getDown()) { y = -1; x = faceVerts[i].x; z = faceVerts[i].y; }

        projected[i] = project(x * 1.1f, y * 1.1f, z * 1.1f, center, size);
    }

    // Draw black background for face
    drawList->AddQuadFilled(projected[0], projected[1], projected[2], projected[3],
                          IM_COL32(20, 20, 20, 255));

    // Draw 3x3 stickers on the face
    // We need to interpolate positions for each sticker
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int index = row * 3 + col;
            float u = (col - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67
            float v = (row - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67

            // Get face color
            ImU32 stickerColor = getFaceColor(face[index]);

            // Calculate sticker positions by interpolation
            float x = 0, y = 0, z = 0;
            float sx = 0, sy = 0, sz = 0;

            if (face == cube_.getFront()) { z = 1; x = u; y = -v; sx = 0; sy = 0; sz = 0; }
            else if (face == cube_.getBack()) { z = -1; x = u; y = -v; sx = 0; sy = 0; sz = 0; }
            else if (face == cube_.getLeft()) { x = -1; y = u; z = v; sx = 0; sy = 0; sz = 0; }
            else if (face == cube_.getRight()) { x = 1; y = u; z = v; sx = 0; sy = 0; sz = 0; }
            else if (face == cube_.getUp()) { y = 1; x = u; z = -v; sx = 0; sy = 0; sz = 0; }
            else if (face == cube_.getDown()) { y = -1; x = u; z = -v; sx = 0; sy = 0; sz = 0; }

            // Calculate sticker corners
            float stickerSize = 0.6f;
            float halfSize = stickerSize / 2.0f;

            ImVec2 corners[4];
            corners[0] = project((x + sx) * 1.1f, (y + sy + halfSize) * 1.1f, (z + sz - halfSize) * 1.1f, center, size);
            corners[1] = project((x + sx + halfSize) * 1.1f, (y + sy + halfSize) * 1.1f, (z + sz) * 1.1f, center, size);
            corners[2] = project((x + sx + halfSize) * 1.1f, (y + sy - halfSize) * 1.1f, (z + sz) * 1.1f, center, size);
            corners[3] = project((x + sx) * 1.1f, (y + sy - halfSize) * 1.1f, (z + sz - halfSize) * 1.1f, center, size);

            // Draw sticker
            drawList->AddQuadFilled(corners[0], corners[1], corners[2], corners[3], stickerColor);
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
