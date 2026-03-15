#ifndef IRENDERER_3D_H
#define IRENDERER_3D_H

// Forward declarations
struct ViewState;
class ColorProvider;
class CubeAnimator;
class RubiksCube;

class IRenderer3D {
public:
    virtual ~IRenderer3D() = default;
    
    // 渲染 3D 视图（在 ImGui 渲染后调用）
    virtual void render(int windowWidth, int windowHeight) = 0;
    
    // 设置依赖项（使用指针，不拥有所有权）
    virtual void setViewState(const ViewState* state) = 0;
    virtual void setColorProvider(const ColorProvider* provider) = 0;
    virtual void setAnimator(const CubeAnimator* animator) = 0;
    virtual void setCube(const RubiksCube* cube) = 0;
};

#endif // IRENDERER_3D_H
