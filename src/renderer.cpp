#include "renderer.h"
#include "config.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// Global flag to enable/disable cube dump (defined in main.cpp)
extern bool g_enableDump;

CubeRenderer::CubeRenderer()
    : cube_()
{
    if (initGL3D()) {
        std::cout << "OpenGL 3D rendering initialized successfully" << std::endl;
    }
}

void CubeRenderer::setCustomColors(const ColorConfig& config) {
    customFront = config.getFrontColor();
    customBack = config.getBackColor();
    customLeft = config.getLeftColor();
    customRight = config.getRightColor();
    customUp = config.getUpColor();
    customDown = config.getDownColor();
    useCustomColors = !config.isUsingDefaults();
}

void CubeRenderer::executeMove(Move move) {
    moveQueue_.push(move);

    if (!isAnimating_ && moveQueue_.size() == 1) {
        startNextAnimation();
    }

    if (g_enableDump) {
        std::cout << "\n=== Queued " << moveToString(move) << " ===" << std::endl;
    }
}

void CubeRenderer::reset() {
    cube_.reset();
    isAnimating_ = false;
    animationProgress_ = 0.0f;
    while (!moveQueue_.empty()) {
        moveQueue_.pop();
    }
}

void CubeRenderer::undo() {
    // Undo the move on the cube
    cube_.undo();

    // Clear animation state
    isAnimating_ = false;
    animationProgress_ = 0.0f;
    while (!moveQueue_.empty()) {
        moveQueue_.pop();
    }
}

void CubeRenderer::redo() {
    // Redo the move on the cube
    cube_.redo();

    // Clear animation state
    isAnimating_ = false;
    animationProgress_ = 0.0f;
    while (!moveQueue_.empty()) {
        moveQueue_.pop();
    }
}

std::vector<Move> CubeRenderer::scramble(int numMoves) {
    // Generate scramble moves from the cube
    std::vector<Move> scrambleMoves = cube_.scramble(numMoves);

    // Clear the move queue and animation state
    isAnimating_ = false;
    animationProgress_ = 0.0f;
    while (!moveQueue_.empty()) {
        moveQueue_.pop();
    }

    return scrambleMoves;
}

void CubeRenderer::resetView() {
    rotationX = 30.0f;
    rotationY = -30.0f;
    rotationZ = 0.0f;
    targetRotationX = 30.0f;
    targetRotationY = -30.0f;
    targetRotationZ = 0.0f;
    scale = 3.1f;
    scale2D = 0.8f;
    animationSpeed = 1.0f;
    enableAnimation = true;
}

bool CubeRenderer::isSolved() const {
    return cube_.isSolved();
}

void CubeRenderer::dump() const {
    cube_.dump();
}

bool CubeRenderer::initGL3D() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Disable lighting to show colors directly
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);

    // Enable smooth shading
    glShadeModel(GL_SMOOTH);

    // Load cube model
    cubeModel = new Model("src/models/cube.obj");

    if (cubeModel && !cubeModel->meshes.empty()) {
        std::cout << "Model loaded: " << cubeModel->meshes.size() << " mesh(es), "
                  << cubeModel->meshes[0].vertices.size() << " vertices" << std::endl;
    } else {
        std::cout << "ERROR: Failed to load model" << std::endl;
    }

    return (cubeModel != nullptr);
}

void CubeRenderer::draw2D(ImDrawList* drawList, ImVec2 offset, float scale) {
    // Draw cube net (2D unfolded view)
    float stickerSize = 30.0f * scale;
    float gap = 3.0f * scale;
    float faceSize = stickerSize * 3.0f + gap * 2.0f;

    // Standard cross-shaped layout: Up above Front, Left beside Front
    // Face spacing
    float spacing = faceSize + gap;
    offset.x -= spacing * 0.5;
    //offset.y += spacing * 0.5;

    // Draw in order: Up, Left, Front, Right, Down, Back
    drawFace(drawList, cube_.getUp(),
            ImVec2(offset.x, offset.y - spacing),
            stickerSize, gap, false, Color::WHITE);

    drawFace(drawList, cube_.getLeft(),
            ImVec2(offset.x - spacing, offset.y),
            stickerSize, gap, false, Color::ORANGE);
    drawFace(drawList, cube_.getFront(),
            ImVec2(offset.x + spacing * 0, offset.y),
            stickerSize, gap, false, Color::GREEN);
    drawFace(drawList, cube_.getRight(),
            ImVec2(offset.x + spacing * 1, offset.y),
            stickerSize, gap, false, Color::RED);
    drawFace(drawList, cube_.getBack(),
            ImVec2(offset.x + spacing * 2, offset.y),
            stickerSize, gap, false, Color::BLUE);
    drawFace(drawList, cube_.getDown(),
            ImVec2(offset.x, offset.y + spacing),
            stickerSize, gap, false, Color::YELLOW);
}

void CubeRenderer::draw3D(ImDrawList* drawList, ImVec2 offset, float scale) {
    if (!cubeModel) {
        // Fallback to old rendering if OpenGL 3D not initialized
        return;
    }

    // Get window dimensions from ImGui
    ImGuiIO& io = ImGui::GetIO();
    int windowWidth = io.DisplaySize.x;
    int windowHeight = io.DisplaySize.y;

    // Calculate the 3D view window position and size
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    // Set viewport to match the 3D view window content area
    glViewport(windowPos.x, windowHeight - windowPos.y - windowSize.y, windowSize.x, windowSize.y);

    // Clear depth buffer for this viewport
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set up projection matrix (perspective)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = windowSize.x / windowSize.y;
    float fov = 45.0f;
    float near = 0.1f;
    float far = 100.0f;
    float top = tanf(fov * M_PI / 360.0f) * near;
    float bottom = -top;
    float right = top * aspect;
    float left = -right;
    glFrustum(left, right, bottom, top, near, far);

    // Set up model-view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Apply camera translation (move back from the cube)
    glTranslatef(0.0f, 0.0f, -3.0f);

    // Apply rotation from user controls
    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(rotationZ, 0.0f, 0.0f, 1.0f);

    // Scale down the cube slightly
    glScalef(0.9f, 0.9f, 0.9f);

    // Set object color (white cube)
    glColor3f(1.0f, 1.0f, 1.0f);

    // Draw the cube model
    cubeModel->Draw();

    // Reset viewport to full window
    glViewport(0, 0, windowWidth, windowHeight);
}

