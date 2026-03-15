#include "renderer_3d_opengl.h"
#include "view_state.h"
#include "color_provider.h"
#include "cube_animator.h"
#include <cmath>
#include <iostream>

Renderer3DOpenGL::Renderer3DOpenGL() {
    if (initGL()) {
        std::cout << "OpenGL 3D rendering initialized successfully" << std::endl;
    }
}

Renderer3DOpenGL::~Renderer3DOpenGL() {
    delete cubeModel_;
    cubeModel_ = nullptr;
}

void Renderer3DOpenGL::setViewState(const ViewState* state) {
    viewState_ = state;
}

void Renderer3DOpenGL::setColorProvider(const ColorProvider* provider) {
    colorProvider_ = provider;
}

void Renderer3DOpenGL::setAnimator(const CubeAnimator* animator) {
    animator_ = animator;
}

void Renderer3DOpenGL::setCube(const RubiksCube* cube) {
    cube_ = cube;
}

bool Renderer3DOpenGL::initGL() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);

    cubeModel_ = new Model("src/models/cube.obj");

    if (cubeModel_ && !cubeModel_->meshes.empty()) {
        std::cout << "Model loaded: " << cubeModel_->meshes.size() << " mesh(es), "
                  << cubeModel_->meshes[0].vertices.size() << " vertices" << std::endl;
    } else {
        std::cout << "ERROR: Failed to load model" << std::endl;
    }

    return (cubeModel_ != nullptr);
}

void Renderer3DOpenGL::render(int windowWidth, int windowHeight) {
    if (!cubeModel_ || !viewState_ || !colorProvider_ || !animator_ || !cube_) {
        return;
    }

    float sidebarWidth = 480.0f;
    int viewX = 10;
    int viewY = 10;
    int viewWidth = windowWidth - (int)sidebarWidth - 20;
    int viewHeight = windowHeight - 20;

    glViewport(viewX, windowHeight - viewY - viewHeight, viewWidth, viewHeight);

    glEnable(GL_SCISSOR_TEST);
    glScissor(viewX, windowHeight - viewY - viewHeight, viewWidth, viewHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)viewWidth / viewHeight;
    float fov = 45.0f;
    float near = 0.1f;
    float far = 100.0f;
    float top = tanf(fov * M_PI / 360.0f) * near;
    float bottom = -top;
    float right = top * aspect;
    float left = -right;
    glFrustum(left, right, bottom, top, near, far);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);
    drawCircleCanvas();
    glPopMatrix();

    glRotatef(viewState_->rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(viewState_->rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(viewState_->rotationZ, 0.0f, 0.0f, 1.0f);

    glScalef(0.9f, 0.9f, 0.9f);
    glDisable(GL_CULL_FACE);
    glScalef(0.3f, 0.3f, 0.3f);

    float gap = 0.03f;
    int cubeIndex = 0;
    float animAngle = animator_->getCurrentAngle();

    for (int layer = 0; layer < 3; layer++) {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                bool cubeShouldAnimate = animator_->isAnimating() && animator_->isCubeInAnimatingSlice(cubeIndex);

                float xOffset = (col - 1.0f) * (1.0f + gap);
                float yOffset = (row - 1.0f) * (1.0f + gap);
                float zOffset = (layer - 1.0f) * (1.0f + gap);

                glPushMatrix();

                if (cubeShouldAnimate) {
                    applyRotationTransform(animAngle, animator_->currentMove());
                }

                glTranslatef(xOffset, yOffset, zOffset);
                drawCube(cubeIndex, cubeShouldAnimate);
                glPopMatrix();

                cubeIndex++;
            }
        }
    }

    glDisable(GL_LIGHTING);
    glViewport(0, 0, windowWidth, windowHeight);
}

void Renderer3DOpenGL::drawCircleCanvas() {
    float radius = 0.75f;
    float yOffset = -0.8f;
    int segments = 64;
    float r = 0.3f;
    float g = 0.35f;
    float b = 0.45f;
    float a = 0.3f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(r, g, b, a);
    glVertex3f(0.0f, yOffset, 0.0f);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        glVertex3f(x, yOffset, z);
    }
    glEnd();

    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glColor4f(r * 1.2f, g * 1.2f, b * 1.2f, a * 1.5f);
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        glVertex3f(x, yOffset, z);
    }
    glEnd();

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
}

