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
    
private:
    const RubiksCube* cube_ = nullptr;
    const ViewState* viewState_ = nullptr;
    const ColorProvider* colorProvider_ = nullptr;
    const CubeAnimator* animator_ = nullptr;
    
    // Pre-computed geometry (eliminates per-frame trig and glBegin/glEnd overhead)
    struct FaceGeometry {
        std::vector<float> vertices;  // interleaved x,y,z (3 floats per vertex)
        int vertexCount;
    };
    
    FaceGeometry cubeBlackFaceGeom_;  // One cube's 6 black faces
    FaceGeometry stickerTemplates_[6];    // 6 face-direction sticker templates
    FaceGeometry circleFillGeom_;       // Circle canvas fill triangles
    FaceGeometry circleLineGeom_;       // Circle canvas line loop vertices
    
    // Pre-computed sticker info per cube (FIX #3: consistent data source)
    struct StickerInfo {
        int templateIdx;  // 0-5: which face-direction template to use
        int faceIdx;      // 0=FRONT, 1=BACK, 2=UP, 3=DOWN, 4=RIGHT, 5=LEFT
        int colorIdx;     // Index into the 9-element face color array (0-8)
    };
    std::vector<StickerInfo> stickerInfos_[27];
    
    // Geometry pre-computation (called once at construction)
    void buildGeometry();
    static std::vector<float> buildRoundedRect2D(float size, float cornerRadius);
    static std::vector<float> fanToTriangles(const std::vector<float>& fan2d);
    static std::vector<float> transformFaceTo3D(const std::vector<float>& xyTris,
                                                 float offset, float nx, float ny, float nz);
    void buildCubeBlackFaces();
    void buildStickerTemplates();
    void buildStickerInfo();
    void buildCircleCanvas();
    
    // Render helpers
    void renderCircleCanvas();
    static std::array<Color, 9> getCubeFace(const RubiksCube& cube, int faceIdx);
};

#endif // RENDERER_3D_OPENGL_H
