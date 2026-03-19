#include "renderer_3d_shader.h"
#include "renderer.h"
#include "color.h"
#include "animator.h"
#include "move.h"
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const char* cubieVertShader = R"(
#version 330 core

out vec3 vWorldPos;

uniform mat4 view;
uniform mat4 projection;

const vec2 quadVertices[6] = vec2[6](
    vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(1.0, 1.0),
    vec2(-1.0, -1.0), vec2(1.0, 1.0), vec2(-1.0, 1.0)
);

void main() {
    vec2 pos = quadVertices[gl_VertexID];
    
    vec4 clipPos = vec4(pos, -1.0, 1.0);
    vec4 viewPos = inverse(projection) * clipPos;
    viewPos = viewPos / viewPos.w;
    
    vWorldPos = (inverse(view) * vec4(viewPos.xyz, 1.0)).xyz;
    
    gl_Position = clipPos;
}
)";

static const char* cubieFragShader = R"(
#version 330 core

in vec3 vWorldPos;
out vec4 fragColor;

uniform vec3 cameraPos;
uniform vec3 cubiePositions[27];
uniform float cubieSize;
uniform float animAngle;
uniform vec3 animAxis;
uniform float gap;
uniform float animSliceMask[27];
uniform vec3 lightPos;
uniform vec3 lightColor;

float sdRoundBox(vec3 p, vec3 b, float r) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}

mat3 rotateAroundAxis(vec3 axis, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;
    vec3 a = normalize(axis);
    return mat3(
        t * a.x * a.x + c,       t * a.x * a.y - s * a.z, t * a.x * a.z + s * a.y,
        t * a.x * a.y + s * a.z, t * a.y * a.y + c,       t * a.y * a.z - s * a.x,
        t * a.x * a.z - s * a.y, t * a.y * a.z + s * a.x, t * a.z * a.z + c
    );
}

float sceneSDF(vec3 p, out int cubieIndex) {
    float minDist = 1e10;
    cubieIndex = -1;

    mat3 animRot;
    bool hasAnim = (animAngle != 0.0);
    if (hasAnim) {
        animRot = rotateAroundAxis(animAxis, radians(animAngle));
    }

    for (int i = 0; i < 27; i++) {
        vec3 pos = cubiePositions[i];

        if (hasAnim && animSliceMask[i] > 0.5) {
            pos = animRot * pos;
        }

        vec3 localP = p - pos;

        if (hasAnim && animSliceMask[i] > 0.5) {
            localP = transpose(animRot) * localP;
        }

        float d = sdRoundBox(localP, vec3(cubieSize), cubieSize * 0.1);

        if (d < minDist) {
            minDist = d;
            cubieIndex = i;
        }
    }
    return minDist;
}

vec3 calcNormal(vec3 p) {
    const float eps = 0.001;
    int dummy;
    vec3 n;
    n.x = sceneSDF(p + vec3(eps,0,0), dummy) - sceneSDF(p - vec3(eps,0,0), dummy);
    n.y = sceneSDF(p + vec3(0,eps,0), dummy) - sceneSDF(p - vec3(0,eps,0), dummy);
    n.z = sceneSDF(p + vec3(0,0,eps), dummy) - sceneSDF(p - vec3(0,0,eps), dummy);
    return normalize(n);
}