void CubeRenderer::render3DOverlay(int windowWidth, int windowHeight) {
    if (!cubeModel) {
        return;
    }

    // Hardcoded 3D view window position (same as in main.cpp)
    int sidebarWidth = 400;
    int viewX = 10;
    int viewY = 10;
    int viewWidth = windowWidth - sidebarWidth - 20;
    int viewHeight = windowHeight - 20;

    // Set viewport to match the 3D view window
    glViewport(viewX, windowHeight - viewY - viewHeight, viewWidth, viewHeight);

    // Clear depth buffer for this viewport only
    glEnable(GL_SCISSOR_TEST);
    glScissor(viewX, windowHeight - viewY - viewHeight, viewWidth, viewHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    // Set up projection matrix (perspective)
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

    // Set up model-view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Apply camera translation (move back from the cube)
    glTranslatef(0.0f, 0.0f, -3.0f);

    // Apply rotation from user controls
    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(rotationZ, 0.0f, 0.0f, 1.0f);

    // Scale down the cube slightly
    glScalef(0.9f, 0.9f, 0.9f);

    // Disable lighting - render colors directly
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);

    // Disable face culling to show all faces
    glDisable(GL_CULL_FACE);

    // Scale down the cube
    glScalef(0.3f, 0.3f, 0.3f);

    // Draw 27 cubes in a 3x3x3 cube with gaps
    float gap = 0.03f;  // Gap between cubes
    int cubeIndex = 0;

    // Calculate animation angle for rotating cubes
    float animAngle = 0.0f;
    if (isAnimating_) {
        // Quadratic ease-in-out: 3t^2 - 2t^3
        float easeProgress = animationProgress_ * animationProgress_ * (3.0f - 2.0f * animationProgress_);
        animAngle = 90.0f * easeProgress;
    }

    for (int layer = 0; layer < 3; layer++) {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                bool cubeShouldAnimate = isAnimating_ && isCubeAnimating(cubeIndex);

                float xOffset = (col - 1.0f) * (1.0f + gap);
                float yOffset = (row - 1.0f) * (1.0f + gap);
                float zOffset = (layer - 1.0f) * (1.0f + gap);

                glPushMatrix();

                // Apply rotation transformation only to cubes in the rotating slice
                if (cubeShouldAnimate) {
                    applyRotationTransform(animAngle, currentMove_);
                }

                glTranslatef(xOffset, yOffset, zOffset);
                drawCube(cubeIndex, cubeShouldAnimate);
                glPopMatrix();

                cubeIndex++;
            }
        }
    }

    // Disable lighting (restore to default)
    glDisable(GL_LIGHTING);

    // Reset viewport to full window
    glViewport(0, 0, windowWidth, windowHeight);
}

ImVec2 CubeRenderer::project(float x, float y, float z, ImVec2 center, float scale) {
    // Isometric-style projection
    float angleX = rotationX * M_PI / 180.0f;
    float angleY = rotationY * M_PI / 180.0f;
    float angleZ = rotationZ * M_PI / 180.0f;

    // Rotate around X axis
    float y1 = y * cosf(angleX) - z * sinf(angleX);
    float z1 = y * sinf(angleX) + z * cosf(angleX);

    // Rotate around Y axis
    float x2 = x * cosf(angleY) + z1 * sinf(angleY);
    float z2 = -x * sinf(angleY) + z1 * cosf(angleY);

    // Rotate around Z axis (in the XY plane)
    float x3 = x2 * cosf(angleZ) - y1 * sinf(angleZ);
    float y3 = x2 * sinf(angleZ) + y1 * cosf(angleZ);

    return ImVec2(
        center.x + x3 * scale,
        center.y + y3 * scale
    );
}

void CubeRenderer::drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                          ImVec2 offset, float size, float gap, bool flipVertical, Color faceType) {
    float totalSize = size * 3.0f + gap * 2.0f;
    float startX = offset.x - totalSize / 2.0f + size / 2.0f;
    float startY = offset.y - totalSize / 2.0f + size / 2.0f;

    // Draw black background for face
    ImVec2 p1(offset.x - totalSize / 2.0f, offset.y + totalSize / 2.0f);
    ImVec2 p2(offset.x + totalSize / 2.0f, offset.y + totalSize / 2.0f);
    ImVec2 p3(offset.x + totalSize / 2.0f, offset.y - totalSize / 2.0f);
    ImVec2 p4(offset.x - totalSize / 2.0f, offset.y - totalSize / 2.0f);
    drawList->AddQuadFilled(p1, p2, p3, p4, IM_COL32(25, 25, 25, 255));

    // Draw 3x3 grid of stickers
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            // Map visual position to face index, with optional vertical flip
            // Normal: row 0 -> indices 0,1,2 (top)
            // Flipped: row 0 -> indices 6,7,8 (bottom)
            // For Back face (viewed from behind), use correct mapping without flip
            int index;
            if (faceType == Color::BLUE) {
                // Back face: use standard mapping (no flip needed)
                // Back face is already in the correct orientation in cube data
                index = row * 3 + col;
            } else if (flipVertical) {
                // Other faces with flipVertical: only flip vertically
                index = (2 - row) * 3 + col;
            } else {
                index = row * 3 + col;
            }

            ImU32 color = getFaceColor(face[index]);
            float x = startX + static_cast<float>(col) * (size + gap);
            float y = startY + static_cast<float>(row) * (size + gap);

            // Draw sticker
            ImVec2 s1(x - size / 2.0f, y + size / 2.0f);
            ImVec2 s2(x + size / 2.0f, y + size / 2.0f);
            ImVec2 s3(x + size / 2.0f, y - size / 2.0f);
            ImVec2 s4(x - size / 2.0f, y - size / 2.0f);
            drawList->AddQuadFilled(s1, s2, s3, s4, color);

            // Draw black border
            ImU32 black = IM_COL32(0, 0, 0, 255);
            drawList->AddQuad(s1, s2, s3, s4, black, 2.0f);
        }
    }
}

