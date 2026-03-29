#include "renderer_3d_opengl.h"
#include "renderer.h"
#include <GL/gl.h>
#include <cmath>
#include <iostream>

extern "C" {
extern void glUseProgram(GLuint program);
extern void glDisableVertexAttribArray(GLuint index);
}

// kFaceDirs is now in renderer_3d.h (shared)

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

void Renderer3DOpenGL::buildCircleCanvas() {
    float radius = 1.5f;
    float yOffset = -1.6f;
    int segments = 64;
    
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
    circleFillGeom_.vertexCount = fillVerts.size() / 3;
    circleFillGeom_.vertices = std::move(fillVerts);
    
    // Circle line loop
    std::vector<float> lineVerts;
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        lineVerts.push_back(radius * cosf(angle));
        lineVerts.push_back(yOffset);
        lineVerts.push_back(radius * sinf(angle));
    }
    circleLineGeom_.vertexCount = lineVerts.size() / 3;
    circleLineGeom_.vertices = std::move(lineVerts);
}

void Renderer3DOpenGL::buildGeometry() {
    buildCubeBlackFaces();
    buildStickerTemplates();
    for (int i = 0; i < 27; i++) {
        stickerInfos_[i] = IRenderer3D::buildStickerInfoForCube(i);
    }
    buildCircleCanvas();
}

void Renderer3DOpenGL::renderCircleCanvas() {
    // Save current matrix state
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -kCameraDist);
    
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

void Renderer3DOpenGL::render(int windowWidth, int windowHeight, float sidebarWidth) {
    if (!viewState_ || !colorProvider_ || !animator_ || !cube_) {
        return;
    }

    glUseProgram(0);
    glDisableVertexAttribArray(0);

    // Setup viewport and scissor
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
    glTranslatef(0.0f, 0.0f, -kCameraDist);
    
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
    
    glDisable(GL_CULL_FACE);

    float spacing = (kCubieFace + gap_) * cubeScale_;
    float cubieSize = kCubieFace * cubeScale_;
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
                // Calculate cube grid position in world space
                float xOffset = (col - 1.0f) * spacing;
                float yOffset = (row - 1.0f) * spacing;
                float zOffset = (layer - 1.0f) * spacing;
                
                // Check if this cube should rotate during animation
                bool shouldAnimate = isAnimating && 
                    MoveLookup::isInSlice(cubeIndex, getAnimationSlice(animMove));
                
                glPushMatrix();
                
                // CRITICAL: Apply rotation BEFORE translation
                // This makes the cube rotate around the cube center (layer axis),
                // not around its own center (which would look like spinning in place)
                if (shouldAnimate) {
                    glRotatef(animAngle, animAxis.x, animAxis.y, animAxis.z);
                }
                
                glTranslatef(xOffset, yOffset, zOffset);
                glScalef(cubieSize, cubieSize, cubieSize);
                
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
                    const auto& face = IRenderer3D::getCubeFace(renderCube, si.faceIdx);
                    Color c = face[si.colorIdx];
                    auto rgb = colorProvider_->getFaceColorRgb(c);
                    glColor3f(rgb.r, rgb.g, rgb.b);
                    
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
