#include "renderer_3d_opengl.h"
#include "view_state.h"
#include "color_provider.h"
#include "cube_animator.h"
#include "move_utils.h"
#include <cmath>
#include <iostream>
#include <cstring>

// Face direction definitions: offset and normal for 6 faces
static const struct { float offset; float nx; float ny; float nz; } kFaceDirs[6] = {
    { 0.5f,    0.0f,  0.0f,  1.0f },   // 0: Front (+Z)
    {-0.5f,    0.0f,  0.0f, -1.0f },   // 1: Back (-Z)
    { 0.5f,    0.0f,  1.0f,  0.0f },   // 2: Up (+Y)
    {-0.5f,    0.0f, -1.0f,  0.0f },   // 3: Down (-Y)
    { 0.5f,    1.0f,  0.0f,  0.0f },   // 4: Right (+X)
    {-0.5f,   -1.0f,  0.0f,  0.0f }    // 5: Left (-X)
};

Renderer3DOpenGL::Renderer3DOpenGL() {
    // Setup GL state once at initialization
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);
    
    // FIX #2 & #1: Pre-compute all geometry (eliminates per-frame trig)
    buildGeometry();
    
    std::cout << "OpenGL 3D rendering initialized (pre-computed geometry + vertex arrays)" << std::endl;
}

Renderer3DOpenGL::~Renderer3DOpenGL() {
    // No VBOs to clean up (CPU-side geometry)
}

std::vector<float> Renderer3DOpenGL::buildRoundedRect2D(float size, float cornerRadius) {
    float half = size / 2.0f;
    float inner = half - cornerRadius;
    int segments = 16;  // Same as original (FIX #2: pre-computed once)
    
    // Build triangle fan vertices in 2D (XY plane, centered at origin)
    // FIX #2: Pre-compute all sin/cos values once
    std::vector<float> fan;
    fan.push_back(0.0f);  // center x
    fan.push_back(0.0f);  // center y
    
    auto addCorner = [&](float cx, float cy, float startAngle) {
        for (int i = 0; i <= segments; i++) {
            float angle = startAngle + (M_PI / 2.0f) * i / segments;
            fan.push_back(cx + cornerRadius * cosf(angle));
            fan.push_back(cy + cornerRadius * sinf(angle));
        }
    };
    
    // Add 5 corners in CCW order (triangle fan winding)
    addCorner(inner, -inner, -M_PI / 2.0f);   // Bottom-left
    addCorner(inner,  inner,  0.0f);            // Top-left
    addCorner(-inner, inner,  M_PI / 2.0f);     // Top-right
    addCorner(-inner, -inner,  M_PI);             // Bottom-right
    addCorner(inner,  -inner, -M_PI / 2.0f);   // Close to first
    
    return fan;
}

std::vector<float> Renderer3DOpenGL::fanToTriangles(const std::vector<float>& fan2d) {
    // Convert triangle fan to GL_TRIANGLES
    // FIX #1: Convert to triangles for batch rendering
    // Triangle i = (center, vertex[i], vertex[i+1])
    std::vector<float> tris;
    tris.reserve((fan2d.size() / 2 - 1) * 9);  // (n-2) triangles × 3 coords × 3 floats per vertex
    
    for (size_t i = 1; i < fan2d.size() / 2 - 1; i++) {
        int idx = i * 2;  // fan2d has [x, y] pairs
        // Triangle: center, vertex[i], vertex[i+1]
        tris.push_back(fan2d[0]);  tris.push_back(fan2d[1]);  tris.push_back(0.0f);  // Center
        tris.push_back(fan2d[idx]);   tris.push_back(fan2d[idx + 1]);  tris.push_back(0.0f);  // Vertex i
        tris.push_back(fan2d[idx + 2]); tris.push_back(fan2d[idx + 3]);  tris.push_back(0.0f);  // Vertex i+1
    }
    
    return tris;
}

