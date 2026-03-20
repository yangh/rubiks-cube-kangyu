#ifndef RENDERER_3D_SHADER_H
#define RENDERER_3D_SHADER_H

#include "renderer_3d.h"
#include "cube.h"
#include "shader.h"

class Renderer3DShader : public IRenderer3D {
public:
    Renderer3DShader();
    ~Renderer3DShader() override;

    void render(int windowWidth, int windowHeight, float sidebarWidth) override;
    void setViewState(const ViewState* state) override;
    void setColorProvider(const ColorProvider* provider) override;
    void setAnimator(const CubeAnimator* animator) override;
    void setCube(const RubiksCube* cube) override;
    void setScale(float scale) override { cubeScale_ = scale; }
    float getScale() const override { return cubeScale_; }
    void setGap(float gap) override { gap_ = gap; }
    float getGap() const override { return gap_; }

    float gap_ = 0.03f;
    float cubeScale_ = 0.55f;

private:
    const RubiksCube* cube_ = nullptr;
    const ViewState* viewState_ = nullptr;
    const ColorProvider* colorProvider_ = nullptr;
    const CubeAnimator* animator_ = nullptr;

    Shader cubieShader_;

    float cubieSize_ = 0.40f;  // half-size of a single cubie
    float cubieRadius_ = 0.04f;  // cubie corner radius

    void buildShaders();
    void prepareUniforms(int viewW, int viewH);
    void renderFullScreenQuad();

    static std::array<Color, 9> getCubeFace(const RubiksCube& cube, int faceIdx);
};

#endif // RENDERER_3D_SHADER_H
