#ifndef RENDERER_3D_H
#define RENDERER_3D_H

#include "cube.h"
#include <cmath>
#include <vector>

struct ViewState;
class ColorProvider;
class CubeAnimator;

struct FaceDir {
    float offset;
    float nx, ny, nz;
};

// Face direction definitions: offset and normal for 6 faces
// 0: Front (+Z), 1: Back (-Z), 2: Up (+Y), 3: Down (-Y), 4: Right (+X), 5: Left (-X)
static constexpr FaceDir kFaceDirs[6] = {
    { 0.5f,  0.0f,  0.0f,  1.0f },   // Front (+Z)
    {-0.5f,  0.0f,  0.0f, -1.0f },   // Back (-Z)
    { 0.5f,  0.0f,  1.0f,  0.0f },   // Up (+Y)
    {-0.5f,  0.0f, -1.0f,  0.0f },   // Down (-Y)
    { 0.5f,  1.0f,  0.0f,  0.0f },   // Right (+X)
    {-0.5f, -1.0f,  0.0f,  0.0f }    // Left (-X)
};

class IRenderer3D {
public:
    virtual ~IRenderer3D() = default;

    // Shared coordinate system constants
    static constexpr float kCubieHalf = 0.40f;    // half-extent of one cubie
    static constexpr float kCubieFace = 0.80f;    // full face width (2 * kCubieHalf)
    static constexpr float kSpacingBase = 0.90f;  // base center-to-center spacing
    static constexpr float kCameraDist = 6.0f;    // camera distance from origin

    // Render 3D view (called after ImGui rendering)
    virtual void render(int windowWidth, int windowHeight, float sidebarWidth) = 0;

    // Set dependencies (uses pointers, no ownership)
    virtual void setViewState(const ViewState* state) = 0;
    virtual void setColorProvider(const ColorProvider* provider) = 0;
    virtual void setAnimator(const CubeAnimator* animator) = 0;
    virtual void setCube(const RubiksCube* cube) = 0;

    virtual void setScale(float scale) = 0;
    virtual float getScale() const = 0;
    virtual void setGap(float gap) = 0;
    virtual float getGap() const = 0;

    // Per-cubie sticker lookup: which faces are visible and how to index into them
    struct StickerInfo {
        int templateIdx;
        int faceIdx;
        int colorIdx;
    };

    static const FaceColor& getCubeFace(const RubiksCube& cube, int faceIdx) {
        switch (faceIdx) {
            case 0: return cube.getFront();
            case 1: return cube.getBack();
            case 2: return cube.getUp();
            case 3: return cube.getDown();
            case 4: return cube.getRight();
            case 5: return cube.getLeft();
            default: return cube.getFront();
        }
    }

    static std::vector<StickerInfo> buildStickerInfoForCube(int cubeIndex) {
        int layer = cubeIndex / 9;
        int posInLayer = cubeIndex % 9;
        int row = posInLayer / 3;
        int col = posInLayer % 3;
        std::vector<StickerInfo> infos;
        if (layer == 2) infos.push_back({0, 0, (2 - row) * 3 + col});
        if (layer == 0) infos.push_back({1, 1, (2 - row) * 3 + (2 - col)});
        if (row == 2)   infos.push_back({2, 2, layer * 3 + col});
        if (row == 0)   infos.push_back({3, 3, (2 - layer) * 3 + col});
        if (col == 2)   infos.push_back({4, 4, (2 - row) * 3 + (2 - layer)});
        if (col == 0)   infos.push_back({5, 5, (2 - row) * 3 + layer});
        return infos;
    }

    static std::vector<float> buildRoundedRect2D(float size, float cornerRadius) {
        float half = size / 2.0f;
        float inner = half - cornerRadius;
        int segments = 16;

        std::vector<float> fan;
        fan.push_back(0.0f);
        fan.push_back(0.0f);

        auto addCorner = [&](float cx, float cy, float startAngle) {
            for (int i = 0; i <= segments; i++) {
                float angle = startAngle + (M_PI / 2.0f) * i / segments;
                fan.push_back(cx + cornerRadius * cosf(angle));
                fan.push_back(cy + cornerRadius * sinf(angle));
            }
        };

        addCorner(inner, -inner, -M_PI / 2.0f);
        addCorner(inner,  inner,  0.0f);
        addCorner(-inner, inner,  M_PI / 2.0f);
        addCorner(-inner, -inner, M_PI);
        addCorner(inner,  -inner, -M_PI / 2.0f);

        return fan;
    }

    static std::vector<float> fanToTriangles(const std::vector<float>& fan2d) {
        std::vector<float> tris;
        tris.reserve((fan2d.size() / 2 - 1) * 9);
        for (size_t i = 1; i < fan2d.size() / 2 - 1; i++) {
            int idx = i * 2;
            tris.push_back(fan2d[0]);      tris.push_back(fan2d[1]);      tris.push_back(0.0f);
            tris.push_back(fan2d[idx]);     tris.push_back(fan2d[idx + 1]); tris.push_back(0.0f);
            tris.push_back(fan2d[idx + 2]); tris.push_back(fan2d[idx + 3]); tris.push_back(0.0f);
        }
        return tris;
    }

    static std::vector<float> transformFaceTo3D(const std::vector<float>& xyTris,
                                                  float offset, float, float ny, float nz) {
        std::vector<float> out;
        out.reserve(xyTris.size());
        for (size_t i = 0; i < xyTris.size(); i += 3) {
            float u = xyTris[i], v = xyTris[i + 1];
            if (nz != 0) {
                out.push_back(u); out.push_back(v); out.push_back(offset);
            } else if (ny != 0) {
                out.push_back(u); out.push_back(offset); out.push_back(v);
            } else {
                out.push_back(offset); out.push_back(v); out.push_back(u);
            }
        }
        return out;
    }

    static std::vector<float> addNormals(const std::vector<float>& posTris,
                                           float nx, float ny, float nz) {
        size_t vertCount = posTris.size() / 3;
        std::vector<float> result;
        result.reserve(vertCount * 6);
        for (size_t i = 0; i < vertCount; i++) {
            result.push_back(posTris[i * 3]);
            result.push_back(posTris[i * 3 + 1]);
            result.push_back(posTris[i * 3 + 2]);
            result.push_back(nx);
            result.push_back(ny);
            result.push_back(nz);
        }
        return result;
    }
};

#endif // RENDERER_3D_H