std::vector<float> Renderer3DOpenGL::transformFaceTo3D(const std::vector<float>& xyTris,
                                                        float offset, float nx, float ny, float nz) {
    std::vector<float> out;
    out.reserve(xyTris.size());
    
    // Transform XY-plane triangle vertices to specific face orientation
    for (size_t i = 0; i < xyTris.size(); i += 3) {
        float u = xyTris[i];
        float v = xyTris[i + 1];
        
        if (nz != 0) {      // ±Z faces (Front/Back)
            out.push_back(u);
            out.push_back(v);
            out.push_back(offset);
        } else if (ny != 0) { // ±Y faces (Up/Down)
            out.push_back(u);
            out.push_back(offset);
            out.push_back(v);
        } else {             // ±X faces (Right/Left)
            out.push_back(offset);
            out.push_back(v);
            out.push_back(u);
        }
    }
    
    return out;
}

void Renderer3DOpenGL::buildCubeBlackFaces() {
    // Build one cube's 6 black faces as triangles (pre-computed once)
    float faceSize = 1.0f;
    float faceRadius = faceSize * 0.10f;
    auto face2d = buildRoundedRect2D(faceSize, faceRadius);
    auto faceTris = fanToTriangles(face2d);
    
    std::vector<float> allVerts;
    cubeBlackFaceGeom_.vertexCount = 0;
    
    for (int f = 0; f < 6; f++) {
        auto transformed = transformFaceTo3D(faceTris,
            kFaceDirs[f].offset, kFaceDirs[f].nx, kFaceDirs[f].ny, kFaceDirs[f].nz);
        allVerts.insert(allVerts.end(), transformed.begin(), transformed.end());
        cubeBlackFaceGeom_.vertexCount += transformed.size() / 3;
    }
    
    cubeBlackFaceGeom_.vertices = std::move(allVerts);
}

void Renderer3DOpenGL::buildStickerTemplates() {
    // Build 6 sticker templates (one per face direction, pre-computed once)
    float stickerSize = 0.9f;
    float stickerRadius = stickerSize * 0.10f;
    float stickerOffsetExtra = 0.001f;
    
    auto sticker2d = buildRoundedRect2D(stickerSize, stickerRadius);
    auto stickerTris = fanToTriangles(sticker2d);
    
    for (int f = 0; f < 6; f++) {
        float offset = kFaceDirs[f].offset;
        // Sticker offset is slightly larger/smaller than face offset
        float stickerOff = (offset > 0) ? offset + stickerOffsetExtra : offset - stickerOffsetExtra;
        
        auto transformed = transformFaceTo3D(stickerTris,
            stickerOff, kFaceDirs[f].nx, kFaceDirs[f].ny, kFaceDirs[f].nz);
        
        stickerTemplates_[f].vertices = std::move(transformed);
        stickerTemplates_[f].vertexCount = stickerTris.size() / 3;
    }
}

void Renderer3DOpenGL::buildStickerInfo() {
    // Pre-compute which stickers each cube has and how to look up their colors
    // FIX #3: Ensures all sticker faces use the same cube state
    // This eliminates per-frame logic for determining visible faces
    
    for (int cubeIndex = 0; cubeIndex < 27; cubeIndex++) {
        int layer = cubeIndex / 9;
        int posInLayer = cubeIndex % 9;
        int row = posInLayer / 3;
        int col = posInLayer % 3;
        
        stickerInfos_[cubeIndex].clear();
        
        // Front face (+Z, layer=2)
        if (layer == 2) {
            int idx = (2 - row) * 3 + col;
            stickerInfos_[cubeIndex].push_back({0, 0, idx});  // Template 0 (+Z), face FRONT
        }
        // Back face (-Z, layer=0)
        if (layer == 0) {
            int idx = (2 - row) * 3 + (2 - col);
            stickerInfos_[cubeIndex].push_back({1, 1, idx});  // Template 1 (-Z), face BACK
        }
        // Up face (+Y, row=2)
        if (row == 2) {
            int idx = layer * 3 + col;
            stickerInfos_[cubeIndex].push_back({2, 2, idx});  // Template 2 (+Y), face UP
        }
        // Down face (-Y, row=0)
        if (row == 0) {
            int idx = (2 - layer) * 3 + col;
            stickerInfos_[cubeIndex].push_back({3, 3, idx});  // Template 3 (-Y), face DOWN
        }
        // Right face (+X, col=2)
        if (col == 2) {
            int idx = (2 - row) * 3 + (2 - layer);
            stickerInfos_[cubeIndex].push_back({4, 4, idx});  // Template 4 (+X), face RIGHT
        }
        // Left face (-X, col=0)
        if (col == 0) {
            int idx = (2 - row) * 3 + layer;
            stickerInfos_[cubeIndex].push_back({5, 5, idx});  // Template 5 (-X), face LEFT
        }
    }
}