void main() {
    vec3 ro = cameraPos;
    vec3 rd = normalize(vWorldPos - cameraPos);

    float t = 0.0;
    int cubieIdx = -1;
    bool hit = false;

    for (int i = 0; i < 64; i++) {
        vec3 p = ro + t * rd;
        float d = sceneSDF(p, cubieIdx);
        if (d < 0.001) { hit = true; break; }
        if (t > 20.0) break;
        t += max(d, 0.001);
    }

    if (!hit) { discard; }

    vec3 p = ro + t * rd;
    vec3 n = calcNormal(p);
    vec3 lightDir = normalize(lightPos - p);
    vec3 viewDir = normalize(cameraPos - p);
    vec3 reflectDir = reflect(-lightDir, n);

    float ambient = 0.25;
    float diff = max(dot(n, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 cubieColor = vec3(0.02, 0.02, 0.02);
    vec3 color = cubieColor * (ambient + diff) + lightColor * spec * 0.4;

    fragColor = vec4(color, 1.0);
}
)";

static const char* stickerVertShader = R"(
#version 330 core

out vec2 vUV;
out vec3 vRayDir;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

const vec2 quadVertices[6] = vec2[6](
    vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(1.0, 1.0),
    vec2(-1.0, -1.0), vec2(1.0, 1.0), vec2(-1.0, 1.0)
);

void main() {
    vUV = quadVertices[gl_VertexID] * 0.5 + 0.5;

    vec4 rayTarget = inverse(projection) * vec4(quadVertices[gl_VertexID], 1.0, 1.0);
    rayTarget.w = 0.0;
    vRayDir = mat3(view) * rayTarget.xyz;

    gl_Position = vec4(quadVertices[gl_VertexID], -1.0, 1.0);
}
)";

static const char* stickerFragShader = R"(
#version 330 core

#define MAX_STICKERS 54

in vec3 vRayDir;
out vec4 fragColor;

uniform vec3 cameraPos;
uniform vec3 stickerCenters[MAX_STICKERS];
uniform vec3 stickerNormals[MAX_STICKERS];
uniform vec3 stickerColors[MAX_STICKERS];
uniform int stickerCount;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float animAngle;
uniform vec3 animAxis;

float sdRoundRect(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

mat3 rotateAroundAxis(vec3 axis, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;
    vec3 a = normalize(axis);
    return mat3(
        t * a.x * a.x + c,       t * a.x * a.y - s * a.z, t * a.x * a.z + s * a.y,
        t * a.x * a.y + s * a.z, t * a.y * a.y + c,       t * a.y * a.z - s * a.x,
        t * a.x * a.z - s * a.y, t * a.y * a.z + s * a.x, t * a.z * a.z + c
    );
}

bool rayIntersectPlane(vec3 ro, vec3 rd, vec3 center, vec3 normal, out float t) {
    float denom = dot(rd, normal);
    if (abs(denom) < 1e-6) return false;
    t = dot(center - ro, normal) / denom;
    return t > 0.0;
}

void main() {
    vec3 ro = cameraPos;
    vec3 rd = normalize(vRayDir);

    float minT = 1e10;
    vec3 hitColor = vec3(0.0);
    vec3 hitNormal = vec3(0.0);

    mat3 animRot;
    bool hasAnim = (animAngle != 0.0);
    if (hasAnim) {
        animRot = rotateAroundAxis(animAxis, radians(animAngle));
    }

    for (int i = 0; i < stickerCount; i++) {
        vec3 center = stickerCenters[i];
        vec3 normal = stickerNormals[i];

        if (hasAnim) {
            center = animRot * center;
            normal = animRot * normal;
        }

        float t;
        if (!rayIntersectPlane(ro, rd, center, normal, t)) continue;
        if (t > minT) continue;

        vec3 p = ro + t * rd;
        vec3 localP = p - center;

        vec3 u = normalize(cross(normal, vec3(0.0, 1.0, 0.0)));
        if (length(u) < 0.001) u = normalize(cross(normal, vec3(1.0, 0.0, 0.0)));
        vec3 v = cross(normal, u);

        float uCoord = dot(localP, u);
        float vCoord = dot(localP, v);

        float d = sdRoundRect(vec2(uCoord, vCoord), vec2(0.13), 0.015);
        if (d < 0.0) {
            minT = t;
            hitColor = stickerColors[i];
            hitNormal = normal;
        }
    }

    if (minT < 1e10) {
        vec3 p = ro + minT * rd;
        vec3 lightDir = normalize(lightPos - p);
        vec3 viewDir = normalize(cameraPos - p);
        vec3 reflectDir = reflect(-lightDir, hitNormal);

        float ambient = 0.25;
        float diff = max(dot(hitNormal, lightDir), 0.0);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

        vec3 color = hitColor * (ambient + diff) + lightColor * spec * 0.4;
        fragColor = vec4(color, 1.0);
    } else {
        discard;
    }
}
)";