void Renderer3DOpenGL::drawRoundedFace(float centerX, float centerY, float centerZ,
                                          float size, const float rgb[3],
                                          float nx, float ny, float nz, float cornerRadius) {
    float halfSize = size / 2.0f;
    int cornerSegments = 16;
    
    auto addVertex = [&](float dx, float dy) {
        if (nz != 0) {
            glVertex3f(centerX + dx, centerY + dy, centerZ);
        } else if (nx != 0) {
            glVertex3f(centerX, centerY + dy, centerZ + dx);
        } else {
            glVertex3f(centerX + dx, centerY, centerZ + dy);
        }
    };
    
    glColor3f(rgb[0], rgb[1], rgb[2]);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glBegin(GL_TRIANGLE_FAN);
    addVertex(0.0f, 0.0f);
    
    float inner = halfSize - cornerRadius;
    auto addCorner = [&](float cx, float cy, float startAngle) {
        for (int i = 0; i <= cornerSegments; i++) {
            float angle = startAngle + (M_PI / 2.0f) * i / cornerSegments;
            float dx = cx + cornerRadius * cosf(angle);
            float dy = cy + cornerRadius * sinf(angle);
            addVertex(dx, dy);
        }
    };
    
    addCorner(inner, -inner, -M_PI / 2.0f);
    addCorner(inner, inner, 0.0f);
    addCorner(-inner, inner, M_PI / 2.0f);
    addCorner(-inner, -inner, M_PI);
    addCorner(inner, -inner, -M_PI / 2.0f);
    glEnd();
    glDisable(GL_POLYGON_SMOOTH);
}

void Renderer3DOpenGL::drawSticker(float centerX, float centerY, float centerZ, 
                                     float size, const float rgb[3],
                                     float nx, float ny, float nz) {
    float cornerRadius = size * 0.10f;
    drawRoundedFace(centerX, centerY, centerZ, size, rgb, nx, ny, nz, cornerRadius);
}

void Renderer3DOpenGL::drawCube(int cubeIndex, bool usePreAnimationState) {
    float black[3] = {0.0f, 0.0f, 0.0f};
    float faceSize = 1.0f;
    float faceRadius = faceSize * 0.10f;

    drawRoundedFace(0.0f, 0.0f, 0.5f, faceSize, black, 0.0f, 0.0f, 1.0f, faceRadius);
    drawRoundedFace(0.0f, 0.0f, -0.5f, faceSize, black, 0.0f, 0.0f, -1.0f, faceRadius);
    drawRoundedFace(0.0f, 0.5f, 0.0f, faceSize, black, 0.0f, 1.0f, 0.0f, faceRadius);
    drawRoundedFace(0.0f, -0.5f, 0.0f, faceSize, black, 0.0f, -1.0f, 0.0f, faceRadius);
    drawRoundedFace(0.5f, 0.0f, 0.0f, faceSize, black, 1.0f, 0.0f, 0.0f, faceRadius);
    drawRoundedFace(-0.5f, 0.0f, 0.0f, faceSize, black, -1.0f, 0.0f, 0.0f, faceRadius);

    drawStickers(cubeIndex, usePreAnimationState);
}

