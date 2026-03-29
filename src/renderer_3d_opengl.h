#ifndef RENDERER_3D_OPENGL_H
#define RENDERER_3D_OPENGL_H

#include "renderer_3d.h"
#include "cube.h"

class Renderer3DOpenGL : public IRenderer3D {
public:
    Renderer3DOpenGL();
    ~Renderer3DOpenGL() override;
    
    void render(int windowWidth, int windowHeight, float sidebarWidth) override;
    void setViewState(const ViewState* state) override;
    void setColorProvider(const ColorProvider* provider) override;
    void setAnimator(const CubeAnimator* animator) override;
    void setCube(const RubiksCube* cube) override;
    void setScale(float scale) override { cubeScale_ = scale; }
    float getScale() const override { return cubeScale_; }
    void setGap(float gap) override { gap_ = gap; }
    float getGap() const override { return gap_; }

    float cubeScale_ = 0.6f;
    float gap_ = 0.03f;
    
private:
    const RubiksCube* cube_ = nullptr;
    const ViewState* viewState_ = nullptr;
    const ColorProvider* colorProvider_ = nullptr;
    const CubeAnimator* animator_ = nullptr;

    struct FaceGeometry {
        std::vector<float> vertices;
        int vertexCount;
    };

    FaceGeometry cubeBlackFaceGeom_;
    FaceGeometry stickerTemplates_[6];
    FaceGeometry circleFillGeom_;
    FaceGeometry circleLineGeom_;

    std::vector<StickerInfo> stickerInfos_[27];

    void buildGeometry();
    void buildCubeBlackFaces();
    void buildStickerTemplates();
    void buildCircleCanvas();
    void renderCircleCanvas();
};

#endif // RENDERER_3D_OPENGL_H
