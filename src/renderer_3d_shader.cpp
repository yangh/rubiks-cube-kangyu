#include "renderer_3d_shader.h"
#include "renderer.h"
#include "cubie.vert.glsl.h"
#include "cubie.frag.glsl.h"
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern "C" {
extern void glDrawArrays(GLenum mode, GLint first, GLsizei count);
extern void glDepthFunc(GLenum func);
}

#ifndef GL_DEPTH_BUFFER_BIT
#define GL_DEPTH_BUFFER_BIT 0x00000100
#endif
#ifndef GL_LESS
#define GL_LESS 0x0201
#endif

static const float kFaceNormal[6][3] = {
    { 0.0f,  0.0f,  1.0f },
    { 0.0f,  0.0f, -1.0f },
    { 0.0f,  1.0f,  0.0f },
    { 0.0f, -1.0f,  0.0f },
    { 1.0f,  0.0f,  0.0f },
    {-1.0f,  0.0f,  0.0f }
};

Renderer3DShader::Renderer3DShader() {
    buildShaders();
    std::cout << "Renderer3DShader initialized (OpenGL 3.3 + GLSL 330)" << std::endl;
}

Renderer3DShader::~Renderer3DShader() {
}

void Renderer3DShader::buildShaders() {
    shaderValid_ = false;
    if (!cubieShader_.compileShaderFromString(GL_VERTEX_SHADER, cubieVertShader)) {
        std::cerr << "Failed to compile cubie vertex shader" << std::endl;
        return;
    }
    if (!cubieShader_.compileShaderFromString(GL_FRAGMENT_SHADER, cubieFragShader)) {
        std::cerr << "Failed to compile cubie fragment shader" << std::endl;
        return;
    }
    if (!cubieShader_.linkProgram()) {
        std::cerr << "Failed to link cubie shader program" << std::endl;
        return;
    }
    shaderValid_ = true;
    cacheUniformLocations();
}

void Renderer3DShader::cacheUniformLocations() {
    loc_.view = cubieShader_.getLocation("view");
    loc_.projection = cubieShader_.getLocation("projection");
    loc_.cameraPos = cubieShader_.getLocation("cameraPos");
    loc_.lightPos[0] = cubieShader_.getLocation("lightPos[0]");
    loc_.lightPos[1] = cubieShader_.getLocation("lightPos[1]");
    loc_.lightColor = cubieShader_.getLocation("lightColor");
    loc_.gap = cubieShader_.getLocation("gap");
    loc_.cubieSize = cubieShader_.getLocation("cubieSize");
    loc_.resolution = cubieShader_.getLocation("resolution");
    loc_.animAngle = cubieShader_.getLocation("animAngle");
    loc_.animAxis = cubieShader_.getLocation("animAxis");

    char name[64];
    for (int i = 0; i < 27; i++) {
        snprintf(name, sizeof(name), "animSliceMask[%d]", i);
        loc_.animSliceMask[i] = cubieShader_.getLocation(name);

        snprintf(name, sizeof(name), "cubiePositions[%d]", i);
        loc_.cubiePositions[i] = cubieShader_.getLocation(name);
    }
    for (int i = 0; i < 54; i++) {
        snprintf(name, sizeof(name), "faceColors[%d]", i);
        loc_.faceColors[i] = cubieShader_.getLocation(name);
    }

    locationsCached_ = true;
}

void Renderer3DShader::setViewState(const ViewState* state) {
    viewState_ = state;
}

void Renderer3DShader::setColorProvider(const ColorProvider* provider) {
    colorProvider_ = provider;
}

void Renderer3DShader::setAnimator(const CubeAnimator* animator) {
    animator_ = animator;
}

void Renderer3DShader::setCube(const RubiksCube* cube) {
    cube_ = cube;
}

