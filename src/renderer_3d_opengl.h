#ifndef RENDERER_3D_OPENGL_H
#define RENDERER_3D_OPENGL_H

#include "irenderer_3d.h"
#include "model.h"
#include "cube.h"
#include <GL/gl.h>

class Renderer3DOpenGL : public IRenderer3D {
public:
    Renderer3DOpenGL();
    ~Renderer3DOpenGL() override;
    
    void render(int windowWidth, int windowHeight) override;
    void setViewState(const ViewState* state) override;
    void setColorProvider(const ColorProvider* provider) override;
    void setAnimator(const CubeAnimator* animator) override;
    void setCube(const RubiksCube* cube) override;
    
private:
    Model* cubeModel_ = nullptr;
    const RubiksCube* cube_ = nullptr;
    const ViewState* viewState_ = nullptr;
    const ColorProvider* colorProvider_ = nullptr;
    const CubeAnimator* animator_ = nullptr;
    
    bool initGL();
    void drawCube(int cubeIndex, bool usePreAnimationState);
    void drawCircleCanvas();
    void applyRotationTransform(float angle, Move move);
};

#endif // RENDERER_3D_OPENGL_H