void CubeRenderer::draw3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                            const ImVec2 (&faceVerts)[4], ImVec2 center, float size) {
    // Project vertices to screen space
    ImVec2 projected[4];
    for (int i = 0; i < 4; ++i) {
        float x = 0, y = 0, z = 0;

        // Map face vertices to 3D positions
        if (face == cube_.getFront()) { z = 1.0f; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (face == cube_.getBack()) { z = -1.0f; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (face == cube_.getLeft()) { x = -1.0f; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (face == cube_.getRight()) { x = 1.0f; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (face == cube_.getUp()) { y = 1.0f; x = faceVerts[i].x; z = faceVerts[i].y; }
        else if (face == cube_.getDown()) { y = -1.0f; x = faceVerts[i].x; z = faceVerts[i].y; }

        projected[i] = project(x * 1.1f, y * 1.1f, z * 1.1f, center, size);
    }

    // Draw black background for face
    drawList->AddQuadFilled(projected[0], projected[1], projected[2], projected[3],
                          IM_COL32(20, 20, 20, 255));

    // Draw 3x3 stickers on the face
    // Each sticker is at a position in the face's local coordinate system
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int index = row * 3 + col;
            float u = (col - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67
            float v = (row - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67

            // Get face color
            ImU32 stickerColor = getFaceColor(face[index]);

            // Calculate sticker center position in 3D space
            // u = x offset (left to right), v = y offset (top to bottom) in face-local space
            float x = 0, y = 0, z = 0;

            if (face == cube_.getFront()) {
                // Front face (z=+1): col -> x, row -> -y
                x = u; y = -v; z = 1.0f;
            } else if (face == cube_.getBack()) {
                // Back face (z=-1): col -> -x, row -> -y (mirrored when viewing from front)
                x = -u; y = -v; z = -1.0f;
            } else if (face == cube_.getLeft()) {
                // Left face (x=-1): col -> z, row -> -y
                // col 0 (left in 2D) should be at negative z (back), col 2 at positive z (front)
                x = -1.0f; y = -v; z = u;
            } else if (face == cube_.getRight()) {
                // Right face (x=+1): col -> -z, row -> -y
                // col 0 (left in 2D) should be at positive z (front), col 2 at negative z (back)
                x = 1.0f; y = -v; z = -u;
            } else if (face == cube_.getUp()) {
                // Up face (y=+1): col -> x, row -> z
                // row 0 (top in 2D) should be at positive z (back), row 2 at negative z (front)
                x = u; y = 1.0f; z = v;
            } else if (face == cube_.getDown()) {
                // Down face (y=-1): col -> x, row -> -z
                // row 0 (top in 2D) should be at negative z (front), row 2 at positive z (back)
                x = u; y = -1.0f; z = -v;
            }

            // Calculate sticker corners relative to sticker center
            float stickerSize = 0.6f;
            float halfSize = stickerSize / 2.0f;

            // Sticker corners in face-local coordinates (dx, dy offset from center)
            // dx = horizontal offset (left/right), dy = vertical offset (up/down)
            float cornersLocal[4][2] = {
                {-halfSize, halfSize},  // top-left: dx negative, dy positive
                {halfSize, halfSize},   // top-right: dx positive, dy positive
                {halfSize, -halfSize},  // bottom-right: dx positive, dy negative
                {-halfSize, -halfSize}  // bottom-left: dx negative, dy negative
            };

            // Project all 4 corners to screen space
            ImVec2 corners[4];
            for (int c = 0; c < 4; ++c) {
                float dx = cornersLocal[c][0];  // horizontal offset
                float dy = cornersLocal[c][1];  // vertical offset
                float cx = 0, cy = 0, cz = 0;

                // Map local offsets to 3D space based on face orientation
                if (face == cube_.getFront()) {
                    cx = x + dx; cy = y + dy; cz = 1.0f;
                } else if (face == cube_.getBack()) {
                    cx = x - dx; cy = y + dy; cz = -1.0f;
                } else if (face == cube_.getLeft()) {
                    cx = -1.0f; cy = y + dy; cz = z + dx;
                } else if (face == cube_.getRight()) {
                    cx = 1.0f; cy = y + dy; cz = z - dx;
                } else if (face == cube_.getUp()) {
                    cx = x + dx; cy = 1.0f; cz = z + dy;
                } else if (face == cube_.getDown()) {
                    cx = x + dx; cy = -1.0f; cz = z - dy;
                }

                corners[c] = project(cx * 1.1f, cy * 1.1f, cz * 1.1f, center, size);
            }

            // Draw sticker
            drawList->AddQuadFilled(corners[0], corners[1], corners[2], corners[3], stickerColor);
            // Draw black border around sticker to reduce aliasing
            ImU32 black = IM_COL32(0, 0, 0, 255);
            drawList->AddQuad(corners[0], corners[1], corners[2], corners[3], black, 1.5f);
        }
    }
}

void CubeRenderer::drawAnimated3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                                      const ImVec2 (&faceVerts)[4], ImVec2 center, float size,
                                      int faceIndex) {
    // Quadratic ease-in-out: 3t^2 - 2t^3
    float easeProgress = animationProgress_ * animationProgress_ * (3.0f - 2.0f * animationProgress_);
    float currentAngle = 90.0f * easeProgress;  // Always use positive 90 degrees, rotateSticker handles direction

    // Draw black background for face
    ImVec2 projected[4];
    for (int i = 0; i < 4; ++i) {
        float x = 0, y = 0, z = 0;

        // Map face vertices to 3D positions
        if (faceIndex == 0) { z = 1.0f; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (faceIndex == 1) { z = -1.0f; x = faceVerts[i].x; y = faceVerts[i].y; }
        else if (faceIndex == 2) { x = -1.0f; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (faceIndex == 3) { x = 1.0f; y = faceVerts[i].x; z = faceVerts[i].y; }
        else if (faceIndex == 4) { y = 1.0f; x = faceVerts[i].x; z = faceVerts[i].y; }
        else if (faceIndex == 5) { y = -1.0f; x = faceVerts[i].x; z = faceVerts[i].y; }

        projected[i] = project(x * 1.1f, y * 1.1f, z * 1.1f, center, size);
    }

    drawList->AddQuadFilled(projected[0], projected[1], projected[2], projected[3],
                          IM_COL32(20, 20, 20, 255));

    // Draw 3x3 stickers on the face
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int stickerIndex = row * 3 + col;
            float u = (col - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67
            float v = (row - 1.0f) / 3.0f * 2.0f;  // -0.67, 0, 0.67

            // Get face color
            ImU32 stickerColor = getFaceColor(face[stickerIndex]);

            // Calculate sticker center position in 3D space
            float x = 0, y = 0, z = 0;

            if (faceIndex == 0) { x = u; y = -v; z = 1.0f; }
            else if (faceIndex == 1) { x = -u; y = -v; z = -1.0f; }
            else if (faceIndex == 2) { x = -1.0f; y = -v; z = u; }
            else if (faceIndex == 3) { x = 1.0f; y = -v; z = -u; }
            else if (faceIndex == 4) { x = u; y = 1.0f; z = v; }
            else if (faceIndex == 5) { x = u; y = -1.0f; z = -v; }

            // Calculate sticker corners relative to sticker center
            float stickerSize = 0.6f;
            float halfSize = stickerSize / 2.0f;

            // Sticker corners in face-local coordinates (dx, dy offset from center)
            float cornersLocal[4][2] = {
                {-halfSize, halfSize},
                {halfSize, halfSize},
                {halfSize, -halfSize},
                {-halfSize, -halfSize}
            };

            // Project all 4 corners to screen space
            ImVec2 corners[4];
            for (int c = 0; c < 4; ++c) {
                float dx = cornersLocal[c][0];
                float dy = cornersLocal[c][1];
                float cx = 0, cy = 0, cz = 0;

                // Map local offsets to 3D space based on face orientation
                if (faceIndex == 0) { cx = x + dx; cy = y + dy; cz = 1.0f; }
                else if (faceIndex == 1) { cx = x - dx; cy = y + dy; cz = -1.0f; }
                else if (faceIndex == 2) { cx = -1.0f; cy = y + dy; cz = z + dx; }
                else if (faceIndex == 3) { cx = 1.0f; cy = y + dy; cz = z - dx; }
                else if (faceIndex == 4) { cx = x + dx; cy = 1.0f; cz = z + dy; }
                else if (faceIndex == 5) { cx = x + dx; cy = -1.0f; cz = z - dy; }

                // Apply animation rotation if this sticker is part of the rotating slice
                if (isStickerAnimating(currentMove_, faceIndex, stickerIndex)) {
                    std::array<float, 3> rotated = rotateSticker({cx, cy, cz}, currentMove_, currentAngle);
                    cx = rotated[0];
                    cy = rotated[1];
                    cz = rotated[2];
                }

                corners[c] = project(cx * 1.1f, cy * 1.1f, cz * 1.1f, center, size);
            }

            // Draw sticker
            drawList->AddQuadFilled(corners[0], corners[1], corners[2], corners[3], stickerColor);
            ImU32 black = IM_COL32(0, 0, 0, 255);
            drawList->AddQuad(corners[0], corners[1], corners[2], corners[3], black, 1.5f);
        }
    }
}

ImU32 CubeRenderer::getFaceColor(Color color) {
    std::array<float, 3> rgb;

    // Use custom colors if enabled, otherwise use default colors
    if (useCustomColors) {
        switch (color) {
            case Color::GREEN:  rgb = customFront; break;   // Front face
            case Color::BLUE:   rgb = customBack; break;    // Back face
            case Color::ORANGE: rgb = customLeft; break;    // Left face
            case Color::RED:    rgb = customRight; break;   // Right face
            case Color::WHITE:  rgb = customUp; break;      // Up face
            case Color::YELLOW: rgb = customDown; break;    // Down face
            default:            rgb = colorToRgb(color); break;
        }
    } else {
        rgb = colorToRgb(color);
    }

    return IM_COL32(
        static_cast<int>(rgb[0] * 255),
        static_cast<int>(rgb[1] * 255),
        static_cast<int>(rgb[2] * 255),
        255
    );
}

// Get face color as RGB array for OpenGL rendering
std::array<float, 3> CubeRenderer::getFaceColorRgb(Color color) {
    std::array<float, 3> rgb;

    // Use custom colors if enabled, otherwise use default colors
    if (useCustomColors) {
        switch (color) {
            case Color::GREEN:  rgb = customFront; break;   // Front face
            case Color::BLUE:   rgb = customBack; break;    // Back face
            case Color::ORANGE: rgb = customLeft; break;    // Left face
            case Color::RED:    rgb = customRight; break;   // Right face
            case Color::WHITE:  rgb = customUp; break;      // Up face
            case Color::YELLOW: rgb = customDown; break;    // Down face
            default:            rgb = colorToRgb(color); break;
        }
    } else {
        rgb = colorToRgb(color);
    }

    return rgb;
}

// Helper function to rotate a 3D point
std::array<float, 3> rotatePoint(const std::array<float, 3>& pos, float ax, float ay, float az) {
    float x = pos[0], y = pos[1], z = pos[2];

    // Rotate around Z axis
    float x1 = x * cosf(az) - y * sinf(az);
    float y1 = x * sinf(az) + y * cosf(az);

    // Rotate around X axis
    float y2 = y1 * cosf(ax) - z * sinf(ax);
    float z1 = y1 * sinf(ax) + z * cosf(ax);

    // Rotate around Y axis
    float x2 = x1 * cosf(ay) + z1 * sinf(ay);
    float z2 = -x * sinf(ay) + z1 * cosf(ay);

    return {x2, y2, z2};
}

// Step 1: Draw a simple test cube with different colors for each face
void CubeRenderer::drawTestCube(ImDrawList* drawList, ImVec2 center, float size) {
    // Cube size in 3D space
    float halfSize = 0.4f;  // Cube extends from -0.4 to +0.4

    // Define all 6 faces with their 4 corners each
    // Format: {{x1,y1,z1}, {x2,y2,z2}, {x3,y3,z3}, {x4,y4,z4}}, color
    struct Face {
        float vertices[4][3];
        ImU32 color;
    };

    Face faces[6] = {
        // +X face (right) - RED - x = +halfSize
        {{{halfSize, halfSize, halfSize}, {halfSize, halfSize, -halfSize},
          {halfSize, -halfSize, -halfSize}, {halfSize, -halfSize, halfSize}},
         IM_COL32(255, 0, 0, 255)},

        // -X face (left) - GREEN - x = -halfSize
        {{{-halfSize, halfSize, -halfSize}, {-halfSize, halfSize, halfSize},
          {-halfSize, -halfSize, halfSize}, {-halfSize, -halfSize, -halfSize}},
         IM_COL32(0, 255, 0, 255)},

        // +Y face (top) - BLUE - y = +halfSize
        {{{-halfSize, halfSize, halfSize}, {halfSize, halfSize, halfSize},
          {halfSize, halfSize, -halfSize}, {-halfSize, halfSize, -halfSize}},
         IM_COL32(0, 100, 255, 255)},

        // -Y face (bottom) - YELLOW - y = -halfSize
        {{{-halfSize, -halfSize, -halfSize}, {halfSize, -halfSize, -halfSize},
          {halfSize, -halfSize, halfSize}, {-halfSize, -halfSize, halfSize}},
         IM_COL32(255, 255, 0, 255)},

        // +Z face (front) - MAGENTA - z = +halfSize
        {{{-halfSize, halfSize, halfSize}, {-halfSize, -halfSize, halfSize},
          {halfSize, -halfSize, halfSize}, {halfSize, halfSize, halfSize}},
         IM_COL32(255, 0, 255, 255)},

        // -Z face (back) - CYAN - z = -halfSize
        {{{halfSize, halfSize, -halfSize}, {halfSize, -halfSize, -halfSize},
          {-halfSize, -halfSize, -halfSize}, {-halfSize, halfSize, -halfSize}},
         IM_COL32(0, 255, 255, 255)}
    };

    // Calculate rotation angles for depth sorting
    float angleX = rotationX * M_PI / 180.0f;
    float angleY = rotationY * M_PI / 180.0f;

    // Helper to rotate a point
    auto rotatePoint = [](float x, float y, float z, float ax, float ay) -> std::array<float, 3> {
        float y1 = y * cosf(ax) - z * sinf(ax);
        float z1 = y * sinf(ax) + z * cosf(ax);
        float x2 = x * cosf(ay) + z1 * sinf(ay);
        float z2 = -x * sinf(ay) + z1 * cosf(ay);
        return {x2, y1, z2};
    };

    // Helper to get depth of a face center
    auto getFaceDepth = [&](const Face& face) -> float {
        float cx = 0, cy = 0, cz = 0;
        for (int v = 0; v < 4; ++v) {
            cx += face.vertices[v][0];
            cy += face.vertices[v][1];
            cz += face.vertices[v][2];
        }
        auto rotated = rotatePoint(cx/4, cy/4, cz/4, angleX, angleY);
        return rotated[2];  // Depth is z after rotation
    };

    // Sort faces by depth (far to near)
    int indices[6] = {0, 1, 2, 3, 4, 5};
    for (int i = 0; i < 6; ++i) {
        for (int j = i + 1; j < 6; ++j) {
            float depthI = getFaceDepth(faces[indices[i]]);
            float depthJ = getFaceDepth(faces[indices[j]]);
            if (depthI < depthJ) {  // Smaller z = farther
                std::swap(indices[i], indices[j]);
            }
        }
    }

    // Draw faces in sorted order
    for (int i = 0; i < 6; ++i) {
        int idx = indices[i];
        const Face& face = faces[idx];

        // Project all 4 vertices
        ImVec2 projected[4];
        for (int v = 0; v < 4; ++v) {
            projected[v] = project(face.vertices[v][0], face.vertices[v][1],
                                   face.vertices[v][2], center, size);
        }

        // Draw the face
        drawList->AddQuadFilled(projected[0], projected[1], projected[2], projected[3], face.color);
    }
}

// Animation implementation
void CubeRenderer::updateAnimation(float deltaTime) {
    // Handle move animation (face rotation)
    if (isAnimating_) {
        const float ANIMATION_DURATION = 0.2f / animationSpeed;  // 200ms divided by speed
        animationProgress_ += deltaTime / ANIMATION_DURATION;

        if (animationProgress_ >= 1.0f) {
            animationProgress_ = 1.0f;
            cube_.executeMove(currentMove_);  // Apply actual move
            isAnimating_ = false;

            if (g_enableDump) {
                std::cout << "\n=== Completed " << moveToString(currentMove_) << " ===" << std::endl;
                cube_.dump();
            }

            startNextAnimation();  // Start next queued move
        }
    }

    // Handle view rotation smoothing (always active)
    lerpRotation(rotationX, targetRotationX, deltaTime);
    lerpRotation(rotationY, targetRotationY, deltaTime);
    lerpRotation(rotationZ, targetRotationZ, deltaTime);
}

void CubeRenderer::startNextAnimation() {
    if (moveQueue_.empty()) {
        isAnimating_ = false;
        return;
    }

    // If animation is disabled, execute moves immediately
    if (!enableAnimation) {
        while (!moveQueue_.empty()) {
            Move move = moveQueue_.front();
            moveQueue_.pop();
            cube_.executeMove(move);
            if (g_enableDump) {
                std::cout << "\n=== Completed " << moveToString(move) << " ===" << std::endl;
                cube_.dump();
            }
        }
        isAnimating_ = false;
        return;
    }

    isAnimating_ = true;
    animationProgress_ = 0.0f;
    currentMove_ = moveQueue_.front();
    moveQueue_.pop();
    preAnimationCube_ = cube_;  // Save current state

    if (g_enableDump) {
        std::cout << "\n=== Starting " << moveToString(currentMove_) << " ===" << std::endl;
    }
}

// Check if a specific sticker is part of the rotating slice for the current move
bool CubeRenderer::isStickerAnimating(Move move, int faceIndex, int stickerIndex) const {
    // faceIndex: 0=Front, 1=Back, 2=Left, 3=Right, 4=Up, 5=Down
    // stickerIndex: 0-8 (row*3 + col)

    switch (move) {
        // U move: Up face (all 9) + Front/Left/Back/Right top row (0,1,2)
        case Move::U:
        case Move::UP:
            if (faceIndex == 4) return true;  // Up face
            if (stickerIndex >= 0 && stickerIndex <= 2) {  // Top row
                return faceIndex == 0 || faceIndex == 2 || faceIndex == 1 || faceIndex == 3;  // F, L, B, R
            }
            return false;

        // D move: Down face (all 9) + Front/Left/Back/Right bottom row (6,7,8)
        case Move::D:
        case Move::DP:
            if (faceIndex == 5) return true;  // Down face
            if (stickerIndex >= 6 && stickerIndex <= 8) {  // Bottom row
                return faceIndex == 0 || faceIndex == 2 || faceIndex == 1 || faceIndex == 3;  // F, L, B, R
            }
            return false;

        // L move: Left face (all 9) + Up/Front/Down/Back left column (0,3,6)
        case Move::L:
        case Move::LP:
            if (faceIndex == 2) return true;  // Left face
            if (stickerIndex == 0 || stickerIndex == 3 || stickerIndex == 6) {  // Left column
                return faceIndex == 4 || faceIndex == 0 || faceIndex == 5 || faceIndex == 1;  // U, F, D, B
            }
            return false;

        // R move: Right face (all 9) + Up/Front/Down/Back right column (2,5,8)
        case Move::R:
        case Move::RP:
            if (faceIndex == 3) return true;  // Right face
            if (stickerIndex == 2 || stickerIndex == 5 || stickerIndex == 8) {  // Right column
                return faceIndex == 4 || faceIndex == 0 || faceIndex == 5 || faceIndex == 1;  // U, F, D, B
            }
            return false;

        // F move: Front face (all 9) + Up[6,7,8] + Left[2,5,8] + Down[0,1,2] + Right[0,3,6]
        case Move::F:
        case Move::FP:
            if (faceIndex == 0) return true;  // Front face
            if (faceIndex == 4 && stickerIndex >= 6 && stickerIndex <= 8) return true;  // Up bottom row
            if (faceIndex == 2 && (stickerIndex == 2 || stickerIndex == 5 || stickerIndex == 8)) return true;  // Left right col
            if (faceIndex == 5 && stickerIndex >= 0 && stickerIndex <= 2) return true;  // Down top row
            if (faceIndex == 3 && (stickerIndex == 0 || stickerIndex == 3 || stickerIndex == 6)) return true;  // Right left col
            return false;

        // B move: Back face (all 9) + Up[0,1,2] + Left[0,3,6] + Down[6,7,8] + Right[2,5,8]
        case Move::B:
        case Move::BP:
            if (faceIndex == 1) return true;  // Back face
            if (faceIndex == 4 && stickerIndex >= 0 && stickerIndex <= 2) return true;  // Up top row
            if (faceIndex == 2 && (stickerIndex == 0 || stickerIndex == 3 || stickerIndex == 6)) return true;  // Left left col
            if (faceIndex == 5 && stickerIndex >= 6 && stickerIndex <= 8) return true;  // Down bottom row
            if (faceIndex == 3 && (stickerIndex == 2 || stickerIndex == 5 || stickerIndex == 8)) return true;  // Right right col
            return false;

        // M move: Middle slice - Up[1,4,7] + Down[1,4,7] + Front[1,4,7] + Back[1,4,7]
        case Move::M:
        case Move::MP:
            if (stickerIndex == 1 || stickerIndex == 4 || stickerIndex == 7) {
                return faceIndex == 4 || faceIndex == 5 || faceIndex == 0 || faceIndex == 1;  // U, D, F, B
            }
            return false;

        // E move: Equator slice - Front[3,4,5] + Back[3,4,5] + Left[3,4,5] + Right[3,4,5]
        case Move::E:
        case Move::EP:
            if (stickerIndex >= 3 && stickerIndex <= 5) {
                return faceIndex == 0 || faceIndex == 1 || faceIndex == 2 || faceIndex == 3;  // F, B, L, R
            }
            return false;

        // S move: Standing slice - Up[3,4,5] + Down[3,4,5] + Left[1,4,7] + Right[1,4,7]
        case Move::S:
        case Move::SP:
            if (faceIndex == 4 || faceIndex == 5) {  // U or D
                return stickerIndex >= 3 && stickerIndex <= 5;  // Middle row
            }
            if (faceIndex == 2 || faceIndex == 3) {  // L or R
                return stickerIndex == 1 || stickerIndex == 4 || stickerIndex == 7;  // Middle column
            }
            return false;

        default:
            return false;
    }
}

// Apply rotation transformation to a 3D position based on the current move
std::array<float, 3> CubeRenderer::rotateSticker(const std::array<float, 3>& pos, Move move, float angle) const {
    // Convert angle to radians
    float rad = angle * M_PI / 180.0f;
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    float x = pos[0];
    float y = pos[1];
    float z = pos[2];

    switch (move) {
        // U moves rotate around Y axis (looking down)
        case Move::U:
            return {x * cosA - z * sinA, y, x * sinA + z * cosA};
        case Move::UP:
            return {x * cosA + z * sinA, y, -x * sinA + z * cosA};

        // D moves rotate around Y axis (looking up - opposite direction of U)
        case Move::D:
            return {x * cosA + z * sinA, y, -x * sinA + z * cosA};
        case Move::DP:
            return {x * cosA - z * sinA, y, x * sinA + z * cosA};

        // L moves rotate around X axis (looking from left)
        case Move::L:
            return {x, y * cosA - z * sinA, y * sinA + z * cosA};
        case Move::LP:
            return {x, y * cosA + z * sinA, -y * sinA + z * cosA};

        // R moves rotate around X axis (looking from right - opposite direction of L)
        case Move::R:
            return {x, y * cosA + z * sinA, -y * sinA + z * cosA};
        case Move::RP:
            return {x, y * cosA - z * sinA, y * sinA + z * cosA};

        // F moves rotate around Z axis (looking from front)
        case Move::F:
            return {x * cosA + y * sinA, -x * sinA + y * cosA, z};
        case Move::FP:
            return {x * cosA - y * sinA, x * sinA + y * cosA, z};

        // B moves rotate around Z axis (looking from back - opposite direction of F)
        case Move::B:
            return {x * cosA - y * sinA, x * sinA + y * cosA, z};
        case Move::BP:
            return {x * cosA + y * sinA, -x * sinA + y * cosA, z};

        // M moves rotate around X axis (same as L)
        case Move::M:
            return {x, y * cosA - z * sinA, y * sinA + z * cosA};
        case Move::MP:
            return {x, y * cosA + z * sinA, -y * sinA + z * cosA};

        // E moves rotate around Y axis (same as U, looking down)
        case Move::E:
            return {x * cosA - z * sinA, y, x * sinA + z * cosA};
        case Move::EP:
            return {x * cosA + z * sinA, y, -x * sinA + z * cosA};

        // S moves rotate around Z axis (same as F)
        case Move::S:
            return {x * cosA + y * sinA, -x * sinA + y * cosA, z};
        case Move::SP:
            return {x * cosA - y * sinA, x * sinA + y * cosA, z};

        default:
            return pos;
    }
}
// Helper function to draw a single cube with 6 different face colors
void CubeRenderer::drawCube(int cubeIndex, bool usePreAnimationState) {
    // Select which cube state to use
    const RubiksCube& cube = usePreAnimationState ? preAnimationCube_ : cube_;

    // Calculate position in 3x3x3 grid
    int layer = cubeIndex / 9;   // 0=front/middle, 1=middle, 2=back
    int posInLayer = cubeIndex % 9;
    int row = posInLayer / 3;   // 0=bottom, 1=middle, 2=top
    int col = posInLayer % 3;   // 0=left, 1=middle, 2=right

    // Coordinate mapping:
    // layer 0 -> z=-1 (Back), layer 2 -> z=+1 (Front)
    // row 0 -> y=-1 (Down), row 2 -> y=+1 (Up)
    // col 0 -> x=-1 (Left), col 2 -> x=+1 (Right)

    // Draw all faces as black by default
    float black[3] = {0.0f, 0.0f, 0.0f};

    // Front face (z=+1) - only for layer 2
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    if (layer == 2) {
        const auto& face = cube.getFront();
        // Vertex order: TL(0,0), TR(0,2), BR(2,2), BL(2,0)
        // Row is inverted to match U/D move behavior
        int idx = (2 - row) * 3 + col;
        auto rgb = getFaceColorRgb(face[idx]);
        glColor3f(rgb[0], rgb[1], rgb[2]);
        glVertex3f(-0.5f, 0.5f, 0.5f);  // vertex 0: top-left
        glVertex3f(0.5f, 0.5f, 0.5f);   // vertex 1: top-right
        glVertex3f(0.5f, -0.5f, 0.5f);  // vertex 2: bottom-right
        glVertex3f(-0.5f, -0.5f, 0.5f); // vertex 3: bottom-left
        glEnd();
    } else {
        glColor3fv(black);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glEnd();
    }

    // Back face (z=-1) - only for layer 0
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    if (layer == 0) {
        const auto& face = cube_.getBack();
        // Vertex order: TR(0,2), TL(0,0), BL(2,0), BR(2,2)
        // Back face in 3D is viewed from behind, so left-right is reversed
        // Row is inverted to match U/D move behavior
        // Grid: 0 1 2 / 3 4 5 / 6 7 8 (from 2D view perspective)
        // 3D view: col 0 (left visually) = grid col 2, col 2 (right visually) = grid col 0
        int idx = (2 - row) * 3 + (2 - col);
        auto rgb = getFaceColorRgb(face[idx]);
        glColor3f(rgb[0], rgb[1], rgb[2]);
        glVertex3f(0.5f, 0.5f, -0.5f);  // vertex 0: top-right
        glVertex3f(-0.5f, 0.5f, -0.5f); // vertex 1: top-left
        glVertex3f(-0.5f, -0.5f, -0.5f); // vertex 2: bottom-left
        glVertex3f(0.5f, -0.5f, -0.5f);  // vertex 3: bottom-right
        glEnd();
    } else {
        glColor3fv(black);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glEnd();
    }

    // Top face (y=+1) - only for row 2
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    if (row == 2) {
        const auto& face = cube_.getUp();
        // Up face grid: layer=0 is top (back), layer=2 is bottom (front)
        int idx = layer * 3 + col;
        auto rgb = getFaceColorRgb(face[idx]);
        glColor3f(rgb[0], rgb[1], rgb[2]);
        glVertex3f(-0.5f, 0.5f, -0.5f); // vertex 0: top-back-left
        glVertex3f(0.5f, 0.5f, -0.5f);  // vertex 1: top-back-right
        glVertex3f(0.5f, 0.5f, 0.5f);   // vertex 2: top-front-right
        glVertex3f(-0.5f, 0.5f, 0.5f);  // vertex 3: top-front-left
        glEnd();
    } else {
        glColor3fv(black);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glEnd();
    }

    // Bottom face (y=-1) - only for row 0
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    if (row == 0) {
        const auto& face = cube_.getDown();
        // Down face grid: layer=2 is top (front), layer=0 is bottom (back)
        int idx = layer * 3 + col;
        auto rgb = getFaceColorRgb(face[idx]);
        glColor3f(rgb[0], rgb[1], rgb[2]);
        glVertex3f(-0.5f, -0.5f, 0.5f);  // vertex 0: bottom-front-left
        glVertex3f(0.5f, -0.5f, 0.5f);   // vertex 1: bottom-front-right
        glVertex3f(0.5f, -0.5f, -0.5f);  // vertex 2: bottom-back-right
        glVertex3f(-0.5f, -0.5f, -0.5f); // vertex 3: bottom-back-left
        glEnd();
    } else {
        glColor3fv(black);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glEnd();
    }

    // Right face (x=+1) - only for col 2
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    if (col == 2) {
        const auto& face = cube_.getRight();
        // Right face grid: row=2 is top, layer=2 is front
        // Layer is inverted to match F/B move behavior
        int idx = (2 - row) * 3 + (2 - layer);
        auto rgb = getFaceColorRgb(face[idx]);
        glColor3f(rgb[0], rgb[1], rgb[2]);
        glVertex3f(0.5f, 0.5f, 0.5f);   // vertex 0: right-top-front
        glVertex3f(0.5f, -0.5f, 0.5f);  // vertex 1: right-bottom-front
        glVertex3f(0.5f, -0.5f, -0.5f); // vertex 2: right-bottom-back
        glVertex3f(0.5f, 0.5f, -0.5f);  // vertex 3: right-top-back
        glEnd();
    } else {
        glColor3fv(black);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glEnd();
    }

    // Left face (x=-1) - only for col 0
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    if (col == 0) {
        const auto& face = cube_.getLeft();
        // Left face grid: row=2 is top, layer=0 is back
        // Layer is inverted to match F/B move behavior
        int idx = (2 - row) * 3 + (2 - layer);
        auto rgb = getFaceColorRgb(face[idx]);
        glColor3f(rgb[0], rgb[1], rgb[2]);
        glVertex3f(-0.5f, 0.5f, -0.5f); // vertex 0: left-top-back
        glVertex3f(-0.5f, -0.5f, -0.5f); // vertex 1: left-bottom-back
        glVertex3f(-0.5f, -0.5f, 0.5f);  // vertex 2: left-bottom-front
        glVertex3f(-0.5f, 0.5f, 0.5f);  // vertex 3: left-top-front
        glEnd();
    } else {
        glColor3fv(black);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glEnd();
    }
}

// Smooth interpolation for view rotation considering 360 degree wraparound
void CubeRenderer::lerpRotation(float& current, float target, float deltaTime) {
    float diff = target - current;

    // Handle angle wraparound (e.g., going from 350° to 10° should be +20°, not -340°)
    while (diff > 180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;

    // Linear interpolation with speed factor
    float speed = viewRotationSpeed * deltaTime;
    current += diff * speed;

    // Snap to target if very close
    if (fabsf(diff) < 0.1f) {
        current = target;
    }
}

// Check if a small cube is in the rotating slice for current move
bool CubeRenderer::isCubeAnimating(int cubeIndex) const {
    if (!isAnimating_) return false;

    // Calculate position in 3x3x3 grid
    // Note: In 3D model: layer 0=Back, 1=Middle, 2=Front
    int layer = cubeIndex / 9;
    int posInLayer = cubeIndex % 9;
    int row = posInLayer / 3;   // 0=bottom, 1=middle, 2=top
    int col = posInLayer % 3;   // 0=left, 1=middle, 2=right

    switch (currentMove_) {
        // U move: All cubes at row 2 (top layer)
        case Move::U:
        case Move::UP:
            return (row == 2);

        // D move: All cubes at row 0 (bottom layer)
        case Move::D:
        case Move::DP:
            return (row == 0);

        // L move: All cubes at col 0 (left slice)
        case Move::L:
        case Move::LP:
            return (col == 0);

        // R move: All cubes at col 2 (right slice)
        case Move::R:
        case Move::RP:
            return (col == 2);

        // F move: Front face (layer 2) only
        case Move::F:
        case Move::FP:
            return (layer == 2);

        // B move: Back face (layer 0) only
        case Move::B:
        case Move::BP:
            return (layer == 0);

        // M move: middle column (col 1, between L and R)
        case Move::M:
        case Move::MP:
            return (col == 1);

        // E move: middle row (row 1)
        case Move::E:
        case Move::EP:
            return (row == 1);

        // S move: middle layer (layer 1, between F and B)
        case Move::S:
        case Move::SP:
            return (layer == 1);

        default:
            return false;
    }
}

// Apply rotation transformation to the current cube based on move type
void CubeRenderer::applyRotationTransform(float angle, Move move) {
    switch (move) {
        // U moves rotate around Y axis
        case Move::U:
            glRotatef(angle, 0.0f, -1.0f, 0.0f);
            break;
        case Move::UP:
            glRotatef(angle, 0.0f, 1.0f, 0.0f);
            break;

        // D moves rotate around Y axis (opposite of U)
        case Move::D:
            glRotatef(angle, 0.0f, 1.0f, 0.0f);
            break;
        case Move::DP:
            glRotatef(angle, 0.0f, -1.0f, 0.0f);
            break;

        // L moves rotate around X axis
        case Move::L:
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
            break;
        case Move::LP:
            glRotatef(angle, -1.0f, 0.0f, 0.0f);
            break;

        // R moves rotate around X axis (opposite of L)
        case Move::R:
            glRotatef(angle, -1.0f, 0.0f, 0.0f);
            break;
        case Move::RP:
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
            break;

        // F moves rotate around Z axis
        case Move::F:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;
        case Move::FP:
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
            break;

        // B moves rotate around Z axis (opposite of F)
        case Move::B:
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
            break;
        case Move::BP:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;

        // M moves rotate around X axis (same as L)
        case Move::M:
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
            break;
        case Move::MP:
            glRotatef(angle, -1.0f, 0.0f, 0.0f);
            break;

        // E moves rotate around Y axis (opposite direction of U when viewed from top)
        case Move::E:
            glRotatef(-angle, 0.0f, -1.0f, 0.0f);
            break;
        case Move::EP:
            glRotatef(-angle, 0.0f, 1.0f, 0.0f);
            break;

        // S moves rotate around Z axis (same as F)
        case Move::S:
            glRotatef(angle, 0.0f, 0.0f, -1.0f);
            break;
        case Move::SP:
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
            break;

        default:
            break;
    }
}
