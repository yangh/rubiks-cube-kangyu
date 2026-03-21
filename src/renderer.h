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

    // Render 2D unfolded cube net view
    void draw2D(ImDrawList* drawList, ImVec2 offset, float scale);

    // Render 3D overlay (called after ImGui rendering)
    void render3DOverlay(int windowWidth, int windowHeight, float sidebarWidth);

    void executeMove(Move move);
    void executeMove(Move move, bool recordHistory);  // For animated undo/redo
    void updateAnimation(float deltaTime);
    void reset();

    // Apply color configuration to the renderer
    void setCustomColors(const ColorConfig& config);
    bool isAnimating() const { return animator_.isAnimating(); }
    float animationProgress() const { return animator_.progress(); }
    void resetView();  // Reset 3D view parameters to defaults

    void switchRenderer(int type);
    int getRendererType() const { return rendererType_; }
    void setCubeScale(float scale);
    void setGap(float gap);
    float getCubeScale() const;
    float getGap() const;

    ViewState viewState_;
    CubeAnimator animator_;

    ColorProvider colorProvider_;
    Renderer2D renderer2D_;
    int rendererType_ = 0;

    std::unique_ptr<IRenderer3D> renderer3D_;

private:
    RubiksCube& cube_;
};

#endif // RENDERER_H