void Renderer3DOpenGL::buildCircleCanvas() {
    // Pre-compute circle canvas geometry (FIX #2: no trig per-frame)
    float radius = 0.75f;
    float yOffset = -0.8f;
    int segments = 64;
    float r = 0.3f, g = 0.35f, b = 0.45f, a = 0.3f;
    
    // Circle fill (converted from TRIANGLE_FAN to TRIANGLES for batch rendering)
    // FIX #1: Pre-compute all circle vertices
    std::vector<float> fillVerts;
    for (int i = 0; i < segments; i++) {
        float a1 = 2.0f * M_PI * i / segments;
        float a2 = 2.0f * M_PI * (i + 1) / segments;
        
        // Triangle: (0, yOffset, 0), (radius*cos(a1), yOffset, radius*sin(a1)), (radius*cos(a2), yOffset, radius*sin(a2))
        fillVerts.push_back(0.0f); fillVerts.push_back(yOffset); fillVerts.push_back(0.0f);
        fillVerts.push_back(radius * cosf(a1)); fillVerts.push_back(yOffset); fillVerts.push_back(radius * sinf(a1));
        fillVerts.push_back(radius * cosf(a2)); fillVerts.push_back(yOffset); fillVerts.push_back(radius * sinf(a2));
    }
    circleFillGeom_.vertices = std::move(fillVerts);
    circleFillGeom_.vertexCount = fillVerts.size() / 3;
    
    // Circle line loop
    std::vector<float> lineVerts;
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        lineVerts.push_back(radius * cosf(angle));
        lineVerts.push_back(yOffset);
        lineVerts.push_back(radius * sinf(angle));
    }
    circleLineGeom_.vertices = std::move(lineVerts);
    circleLineGeom_.vertexCount = lineVerts.size() / 3;
}

std::array<Color, 9> Renderer3DOpenGL::getCubeFace(const RubiksCube& cube, int faceIdx) {
    // Helper to look up face color array from face index
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

void Renderer3DOpenGL::buildGeometry() {
    buildCubeBlackFaces();
    buildStickerTemplates();
    buildStickerInfo();
    buildCircleCanvas();
}

void Renderer3DOpenGL::renderCircleCanvas() {
    // Save current matrix state
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    // Draw circle fill using pre-computed geometry (FIX #1: single batch draw)
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &circleFillGeom_.vertices[0]);
    glColor4f(0.3f, 0.35f, 0.45f, 0.3f);
    glDrawArrays(GL_TRIANGLES, 0, circleFillGeom_.vertexCount);
    
    // Draw circle outline using pre-computed geometry (FIX #1: single batch draw)
    glVertexPointer(3, GL_FLOAT, 0, &circleLineGeom_.vertices[0]);
    glColor4f(0.36f, 0.42f, 0.54f, 0.45f);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_LOOP, 0, circleLineGeom_.vertexCount);
    
    // Restore state
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    
    glPopMatrix();
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

