#include "renderer_3d_shader.h"
#include "renderer.h"
#include "cubie_rast.vert.glsl.h"
#include "cubie_rast.frag.glsl.h"
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern "C" {
extern void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
extern void glDepthFunc(GLenum func);
extern void glVertexAttribDivisor(GLuint index, GLuint divisor);
}

#ifndef GL_DEPTH_BUFFER_BIT
#define GL_DEPTH_BUFFER_BIT 0x00000100
#endif
#ifndef GL_LESS
#define GL_LESS 0x0201
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif

static const struct { float offset; float nx; float ny; float nz; } kFaceDirs[6] = {
    { 0.5f,    0.0f,  0.0f,  1.0f },
    {-0.5f,    0.0f,  0.0f, -1.0f },
    { 0.5f,    0.0f,  1.0f,  0.0f },
    {-0.5f,    0.0f, -1.0f,  0.0f },
    { 0.5f,    1.0f,  0.0f,  0.0f },
    {-0.5f,   -1.0f,  0.0f,  0.0f }
};

Renderer3DShader::Renderer3DShader() {
    buildShaders();
    if (shaderValid_) {
        buildGeometry();
        cacheUniformLocations();
    }
    std::cout << "Renderer3DShader initialized (instanced rasterization, VBO + GLSL 330)" << std::endl;
}

Renderer3DShader::~Renderer3DShader() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (blackFaceVBO_) glDeleteBuffers(1, &blackFaceVBO_);
    for (int i = 0; i < 6; i++) {
        if (stickerVBOs_[i]) glDeleteBuffers(1, &stickerVBOs_[i]);
    }
    if (instanceVBO_) glDeleteBuffers(1, &instanceVBO_);
    if (colorVBO_) glDeleteBuffers(1, &colorVBO_);
}

void Renderer3DShader::buildShaders() {
    shaderValid_ = false;
    if (!rastShader_.compileShaderFromString(GL_VERTEX_SHADER, cubieRastVertShader)) {
        std::cerr << "Failed to compile rasterization vertex shader" << std::endl;
        return;
    }
    if (!rastShader_.compileShaderFromString(GL_FRAGMENT_SHADER, cubieRastFragShader)) {
        std::cerr << "Failed to compile rasterization fragment shader" << std::endl;
        return;
    }
    if (!rastShader_.linkProgram()) {
        std::cerr << "Failed to link rasterization shader program" << std::endl;
        return;
    }
    shaderValid_ = true;
}

void Renderer3DShader::cacheUniformLocations() {
    loc_.view = rastShader_.getLocation("uView");
    loc_.projection = rastShader_.getLocation("uProjection");
    loc_.cameraPos = rastShader_.getLocation("uCameraPos");
    loc_.lightPos[0] = rastShader_.getLocation("uLightPos[0]");
    loc_.lightPos[1] = rastShader_.getLocation("uLightPos[1]");
    loc_.lightColor = rastShader_.getLocation("uLightColor");
}

void Renderer3DShader::setViewState(const ViewState* state) { viewState_ = state; }
void Renderer3DShader::setColorProvider(const ColorProvider* provider) { colorProvider_ = provider; }
void Renderer3DShader::setAnimator(const CubeAnimator* animator) { animator_ = animator; }
void Renderer3DShader::setCube(const RubiksCube* cube) { cube_ = cube; }

std::vector<float> Renderer3DShader::buildRoundedRect2D(float size, float cornerRadius) {
    float half = size / 2.0f;
    float inner = half - cornerRadius;
    int segments = 16;

    std::vector<float> fan;
    fan.push_back(0.0f);
    fan.push_back(0.0f);

    auto addCorner = [&](float cx, float cy, float startAngle) {
        for (int i = 0; i <= segments; i++) {
            float angle = startAngle + (M_PI / 2.0f) * i / segments;
            fan.push_back(cx + cornerRadius * cosf(angle));
            fan.push_back(cy + cornerRadius * sinf(angle));
        }
    };

    addCorner(inner, -inner, -M_PI / 2.0f);
    addCorner(inner,  inner,  0.0f);
    addCorner(-inner, inner,  M_PI / 2.0f);
    addCorner(-inner, -inner, M_PI);
    addCorner(inner,  -inner, -M_PI / 2.0f);

    return fan;
}