void Renderer3DShader::prepareUniforms(int viewW, int viewH) {
    if (!viewState_ || !colorProvider_ || !animator_ || !cube_ || !locationsCached_) {
        return;
    }

    float animAngle = animator_->getCurrentAngle();
    bool isAnimating = animator_->isAnimating();
    Move animMove = animator_->currentMove();
    RotationAxis animAxis = getRotationAxis(animMove);

    float cubiePositions[27 * 3];
    float stickerColors[54 * 3];

    for (int i = 0; i < 27; i++) {
        int layer = i / 9;
        int posInLayer = i % 9;
        int row = posInLayer / 3;
        int col = posInLayer % 3;

        float x = (col - 1.0f) * (kSpacingBase + gap_) * cubeScale_;
        float y = (row - 1.0f) * (kSpacingBase + gap_) * cubeScale_;
        float z = (layer - 1.0f) * (kSpacingBase + gap_) * cubeScale_;

        cubiePositions[i * 3] = x;
        cubiePositions[i * 3 + 1] = y;
        cubiePositions[i * 3 + 2] = z;
    }

    int cubeIndex = 0;
    for (int layer = 0; layer < 3; layer++) {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                bool shouldAnimate = isAnimating &&
                    MoveLookup::isInSlice(cubeIndex, getAnimationSlice(animMove));

                const RubiksCube& renderCube = shouldAnimate
                    ? animator_->getPreAnimationCube() : *cube_;

                struct StickerMapping {
                    int faceIdx;
                    int stickerOffset;
                    int localRow, localCol;
                };
                StickerMapping stickers[6] = {
                    { 0,  0, 2 - row, col },
                    { 1,  9, 2 - row, 2 - col },
                    { 2, 18, layer, col },
                    { 3, 27, 2 - layer, col },
                    { 4, 36, 2 - row, 2 - layer },
                    { 5, 45, 2 - row, layer }
                };

                bool isExterior[6] = {
                    layer == 2, layer == 0, row == 2, row == 0, col == 2, col == 0
                };

                for (int f = 0; f < 6; f++) {
                    if (!isExterior[f]) continue;
                    auto& sm = stickers[f];
                    const FaceColor& face = IRenderer3D::getCubeFace(renderCube, sm.faceIdx);
                    int idx = sm.localRow * 3 + sm.localCol;
                    auto rgb = colorProvider_->getFaceColorRgb(face[idx]);
                    int si = sm.stickerOffset + idx;
                    stickerColors[si * 3 + 0] = rgb.r;
                    stickerColors[si * 3 + 1] = rgb.g;
                    stickerColors[si * 3 + 2] = rgb.b;
                }

                cubeIndex++;
            }
        }
    }

    float aspect = (float)viewW / (float)viewH;
    float fov = 45.0f;
    float near = 0.1f;
    float far = 100.0f;

    glm::mat4 projMatrix = glm::perspective(glm::radians(fov), aspect, near, far);

    float rx = viewState_->rotationX * M_PI / 180.0f;
    float ry = viewState_->rotationY * M_PI / 180.0f;
    float rz = viewState_->rotationZ * M_PI / 180.0f;

    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), rx, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), ry, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), rz, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 6.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    viewMatrix = viewMatrix * rotX * rotY * rotZ;

    glm::vec3 actualCamPos = glm::vec3(glm::inverse(viewMatrix)[3]);

    cubieShader_.use();
    cubieShader_.setMat4("view", glm::value_ptr(viewMatrix));
    cubieShader_.setMat4("projection", glm::value_ptr(projMatrix));
    cubieShader_.setVec3At(loc_.cameraPos, actualCamPos.x, actualCamPos.y, actualCamPos.z);

    glm::vec3 camRight = glm::normalize(glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]));
    glm::vec3 camUp = glm::normalize(glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]));
    glm::vec3 camForward = glm::normalize(glm::vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]));
    float lightDist = 5.0f;
    glm::vec3 lp0 = actualCamPos + (camRight * 0.7f + camUp * 0.8f + camForward * 0.7f) * lightDist;
    glm::vec3 lp1 = actualCamPos + (-camRight * 0.7f + camUp * 0.8f + camForward * 0.7f) * lightDist;
    cubieShader_.setVec3At(loc_.lightPos[0], lp0.x, lp0.y, lp0.z);
    cubieShader_.setVec3At(loc_.lightPos[1], lp1.x, lp1.y, lp1.z);
    cubieShader_.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    cubieShader_.setFloatAt(loc_.gap, gap_);
    cubieShader_.setFloatAt(loc_.cubieSize, cubieSize_ * cubeScale_);
    cubieShader_.setVec2("resolution", (float)viewW, (float)viewH);
    cubieShader_.setFloatAt(loc_.animAngle, isAnimating ? -animAngle : 0.0f);
    cubieShader_.setVec3At(loc_.animAxis, animAxis.x, animAxis.y, animAxis.z);

    for (int i = 0; i < 27; i++) {
        float mask = 0.0f;
        if (isAnimating) {
            mask = MoveLookup::isInSlice(i, getAnimationSlice(animMove)) ? 1.0f : 0.0f;
        }
        cubieShader_.setFloatAt(loc_.animSliceMask[i], mask);
        cubieShader_.setVec3At(loc_.cubiePositions[i],
            cubiePositions[i*3], cubiePositions[i*3+1], cubiePositions[i*3+2]);
    }
    for (int i = 0; i < 54; i++) {
        cubieShader_.setVec3At(loc_.faceColors[i],
            stickerColors[i*3], stickerColors[i*3+1], stickerColors[i*3+2]);
    }
}

void Renderer3DShader::renderFullScreenQuad() {
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer3DShader::render(int windowWidth, int windowHeight, float sidebarWidth) {
    if (!shaderValid_ || !viewState_ || !colorProvider_ || !animator_ || !cube_) {
        return;
    }

    int viewX = 10;
    int viewY = 10;
    int viewW = windowWidth - (int)sidebarWidth - 20;
    int viewH = windowHeight - 20;

    glViewport(viewX, windowHeight - viewY - viewH, viewW, viewH);
    glEnable(GL_SCISSOR_TEST);
    glScissor(viewX, windowHeight - viewY - viewH, viewW, viewH);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    prepareUniforms(viewW, viewH);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnable(GL_DEPTH_TEST);
    cubieShader_.use();
    renderFullScreenQuad();

    glDisableVertexAttribArray(0);
    glDisable(GL_DEPTH_TEST);

    glViewport(0, 0, windowWidth, windowHeight);
}