void Renderer3DOpenGL::render(int windowWidth, int windowHeight) {
    if (!viewState_ || !colorProvider_ || !animator_ || !cube_) {
        return;
    }
    
    // Setup viewport and scissor (same as original)
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
    
    // Setup projection matrix (perspective)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)viewWidth / viewHeight;
    float fov = 45.0f;
    float near = 0.1f;
    float far = 100.0f;
    float top = tanf(fov * M_PI / 360.0f) * near;
    glFrustum(-top * aspect, top * aspect, -top, top, near, far);
    
    // Setup modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);
    
    // Ensure lighting is disabled (same as original)
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    
    // Draw circle canvas first (before cube rotation, as in original)
    renderCircleCanvas();
    
    // Apply view rotation (X, Y, Z axes)
    glRotatef(viewState_->rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(viewState_->rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(viewState_->rotationZ, 0.0f, 0.0f, 1.0f);
    
    // Apply overall scale factors (same as original)
    glScalef(0.9f, 0.9f, 0.9f);
    glDisable(GL_CULL_FACE);
    glScalef(0.3f, 0.3f, 0.3f);
    
    float gap = 0.03f;
    float animAngle = animator_->getCurrentAngle();
    bool isAnimating = animator_->isAnimating();
    Move animMove = animator_->currentMove();
    RotationAxis animAxis = getRotationAxis(animMove);
    
    // FIX #1: Enable vertex arrays for batch rendering
    // This eliminates per-vertex glBegin/glEnd overhead
    glEnableClientState(GL_VERTEX_ARRAY);
    
    // Render all 27 cubes
    int cubeIndex = 0;
    for (int layer = 0; layer < 3; layer++) {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                // Calculate cube grid position
                float xOffset = (col - 1.0f) * (1.0f + gap);
                float yOffset = (row - 1.0f) * (1.0f + gap);
                float zOffset = (layer - 1.0f) * (1.0f + gap);
                
                glPushMatrix();
                glTranslatef(xOffset, yOffset, zOffset);
                
                // Check if this cube should rotate during animation
                bool shouldAnimate = isAnimating && 
                    MoveLookup::isInSlice(cubeIndex, getAnimationSlice(animMove));
                
                if (shouldAnimate) {
                    // Apply animation rotation around the move's axis
                    glRotatef(animAngle, animAxis.x, animAxis.y, animAxis.z);
                }
                
                // Draw black faces using pre-computed geometry (FIX #1: single batch call)
                glVertexPointer(3, GL_FLOAT, 0, &cubeBlackFaceGeom_.vertices[0]);
                glColor3f(0.0f, 0.0f, 0.0f);
                glDrawArrays(GL_TRIANGLES, 0, cubeBlackFaceGeom_.vertexCount);
                
                // Draw stickers (FIX #3: use correct cube state for ALL faces)
                // During animation, cubes in rotating slice use preAnimationCube;
                // Non-animating cubes use current cube state.
                const RubiksCube& renderCube = shouldAnimate 
                    ? animator_->getPreAnimationCube() : *cube_;
                
                for (const StickerInfo& si : stickerInfos_[cubeIndex]) {
                    // Look up sticker color from the correct cube state
                    const auto& face = getCubeFace(renderCube, si.faceIdx);
                    Color c = face[si.colorIdx];
                    auto rgb = colorProvider_->getFaceColorRgb(c);
                    glColor3f(rgb[0], rgb[1], rgb[2]);
                    
                    // Draw sticker from pre-built template (FIX #1 & #2: single batch call)
                    glVertexPointer(3, GL_FLOAT, 0, &stickerTemplates_[si.templateIdx].vertices[0]);
                    glDrawArrays(GL_TRIANGLES, 0, stickerTemplates_[si.templateIdx].vertexCount);
                }
                
                glPopMatrix();
                cubeIndex++;
            }
        }
    }
    
    // Cleanup
    glDisableClientState(GL_VERTEX_ARRAY);
    
    // Restore viewport for ImGui
    glDisable(GL_LIGHTING);
    glViewport(0, 0, windowWidth, windowHeight);
}
