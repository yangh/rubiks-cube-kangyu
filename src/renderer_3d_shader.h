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

    Shader rastShader_;
    bool shaderValid_ = false;

    float cubieSize_ = 0.40f;

    GLuint vao_ = 0;
    GLuint blackFaceVBO_ = 0;
    int blackFaceVertexCount_ = 0;
    GLuint stickerVBOs_[6] = {};
    int stickerVertexCounts_[6] = {};
    GLuint instanceVBO_ = 0;
    GLuint colorVBO_ = 0;

    struct StickerInfo {
        int templateIdx;
        int faceIdx;
        int colorIdx;
    };
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

    static std::vector<float> buildRoundedRect2D(float size, float cornerRadius);
    static std::vector<float> fanToTriangles(const std::vector<float>& fan2d);
    static std::vector<float> transformFaceTo3D(const std::vector<float>& xyTris,
                                                 float offset, float nx, float ny, float nz);
    static std::vector<float> addNormals(const std::vector<float>& posTris,
                                          float nx, float ny, float nz);
    void buildBlackFaces();
    void buildStickerTemplates();
    void buildStickerInfo();
};

#endif // RENDERER_3D_SHADER_H