static const float kFaceOffset[6] = { 0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f };
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

    if (!stickerShader_.compileShaderFromString(GL_VERTEX_SHADER, stickerVertShader)) {
        std::cerr << "Failed to compile sticker vertex shader" << std::endl;
    }
    if (!stickerShader_.compileShaderFromString(GL_FRAGMENT_SHADER, stickerFragShader)) {
        std::cerr << "Failed to compile sticker fragment shader" << std::endl;
    }
    if (!stickerShader_.linkProgram()) {
        std::cerr << "Failed to link sticker shader program" << std::endl;
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

    stickerData_.clear();
    int cubeIndex = 0;
    for (int layer = 0; layer < 3; layer++) {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                bool shouldAnimate = isAnimating &&
                    MoveLookup::isInSlice(cubeIndex, getAnimationSlice(animMove));

                const RubiksCube& renderCube = shouldAnimate
                    ? animator_->getPreAnimationCube() : *cube_;

                if (layer == 2) {
                    int idx = (2 - row) * 3 + col;
                    auto face = getCubeFace(renderCube, 0);
                    Color c = face[idx];
                    auto rgb = colorProvider_->getFaceColorRgb(c);
                    StickerData sd;
                    sd.center[0] = kFaceOffset[0];
                    sd.center[1] = (row - 1.0f) * (1.0f + gap_);
                    sd.center[2] = (layer - 1.0f) * (1.0f + gap_);
                    sd.normal[0] = kFaceNormal[0][0];
                    sd.normal[1] = kFaceNormal[0][1];
                    sd.normal[2] = kFaceNormal[0][2];
                    sd.color[0] = rgb[0];
                    sd.color[1] = rgb[1];
                    sd.color[2] = rgb[2];
                    stickerData_.push_back(sd);
                }
                if (layer == 0) {
                    int idx = (2 - row) * 3 + (2 - col);
                    auto face = getCubeFace(renderCube, 1);
                    Color c = face[idx];
                    auto rgb = colorProvider_->getFaceColorRgb(c);
                    StickerData sd;
                    sd.center[0] = kFaceOffset[1];
                    sd.center[1] = (row - 1.0f) * (1.0f + gap_);
                    sd.center[2] = (layer - 1.0f) * (1.0f + gap_);
                    sd.normal[0] = kFaceNormal[1][0];
                    sd.normal[1] = kFaceNormal[1][1];
                    sd.normal[2] = kFaceNormal[1][2];
                    sd.color[0] = rgb[0];
                    sd.color[1] = rgb[1];
                    sd.color[2] = rgb[2];
                    stickerData_.push_back(sd);
                }
                if (row == 2) {
                    int idx = layer * 3 + col;
                    auto face = getCubeFace(renderCube, 2);
                    Color c = face[idx];
                    auto rgb = colorProvider_->getFaceColorRgb(c);
                    StickerData sd;
                    sd.center[0] = (col - 1.0f) * (1.0f + gap_);
                    sd.center[1] = kFaceOffset[2];
                    sd.center[2] = (layer - 1.0f) * (1.0f + gap_);
                    sd.normal[0] = kFaceNormal[2][0];
                    sd.normal[1] = kFaceNormal[2][1];
                    sd.normal[2] = kFaceNormal[2][2];
                    sd.color[0] = rgb[0];
                    sd.color[1] = rgb[1];
                    sd.color[2] = rgb[2];
                    stickerData_.push_back(sd);
                }
                if (row == 0) {
                    int idx = (2 - layer) * 3 + col;
                    auto face = getCubeFace(renderCube, 3);
                    Color c = face[idx];
                    auto rgb = colorProvider_->getFaceColorRgb(c);
                    StickerData sd;
                    sd.center[0] = (col - 1.0f) * (1.0f + gap_);
                    sd.center[1] = kFaceOffset[3];
                    sd.center[2] = (layer - 1.0f) * (1.0f + gap_);
                    sd.normal[0] = kFaceNormal[3][0];
                    sd.normal[1] = kFaceNormal[3][1];
                    sd.normal[2] = kFaceNormal[3][2];
                    sd.color[0] = rgb[0];
                    sd.color[1] = rgb[1];
                    sd.color[2] = rgb[2];
                    stickerData_.push_back(sd);
                }
                if (col == 2) {
                    int idx = (2 - row) * 3 + (2 - layer);
                    auto face = getCubeFace(renderCube, 4);
                    Color c = face[idx];
                    auto rgb = colorProvider_->getFaceColorRgb(c);
                    StickerData sd;
                    sd.center[0] = kFaceOffset[4];
                    sd.center[1] = (row - 1.0f) * (1.0f + gap_);
                    sd.center[2] = (layer - 1.0f) * (1.0f + gap_);
                    sd.normal[0] = kFaceNormal[4][0];
                    sd.normal[1] = kFaceNormal[4][1];
                    sd.normal[2] = kFaceNormal[4][2];
                    sd.color[0] = rgb[0];
                    sd.color[1] = rgb[1];
                    sd.color[2] = rgb[2];
                    stickerData_.push_back(sd);
                }
                if (col == 0) {
                    int idx = (2 - row) * 3 + layer;
                    auto face = getCubeFace(renderCube, 5);
                    Color c = face[idx];
                    auto rgb = colorProvider_->getFaceColorRgb(c);
                    StickerData sd;
                    sd.center[0] = kFaceOffset[5];
                    sd.center[1] = (row - 1.0f) * (1.0f + gap_);
                    sd.center[2] = (layer - 1.0f) * (1.0f + gap_);
                    sd.normal[0] = kFaceNormal[5][0];
                    sd.normal[1] = kFaceNormal[5][1];
                    sd.normal[2] = kFaceNormal[5][2];
                    sd.color[0] = rgb[0];
                    sd.color[1] = rgb[1];
                    sd.color[2] = rgb[2];
                    stickerData_.push_back(sd);
                }
                cubeIndex++;
            }
        }
    }
    stickerCount_ = static_cast<int>(stickerData_.size());

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
    cubieShader_.setVec3("lightPos", 5.0f, 5.0f, 5.0f);
    cubieShader_.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    cubieShader_.setFloat("gap", gap_);
    cubieShader_.setFloat("cubieSize", cubieSize_ * cubeScale_);
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

    stickerShader_.use();
    stickerShader_.setMat4("view", glm::value_ptr(viewMatrix));
    stickerShader_.setMat4("projection", glm::value_ptr(projMatrix));
    stickerShader_.setVec3("cameraPos", actualCamPos.x, actualCamPos.y, actualCamPos.z);
    stickerShader_.setVec3("lightPos", 5.0f, 5.0f, 5.0f);
    stickerShader_.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    stickerShader_.setFloat("animAngle", isAnimating ? animAngle : 0.0f);
    stickerShader_.setVec3("animAxis", animAxis.x, animAxis.y, animAxis.z);
    stickerShader_.setInt("stickerCount", stickerCount_);

    for (int i = 0; i < stickerCount_; i++) {
        char centerName[64], normalName[64], colorName[64];
        snprintf(centerName, sizeof(centerName), "stickerCenters[%d]", i);
        snprintf(normalName, sizeof(normalName), "stickerNormals[%d]", i);
        snprintf(colorName, sizeof(colorName), "stickerColors[%d]", i);
        stickerShader_.setVec3(centerName, stickerData_[i].center[0], stickerData_[i].center[1], stickerData_[i].center[2]);
        stickerShader_.setVec3(normalName, stickerData_[i].normal[0], stickerData_[i].normal[1], stickerData_[i].normal[2]);
        stickerShader_.setVec3(colorName, stickerData_[i].color[0], stickerData_[i].color[1], stickerData_[i].color[2]);
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

    // Sticker rendering disabled for debugging
    // glEnable(GL_DEPTH_TEST);
    // stickerShader_.use();
    // renderFullScreenQuad();

    GL_LOADER.glDisableVertexAttribArray(0);
    glDisable(GL_DEPTH_TEST);

    glViewport(0, 0, windowWidth, windowHeight);
}
