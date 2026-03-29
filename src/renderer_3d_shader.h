#ifndef RENDERER_3D_SHADER_H
#define RENDERER_3D_SHADER_H

#include "renderer_3d.h"
#include "cube.h"
#include "shader.h"
#include <glm/glm.hpp>

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

    Shader rastShader_;
    bool shaderValid_ = false;

    GLuint vao_ = 0;
    GLuint blackFaceVBO_ = 0;
    int blackFaceVertexCount_ = 0;
    GLuint stickerVBOs_[6] = {};
    int stickerVertexCounts_[6] = {};
    GLuint instanceMatVBO_ = 0;
    GLuint instanceColorVBO_ = 0;

    std::vector<StickerInfo> stickerInfos_[27];

    struct {
        GLint view = -1;
        GLint projection = -1;
        GLint cameraPos = -1;
        GLint lightPos[2] = {-1, -1};
        GLint lightColor = -1;
    } loc_;

    void buildShaders();
    void buildGeometry();
    void cacheUniformLocations();
    bool loadInstancedFunctions();
    void buildBlackFaces();
    void buildStickerTemplates();
};

#endif // RENDERER_3D_SHADER_H