std::vector<float> Renderer3DShader::fanToTriangles(const std::vector<float>& fan2d) {
    std::vector<float> tris;
    tris.reserve((fan2d.size() / 2 - 1) * 9);

    for (size_t i = 1; i < fan2d.size() / 2 - 1; i++) {
        int idx = i * 2;
        tris.push_back(fan2d[0]);      tris.push_back(fan2d[1]);      tris.push_back(0.0f);
        tris.push_back(fan2d[idx]);     tris.push_back(fan2d[idx + 1]); tris.push_back(0.0f);
        tris.push_back(fan2d[idx + 2]); tris.push_back(fan2d[idx + 3]); tris.push_back(0.0f);
    }

    return tris;
}

std::vector<float> Renderer3DShader::transformFaceTo3D(const std::vector<float>& xyTris,
                                                         float offset, float, float ny, float nz) {
    std::vector<float> out;
    out.reserve(xyTris.size());

    for (size_t i = 0; i < xyTris.size(); i += 3) {
        float u = xyTris[i];
        float v = xyTris[i + 1];

        if (nz != 0) {
            out.push_back(u); out.push_back(v); out.push_back(offset);
        } else if (ny != 0) {
            out.push_back(u); out.push_back(offset); out.push_back(v);
        } else {
            out.push_back(offset); out.push_back(v); out.push_back(u);
        }
    }

    return out;
}

std::vector<float> Renderer3DShader::addNormals(const std::vector<float>& posTris,
                                                  float nx, float ny, float nz) {
    size_t vertCount = posTris.size() / 3;
    std::vector<float> result;
    result.reserve(vertCount * 6);
    for (size_t i = 0; i < vertCount; i++) {
        result.push_back(posTris[i * 3]);
        result.push_back(posTris[i * 3 + 1]);
        result.push_back(posTris[i * 3 + 2]);
        result.push_back(nx);
        result.push_back(ny);
        result.push_back(nz);
    }
    return result;
}

void Renderer3DShader::buildBlackFaces() {
    float faceSize = 1.0f;
    float faceRadius = faceSize * 0.10f;
    auto face2d = buildRoundedRect2D(faceSize, faceRadius);
    auto faceTris = fanToTriangles(face2d);

    std::vector<float> allVerts;
    blackFaceVertexCount_ = 0;

    for (int f = 0; f < 6; f++) {
        auto transformed = transformFaceTo3D(faceTris,
            kFaceDirs[f].offset, kFaceDirs[f].nx, kFaceDirs[f].ny, kFaceDirs[f].nz);
        auto withNormals = addNormals(transformed, kFaceDirs[f].nx, kFaceDirs[f].ny, kFaceDirs[f].nz);
        allVerts.insert(allVerts.end(), withNormals.begin(), withNormals.end());
        blackFaceVertexCount_ += static_cast<int>(withNormals.size() / 6);
    }

    glBindBuffer(GL_ARRAY_BUFFER, blackFaceVBO_);
    glBufferData(GL_ARRAY_BUFFER, allVerts.size() * sizeof(float), allVerts.data(), GL_STATIC_DRAW);
}

void Renderer3DShader::buildStickerTemplates() {
    float stickerSize = 0.9f;
    float stickerRadius = stickerSize * 0.10f;
    float stickerOffsetExtra = 0.001f;

    auto sticker2d = buildRoundedRect2D(stickerSize, stickerRadius);
    auto stickerTris = fanToTriangles(sticker2d);

    for (int f = 0; f < 6; f++) {
        float offset = kFaceDirs[f].offset;
        float stickerOff = (offset > 0) ? offset + stickerOffsetExtra : offset - stickerOffsetExtra;

        auto transformed = transformFaceTo3D(stickerTris,
            stickerOff, kFaceDirs[f].nx, kFaceDirs[f].ny, kFaceDirs[f].nz);
        auto withNormals = addNormals(transformed, kFaceDirs[f].nx, kFaceDirs[f].ny, kFaceDirs[f].nz);

        stickerVertexCounts_[f] = static_cast<int>(withNormals.size() / 6);

        glBindBuffer(GL_ARRAY_BUFFER, stickerVBOs_[f]);
        glBufferData(GL_ARRAY_BUFFER, withNormals.size() * sizeof(float), withNormals.data(), GL_STATIC_DRAW);
    }
}

