#ifndef RENDERER_H
#define RENDERER_H

#include "cube.h"
#include "animator.h"
#include "config.h"
#include "renderer_2d.h"
#include "renderer_3d.h"
#include <imgui.h>
#include <memory>

struct ViewState {
    float rotationX = 30.0f;
    float rotationY = -30.0f;
    float rotationZ = 0.0f;
    float targetRotationX = 30.0f;
    float targetRotationY = -30.0f;
    float targetRotationZ = 0.0f;
    float viewRotationSpeed = 8.0f;
    
    float scale3D = 3.1f;
    float scale2D = 0.8f;
    
    bool celebrationMode = false;
    
    void lerpRotation(float& current, float target, float deltaTime);
    void reset();
};

class CubeRenderer {
public:
    explicit CubeRenderer(RubiksCube& cube);
    ~CubeRenderer() = default;

    void draw2D(ImDrawList* drawList, ImVec2 offset, float scale);
    void render3DOverlay(int windowWidth, int windowHeight, float sidebarWidth);

    void executeMove(Move move);
    void executeMove(Move move, bool recordHistory);
    void undoMove();
    void redoMove();
    void updateAnimation(float deltaTime);
    void reset();

    void setCustomConfig(const CubeConfig& config);
    bool isAnimating() const;
    float animationProgress() const;
    void resetView();

    void switchRenderer(RendererType type);
    RendererType getRendererType() const { return rendererType_; }
    void setCubeScale(float scale);
    void setGap(float gap);
    float getCubeScale() const;
    float getGap() const;

    ViewState& viewState() { return viewState_; }
    const ViewState& viewState() const { return viewState_; }

    ColorProvider& colorProvider() { return colorProvider_; }
    const ColorProvider& colorProvider() const { return colorProvider_; }

    CubeAnimator& animator() { return animator_; }
    const CubeAnimator& animator() const { return animator_; }

    Renderer2D& renderer2D() { return renderer2D_; }

private:
    RubiksCube& cube_;
    CubeAnimator animator_;
    ViewState viewState_;
    ColorProvider colorProvider_;
    Renderer2D renderer2D_;
    RendererType rendererType_ = RendererType::OpenGL;
    std::unique_ptr<IRenderer3D> renderer3D_;
};

#endif // RENDERER_H
