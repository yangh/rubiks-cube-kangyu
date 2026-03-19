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
uniform float animSliceMask[27];
uniform vec3 faceColors[162];
uniform float gap;
uniform vec3 lightPos;
uniform vec3 lightColor;

float sdRoundBox(vec3 p, vec3 b, float r) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}

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

int getFaceIndex(vec3 n) {
    vec3 an = abs(n);
    if (an.x >= an.y && an.x >= an.z) {
        return n.x > 0.0 ? 4 : 5;
    } else if (an.y >= an.z) {
        return n.y > 0.0 ? 2 : 3;
    } else {
        return n.z > 0.0 ? 0 : 1;
    }
}

vec2 projectToFace(vec3 localP, vec3 n) {
    vec3 an = abs(n);
    if (an.x >= an.y && an.x >= an.z) {
        return localP.yz;
    } else if (an.y >= an.z) {
        return localP.xz;
    } else {
        return localP.xy;
    }
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

    int ci = cubieIdx;
    int fi = getFaceIndex(n);

    float stickerSize = cubieSize * 0.92;
    float stickerRadius = cubieSize * 0.08;

    vec3 pos = cubiePositions[ci];
    mat3 animRot;
    bool hasAnim = (animAngle != 0.0);
    if (hasAnim && animSliceMask[ci] > 0.5) {
        animRot = rotateAroundAxis(animAxis, radians(animAngle));
        pos = animRot * pos;
    }
    vec3 localP = p - pos;
    if (hasAnim && animSliceMask[ci] > 0.5) {
        localP = transpose(animRot) * localP;
    }

    vec2 uv = projectToFace(localP, n);
    float sd = sdRoundRect(uv, vec2(stickerSize), stickerRadius);

    vec3 baseColor = vec3(0.02, 0.02, 0.02);
    vec3 stickerColor = faceColors[ci * 6 + fi];
    vec3 surfaceColor = (sd < 0.0) ? stickerColor : baseColor;

    vec3 lightDir = normalize(lightPos - p);
    vec3 viewDir = normalize(cameraPos - p);
    vec3 reflectDir = reflect(-lightDir, n);

    float ambient = 0.25;
    float diff = max(dot(n, lightDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 color = surfaceColor * (ambient + diff) + lightColor * spec * 0.4;

    fragColor = vec4(color, 1.0);
}
)";

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
