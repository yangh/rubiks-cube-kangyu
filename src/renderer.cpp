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
    cube_.executeMove(move);
    if (g_enableDump) {
        std::cout << "\n=== After " << moveToString(move) << " ===" << std::endl;
        cube_.dump();
    }
}

void CubeRenderer::reset() {
    cube_.reset();
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
    offset.y += spacing * 0.5;

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
        if (idx == 0) draw3DFace(drawList, cube_.getFront(), frontVerts, offset, size);
        else if (idx == 1) draw3DFace(drawList, cube_.getBack(), backVerts, offset, size);
        else if (idx == 2) draw3DFace(drawList, cube_.getLeft(), leftVerts, offset, size);
        else if (idx == 3) draw3DFace(drawList, cube_.getRight(), rightVerts, offset, size);
        else if (idx == 4) draw3DFace(drawList, cube_.getUp(), upVerts, offset, size);
        else if (idx == 5) draw3DFace(drawList, cube_.getDown(), downVerts, offset, size);
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
    // Each sticker is at a position in the face's local coordinate system
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int index = row * 3 + col;
            float u = (col - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67
            float v = (row - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67

            // Get face color
            ImU32 stickerColor = getFaceColor(face[index]);

            // Calculate sticker center position in 3D space
            float x = 0, y = 0, z = 0;

            if (face == cube_.getFront()) { z = 1; x = u; y = -v; }
            else if (face == cube_.getBack()) { z = -1; x = -u; y = -v; }
            else if (face == cube_.getLeft()) { x = -1; y = u; z = -v; }
            else if (face == cube_.getRight()) { x = 1; y = -u; z = -v; }
            else if (face == cube_.getUp()) { y = 1; x = u; z = -v; }
            else if (face == cube_.getDown()) { y = -1; x = u; z = v; }

            // Calculate sticker corners relative to sticker center
            float stickerSize = 0.6f;
            float halfSize = stickerSize / 2.0f;

            // Sticker corners in face-local coordinates (x, y offset from center)
            float cornersLocal[4][2] = {
                {-halfSize, halfSize},  // top-left
                {halfSize, halfSize},   // top-right
                {halfSize, -halfSize},  // bottom-right
                {-halfSize, -halfSize}  // bottom-left
            };

            // Project all 4 corners to screen space
            ImVec2 corners[4];
            for (int c = 0; c < 4; ++c) {
                float cx = 0, cy = 0, cz = 0;

                // Map local coordinates to 3D space based on face orientation
                if (face == cube_.getFront()) { cz = 1; cx = x + cornersLocal[c][0]; cy = y + cornersLocal[c][1]; }
                else if (face == cube_.getBack()) { cz = -1; cx = x - cornersLocal[c][0]; cy = y + cornersLocal[c][1]; }
                else if (face == cube_.getLeft()) { cx = -1; cy = y + cornersLocal[c][0]; cz = z + cornersLocal[c][1]; }
                else if (face == cube_.getRight()) { cx = 1; cy = y - cornersLocal[c][0]; cz = z + cornersLocal[c][1]; }
                else if (face == cube_.getUp()) { cy = 1; cx = x + cornersLocal[c][0]; cz = z + cornersLocal[c][1]; }
                else if (face == cube_.getDown()) { cy = -1; cx = x + cornersLocal[c][0]; cz = z - cornersLocal[c][1]; }

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

ImU32 CubeRenderer::getFaceColor(Color color) {
    std::array<float, 3> rgb = colorToRgb(color);
    return IM_COL32(
        static_cast<int>(rgb[0] * 255),
        static_cast<int>(rgb[1] * 255),
        static_cast<int>(rgb[2] * 255),
        255
    );
}
