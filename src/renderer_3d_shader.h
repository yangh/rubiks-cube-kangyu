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
    float cubeScale_ = 0.6f;

private:
    const RubiksCube* cube_ = nullptr;
    const ViewState* viewState_ = nullptr;
    const ColorProvider* colorProvider_ = nullptr;
    const CubeAnimator* animator_ = nullptr;

    Shader cubieShader_;
    bool shaderValid_ = false;
    bool locationsCached_ = false;

    float cubieSize_ = 0.40f;
    float cubieRadius_ = 0.04f;

    struct {
        GLint view = -1;
        GLint projection = -1;
        GLint cameraPos = -1;
        GLint lightPos[2] = {-1, -1};
        GLint lightColor = -1;
        GLint gap = -1;
        GLint cubieSize = -1;
        GLint resolution = -1;
        GLint animAngle = -1;
        GLint animAxis = -1;
        GLint animSliceMask[27] = {};
        GLint cubiePositions[27] = {};
        GLint faceColors[54] = {};
    } loc_;

    void buildShaders();
    void cacheUniformLocations();
    void prepareUniforms(int viewW, int viewH);
    void renderFullScreenQuad();
};

#endif // RENDERER_3D_SHADER_H
