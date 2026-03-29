#ifndef RENDERER_3D_H
#define RENDERER_3D_H

#include "cube.h"

struct ViewState;
class ColorProvider;
class CubeAnimator;

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
};

#endif // RENDERER_3D_H
