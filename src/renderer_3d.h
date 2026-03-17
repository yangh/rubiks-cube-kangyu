#ifndef RENDERER_3D_H
#define RENDERER_3D_H

// Forward declarations
struct ViewState;
class ColorProvider;
class CubeAnimator;
class RubiksCube;

class IRenderer3D {
public:
    virtual ~IRenderer3D() = default;
    
    // Render 3D view (called after ImGui rendering)
    virtual void render(int windowWidth, int windowHeight) = 0;
    
    // Set dependencies (uses pointers, no ownership)
    virtual void setViewState(const ViewState* state) = 0;
    virtual void setColorProvider(const ColorProvider* provider) = 0;
    virtual void setAnimator(const CubeAnimator* animator) = 0;
    virtual void setCube(const RubiksCube* cube) = 0;
};

#endif // RENDERER_3D_H