void Renderer3DShader::buildStickerInfo() {
    for (int cubeIndex = 0; cubeIndex < 27; cubeIndex++) {
        int layer = cubeIndex / 9;
        int posInLayer = cubeIndex % 9;
        int row = posInLayer / 3;
        int col = posInLayer % 3;

        stickerInfos_[cubeIndex].clear();

        if (layer == 2) {
            int idx = (2 - row) * 3 + col;
            stickerInfos_[cubeIndex].push_back({0, 0, idx});
        }
        if (layer == 0) {
            int idx = (2 - row) * 3 + (2 - col);
            stickerInfos_[cubeIndex].push_back({1, 1, idx});
        }
        if (row == 2) {
            int idx = layer * 3 + col;
            stickerInfos_[cubeIndex].push_back({2, 2, idx});
        }
        if (row == 0) {
            int idx = (2 - layer) * 3 + col;
            stickerInfos_[cubeIndex].push_back({3, 3, idx});
        }
        if (col == 2) {
            int idx = (2 - row) * 3 + (2 - layer);
            stickerInfos_[cubeIndex].push_back({4, 4, idx});
        }
        if (col == 0) {
            int idx = (2 - row) * 3 + layer;
            stickerInfos_[cubeIndex].push_back({5, 5, idx});
        }
    }
}

