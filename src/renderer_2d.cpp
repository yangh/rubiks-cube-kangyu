#include "renderer_2d.h"

Renderer2D::Renderer2D() {
}

void Renderer2D::draw(ImDrawList* drawList, ImVec2 offset, float scale,
                      const RubiksCube& cube, const ColorProvider& colors) {
    float stickerSize = 30.0f * scale;
    float gap = 3.0f * scale;
    float faceSize = stickerSize * 3.0f + gap * 2.0f;

    float spacing = faceSize + gap;
    offset.x -= spacing * 0.5;

    drawFace(drawList, cube.getUp(),
            ImVec2(offset.x, offset.y - spacing),
            stickerSize, gap, false, Color::WHITE, colors);

    drawFace(drawList, cube.getLeft(),
            ImVec2(offset.x - spacing, offset.y),
            stickerSize, gap, false, Color::ORANGE, colors);
    drawFace(drawList, cube.getFront(),
            ImVec2(offset.x, offset.y),
            stickerSize, gap, false, Color::GREEN, colors);
    drawFace(drawList, cube.getRight(),
            ImVec2(offset.x + spacing, offset.y),
            stickerSize, gap, false, Color::RED, colors);
    drawFace(drawList, cube.getBack(),
            ImVec2(offset.x + spacing * 2, offset.y),
            stickerSize, gap, false, Color::BLUE, colors);
    drawFace(drawList, cube.getDown(),
            ImVec2(offset.x, offset.y + spacing),
            stickerSize, gap, false, Color::YELLOW, colors);
}

void Renderer2D::drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                          ImVec2 offset, float size, float gap, bool flipVertical, Color faceType,
                          const ColorProvider& colors) {
    float totalSize = size * 3.0f + gap * 2.0f;
    float startX = offset.x - totalSize / 2.0f + size / 2.0f;
    float startY = offset.y - totalSize / 2.0f + size / 2.0f;

    ImVec2 p1(offset.x - totalSize / 2.0f, offset.y + totalSize / 2.0f);
    ImVec2 p2(offset.x + totalSize / 2.0f, offset.y + totalSize / 2.0f);
    ImVec2 p3(offset.x + totalSize / 2.0f, offset.y - totalSize / 2.0f);
    ImVec2 p4(offset.x - totalSize / 2.0f, offset.y - totalSize / 2.0f);
    drawList->AddQuadFilled(p1, p2, p3, p4, IM_COL32(25, 25, 25, 255));

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int index;
            if (faceType == Color::BLUE) {
                index = row * 3 + col;
            } else if (flipVertical) {
                index = (2 - row) * 3 + col;
            } else {
                index = row * 3 + col;
            }

            ImU32 color = colors.getFaceColor(face[index]);
            float x = startX + static_cast<float>(col) * (size + gap);
            float y = startY + static_cast<float>(row) * (size + gap);

            ImVec2 s1(x - size / 2.0f, y + size / 2.0f);
            ImVec2 s2(x + size / 2.0f, y + size / 2.0f);
            ImVec2 s3(x + size / 2.0f, y - size / 2.0f);
            ImVec2 s4(x - size / 2.0f, y - size / 2.0f);
            drawList->AddQuadFilled(s1, s2, s3, s4, color);

            ImU32 black = IM_COL32(0, 0, 0, 255);
            drawList->AddQuad(s1, s2, s3, s4, black, 2.0f);
        }
    }
}
