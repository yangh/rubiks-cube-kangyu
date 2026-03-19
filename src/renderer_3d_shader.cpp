#include "renderer_3d_shader.h"
#include "renderer.h"
#include "cubie.vert.glsl.h"
#include "cubie.frag.glsl.h"
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    if (!cubieShader_.compileShaderFromString(GL_VERTEX_SHADER, cubieVertShader)) {
        std::cerr << "Failed to compile cubie vertex shader" << std::endl;
    }
    if (!cubieShader_.compileShaderFromString(GL_FRAGMENT_SHADER, cubieFragShader)) {
        std::cerr << "Failed to compile cubie fragment shader" << std::endl;
    }
    if (!cubieShader_.linkProgram()) {
        std::cerr << "Failed to link cubie shader program" << std::endl;
    }
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

std::array<Color, 9> Renderer3DShader::getCubeFace(const RubiksCube& cube, int faceIdx) {
    switch (faceIdx) {
        case 0: return cube.getFront();
        case 1: return cube.getBack();
        case 2: return cube.getUp();
        case 3: return cube.getDown();
        case 4: return cube.getRight();
        case 5: return cube.getLeft();
        default: return cube.getFront();
    }
}

void Renderer3DShader::prepareUniforms(int viewW, int viewH) {
    if (!viewState_ || !colorProvider_ || !animator_ || !cube_) {
        return;
    }

    float animAngle = animator_->getCurrentAngle();
    bool isAnimating = animator_->isAnimating();
    Move animMove = animator_->currentMove();
    RotationAxis animAxis = getRotationAxis(animMove);

    float cubiePositions[27 * 3];
    float faceColors[27 * 6 * 3];

    for (int i = 0; i < 27; i++) {
        int layer = i / 9;
        int posInLayer = i % 9;
        int row = posInLayer / 3;
        int col = posInLayer % 3;

        float x = (col - 1.0f) * (0.9f + gap_) * cubeScale_;
        float y = (row - 1.0f) * (0.9f + gap_) * cubeScale_;
        float z = (layer - 1.0f) * (0.9f + gap_) * cubeScale_;

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

                struct FaceMapping {
                    int faceIdx;
                    int cubieRow, cubieCol;
                    int localRow, localCol;
                };
                FaceMapping faces[6] = {
                    { 0, layer, row, 2 - row, col },
                    { 1, layer, row, 2 - row, 2 - col },
                    { 2, row, layer, layer, col },
                    { 3, row, layer, 2 - layer, col },
                    { 4, row, col, 2 - row, 2 - layer },
                    { 5, row, col, 2 - row, layer }
                };

                for (int f = 0; f < 6; f++) {
                    auto& fm = faces[f];
                    bool onFace = false;
                    switch (fm.faceIdx) {
                        case 0: onFace = (layer == 2); break;
                        case 1: onFace = (layer == 0); break;
                        case 2: onFace = (row == 2); break;
                        case 3: onFace = (row == 0); break;
                        case 4: onFace = (col == 2); break;
                        case 5: onFace = (col == 0); break;
                    }

                    glm::vec3 color;
                    if (onFace) {
                        auto face = getCubeFace(renderCube, fm.faceIdx);
                        int idx = fm.localRow * 3 + fm.localCol;
                        auto rgb = colorProvider_->getFaceColorRgb(face[idx]);
                        color.x = rgb[0];
                        color.y = rgb[1];
                        color.z = rgb[2];
                    } else {
                        color = {0.02f, 0.02f, 0.02f};
                    }

                    int ci = cubeIndex;
                    int fi = f;
                    faceColors[(ci * 6 + fi) * 3 + 0] = color.x;
                    faceColors[(ci * 6 + fi) * 3 + 1] = color.y;
                    faceColors[(ci * 6 + fi) * 3 + 2] = color.z;
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
    cubieShader_.setVec3("cameraPos", actualCamPos.x, actualCamPos.y, actualCamPos.z);

    glm::vec3 camRight = glm::normalize(glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]));
    glm::vec3 camUp = glm::normalize(glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]));
    glm::vec3 camForward = glm::normalize(glm::vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]));
    float lightDist = 5.0f;
    glm::vec3 lp0 = actualCamPos + (camRight * 0.7f + camUp * 0.8f + camForward * 0.7f) * lightDist;
    glm::vec3 lp1 = actualCamPos + (-camRight * 0.7f + camUp * 0.8f + camForward * 0.7f) * lightDist;
    cubieShader_.setVec3("lightPos[0]", lp0.x, lp0.y, lp0.z);
    cubieShader_.setVec3("lightPos[1]", lp1.x, lp1.y, lp1.z);
    cubieShader_.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    cubieShader_.setFloat("gap", gap_);
    cubieShader_.setFloat("cubieSize", cubieSize_ * cubeScale_);
    cubieShader_.setVec2("resolution", (float)viewW, (float)viewH);
    cubieShader_.setFloat("animAngle", isAnimating ? -animAngle : 0.0f);
    cubieShader_.setVec3("animAxis", animAxis.x, animAxis.y, animAxis.z);

    int animSliceMask[27] = {};
    if (isAnimating) {
        AnimationSlice animSlice = getAnimationSlice(animMove);
        for (int i = 0; i < 27; i++) {
            animSliceMask[i] = MoveLookup::isInSlice(i, animSlice) ? 1 : 0;
        }
    }
    for (int i = 0; i < 27; i++) {
        char name[64];
        snprintf(name, sizeof(name), "animSliceMask[%d]", i);
        cubieShader_.setFloat(name, (float)animSliceMask[i]);
    }
    for (int i = 0; i < 27; i++) {
        char name[64];
        snprintf(name, sizeof(name), "cubiePositions[%d]", i);
        cubieShader_.setVec3(name, cubiePositions[i*3], cubiePositions[i*3+1], cubiePositions[i*3+2]);
    }
    for (int i = 0; i < 162; i++) {
        char name[64];
        snprintf(name, sizeof(name), "faceColors[%d]", i);
        cubieShader_.setVec3(name, faceColors[i*3], faceColors[i*3+1], faceColors[i*3+2]);
    }
}

void Renderer3DShader::renderFullScreenQuad() {
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer3DShader::render(int windowWidth, int windowHeight, float sidebarWidth) {
    if (!viewState_ || !colorProvider_ || !animator_ || !cube_) {
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

    GL_LOADER.glEnableVertexAttribArray(0);
    GL_LOADER.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnable(GL_DEPTH_TEST);
    cubieShader_.use();
    renderFullScreenQuad();

    GL_LOADER.glDisableVertexAttribArray(0);
    glDisable(GL_DEPTH_TEST);

    glViewport(0, 0, windowWidth, windowHeight);
}