void Renderer3DShader::buildGeometry() {
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &blackFaceVBO_);
    for (int i = 0; i < 6; i++) {
        glGenBuffers(1, &stickerVBOs_[i]);
    }
    glGenBuffers(1, &instanceVBO_);
    glGenBuffers(1, &colorVBO_);

    buildBlackFaces();
    buildStickerTemplates();
    buildStickerInfo();

    glBindVertexArray(0);
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    float aspect = (float)viewW / (float)viewH;
    glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    float rx = viewState_->rotationX * M_PI / 180.0f;
    float ry = viewState_->rotationY * M_PI / 180.0f;
    float rz = viewState_->rotationZ * M_PI / 180.0f;

    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), rx, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), ry, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), rz, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f, kCameraDist),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    viewMatrix = viewMatrix * rotX * rotY * rotZ;

    glm::vec3 cameraPos = glm::vec3(glm::inverse(viewMatrix)[3]);

    glm::vec3 camRight   = glm::normalize(glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]));
    glm::vec3 camUp      = glm::normalize(glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]));
    glm::vec3 camForward = glm::normalize(glm::vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]));
    float lightDist = 5.0f;
    glm::vec3 lp0 = cameraPos + (camRight * 0.7f + camUp * 0.8f + camForward * 0.7f) * lightDist;
    glm::vec3 lp1 = cameraPos + (-camRight * 0.7f + camUp * 0.8f + camForward * 0.7f) * lightDist;

    float animAngle = animator_->getCurrentAngle();
    bool isAnimating = animator_->isAnimating();
    Move animMove = animator_->currentMove();
    RotationAxis animAxis = getRotationAxis(animMove);

    float spacing = (kCubieFace + gap_) * cubeScale_;
    float cubieSize = kCubieFace * cubeScale_;

    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(cubieSize));

    glm::mat4 models[27];
    const RubiksCube* renderCubes[27];
    for (int i = 0; i < 27; i++) {
        int layer = i / 9;
        int posInLayer = i % 9;
        int row = posInLayer / 3;
        int col = posInLayer % 3;

        float xOff = (col - 1.0f) * spacing;
        float yOff = (row - 1.0f) * spacing;
        float zOff = (layer - 1.0f) * spacing;

        bool shouldAnimate = isAnimating &&
            MoveLookup::isInSlice(i, getAnimationSlice(animMove));

        glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(xOff, yOff, zOff));
        if (shouldAnimate) {
            glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(animAngle),
                                       glm::vec3(animAxis.x, animAxis.y, animAxis.z));
            models[i] = R * T * S;
        } else {
            models[i] = T * S;
        }

        renderCubes[i] = shouldAnimate ? &animator_->getPreAnimationCube() : cube_;
    }

    float instanceMatrices[27 * 16];
    for (int i = 0; i < 27; i++) {
        memcpy(&instanceMatrices[i * 16], glm::value_ptr(models[i]), 16 * sizeof(float));
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(instanceMatrices), instanceMatrices, GL_DYNAMIC_DRAW);

    // Black faces: 27 instances, all black
    float blackColors[27 * 3];
    for (int i = 0; i < 27; i++) {
        blackColors[i * 3 + 0] = 0.02f;
        blackColors[i * 3 + 1] = 0.02f;
        blackColors[i * 3 + 2] = 0.02f;
    }
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(blackColors), blackColors, GL_DYNAMIC_DRAW);

    rastShader_.use();
    rastShader_.setMat4("uView", glm::value_ptr(viewMatrix));
    rastShader_.setMat4("uProjection", glm::value_ptr(projMatrix));
    rastShader_.setVec3At(loc_.cameraPos, cameraPos.x, cameraPos.y, cameraPos.z);
    rastShader_.setVec3At(loc_.lightPos[0], lp0.x, lp0.y, lp0.z);
    rastShader_.setVec3At(loc_.lightPos[1], lp1.x, lp1.y, lp1.z);
    rastShader_.setVec3("uLightColor", 1.0f, 1.0f, 1.0f);

    glBindVertexArray(vao_);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    for (int c = 0; c < 4; c++) {
        glEnableVertexAttribArray(2 + c);
    }
    glEnableVertexAttribArray(6);

    glBindBuffer(GL_ARRAY_BUFFER, blackFaceVBO_);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    for (int c = 0; c < 4; c++) {
        glVertexAttribPointer(2 + c, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float),
                              (void*)(c * 4 * sizeof(float)));
        glVertexAttribDivisor(2 + c, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO_);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(6, 1);

    glDrawArraysInstanced(GL_TRIANGLES, 0, blackFaceVertexCount_, 27);

    // Stickers: collect per-template instance data, then draw instanced per template
    struct StickerInstance {
        float matrix[16];
        float color[3];
    };

    std::vector<StickerInstance> templateInstances[6];
    for (int ci = 0; ci < 27; ci++) {
        for (const StickerInfo& si : stickerInfos_[ci]) {
            const FaceColor& face = IRenderer3D::getCubeFace(*renderCubes[ci], si.faceIdx);
            Color c = face[si.colorIdx];
            auto rgb = colorProvider_->getFaceColorRgb(c);

            StickerInstance inst;
            memcpy(inst.matrix, &instanceMatrices[ci * 16], 16 * sizeof(float));
            inst.color[0] = rgb.r;
            inst.color[1] = rgb.g;
            inst.color[2] = rgb.b;
            templateInstances[si.templateIdx].push_back(inst);
        }
    }

    for (int t = 0; t < 6; t++) {
        if (templateInstances[t].empty()) continue;

        int count = static_cast<int>(templateInstances[t].size());

        // Upload per-instance matrices
        float stickerMats[54 * 16];
        float stickerColors[54 * 3];
        for (int i = 0; i < count; i++) {
            memcpy(&stickerMats[i * 16], templateInstances[t][i].matrix, 16 * sizeof(float));
            stickerColors[i * 3 + 0] = templateInstances[t][i].color[0];
            stickerColors[i * 3 + 1] = templateInstances[t][i].color[1];
            stickerColors[i * 3 + 2] = templateInstances[t][i].color[2];
        }

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
        glBufferData(GL_ARRAY_BUFFER, count * 16 * sizeof(float), stickerMats, GL_DYNAMIC_DRAW);
        for (int c = 0; c < 4; c++) {
            glVertexAttribPointer(2 + c, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float),
                                  (void*)(c * 4 * sizeof(float)));
        }

        glBindBuffer(GL_ARRAY_BUFFER, colorVBO_);
        glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), stickerColors, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, stickerVBOs_[t]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        glDrawArraysInstanced(GL_TRIANGLES, 0, stickerVertexCounts_[t], count);
    }

    for (int c = 0; c < 4; c++) {
        glDisableVertexAttribArray(2 + c);
        glVertexAttribDivisor(2 + c, 0);
    }
    glDisableVertexAttribArray(6);
    glVertexAttribDivisor(6, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, windowWidth, windowHeight);
}