void Renderer3DOpenGL::drawStickers(int cubeIndex, bool usePreAnimationState) {
    const RubiksCube& cube = usePreAnimationState ? animator_->getPreAnimationCube() : *cube_;

    int layer = cubeIndex / 9;
    int posInLayer = cubeIndex % 9;
    int row = posInLayer / 3;
    int col = posInLayer % 3;

    float stickerSize = 0.9f;
    float stickerOffset = 0.001f;

    if (layer == 2) {
        const auto& face = cube.getFront();
        int idx = (2 - row) * 3 + col;
        auto rgb = colorProvider_->getFaceColorRgb(face[idx]);
        drawSticker(0.0f, 0.0f, 0.5f + stickerOffset, stickerSize, rgb.data(), 0.0f, 0.0f, 1.0f);
    }

    if (layer == 0) {
        const auto& face = cube_->getBack();
        int idx = (2 - row) * 3 + (2 - col);
        auto rgb = colorProvider_->getFaceColorRgb(face[idx]);
        drawSticker(0.0f, 0.0f, -0.5f - stickerOffset, stickerSize, rgb.data(), 0.0f, 0.0f, -1.0f);
    }

    if (row == 2) {
        const auto& face = cube_->getUp();
        int idx = layer * 3 + col;
        auto rgb = colorProvider_->getFaceColorRgb(face[idx]);
        drawSticker(0.0f, 0.5f + stickerOffset, 0.0f, stickerSize, rgb.data(), 0.0f, 1.0f, 0.0f);
    }

    if (row == 0) {
        const auto& face = cube_->getDown();
        int idx = (2 - layer) * 3 + col;
        auto rgb = colorProvider_->getFaceColorRgb(face[idx]);
        drawSticker(0.0f, -0.5f - stickerOffset, 0.0f, stickerSize, rgb.data(), 0.0f, -1.0f, 0.0f);
    }

    if (col == 2) {
        const auto& face = cube_->getRight();
        int idx = (2 - row) * 3 + (2 - layer);
        auto rgb = colorProvider_->getFaceColorRgb(face[idx]);
        drawSticker(0.5f + stickerOffset, 0.0f, 0.0f, stickerSize, rgb.data(), 1.0f, 0.0f, 0.0f);
    }

    if (col == 0) {
        const auto& face = cube_->getLeft();
        int idx = (2 - row) * 3 + layer;
        auto rgb = colorProvider_->getFaceColorRgb(face[idx]);
        drawSticker(-0.5f - stickerOffset, 0.0f, 0.0f, stickerSize, rgb.data(), -1.0f, 0.0f, 0.0f);
    }
}

void Renderer3DOpenGL::applyRotationTransform(float angle, Move move) {
    switch (move) {
        case Move::U:
            glRotatef(angle, 0.0f, -1.0f, 0.0f);
            break;
        case Move::U2:
            glRotatef(angle, 0.0f, -1.0f, 0.0f);
            break;
        case Move::UP:
            glRotatef(angle, 0.0f, 1.0f, 0.0f);
            break;

        case Move::D:
            glRotatef(angle, 0.0f, 1.0f, 0.0f);
            break;
        case Move::D2:
            glRotatef(angle, 0.0f, 1.0f, 0.0f);
            break;
        case Move::DP:
            glRotatef(angle, 0.0f, -1.0f, 0.0f);
            break;

        case Move::L:
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
            break;
        case Move::L2:
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
            break;
        case Move::LP:
            glRotatef(angle, -1.0f, 0.0f, 0.0f);
            break;

        case Move::R:
            glRotatef(angle, -1.0f, 0.0f, 0.0f);
            break;
        case Move::R2:
            glRotatef(angle, -1.0f, 0.0f, 0.0f);
            break;
        case Move::RP:
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
            break;

        case Move::F:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;
        case Move::F2:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;
        case Move::FP:
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
            break;

        case Move::B:
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
            break;
        case Move::B2:
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
            break;
        case Move::BP:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;

        case Move::M:
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
            break;
        case Move::M2:
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
            break;
        case Move::MP:
            glRotatef(angle, -1.0f, 0.0f, 0.0f);
            break;

        case Move::E:
            glRotatef(-angle, 0.0f, -1.0f, 0.0f);
            break;
        case Move::E2:
            glRotatef(-angle, 0.0f, -1.0f, 0.0f);
            break;
        case Move::EP:
            glRotatef(-angle, 0.0f, 1.0f, 0.0f);
            break;

        case Move::S:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;
        case Move::S2:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;
        case Move::SP:
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
            break;

        case Move::X:
            glRotatef(angle, -1.0f, 0.0f, 0.0f);
            break;
        case Move::X2:
            glRotatef(angle, -1.0f, 0.0f, 0.0f);
            break;
        case Move::XP:
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
            break;

        case Move::Y:
            glRotatef(angle, 0.0f, -1.0f, 0.0f);
            break;
        case Move::Y2:
            glRotatef(angle, 0.0f, -1.0f, 0.0f);
            break;
        case Move::YP:
            glRotatef(angle, 0.0f, 1.0f, 0.0f);
            break;

        case Move::Z:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;
        case Move::Z2:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;
        case Move::ZP:
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
            break;

        default:
            break;
    }
}
