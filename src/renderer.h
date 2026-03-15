#ifndef RENDERER_H
#define RENDERER_H

#include "cube.h"
#include "config.h"
#include <imgui.h>
#include <queue>
#include "model.h"

class CubeRenderer {
public:
    CubeRenderer();
    ~CubeRenderer() = default;

    // Render 2D unfolded cube net view
    void draw2D(ImDrawList* drawList, ImVec2 offset, float scale);

    // Render 3D isometric view (called during ImGui window building)
    void draw3D(ImDrawList* drawList, ImVec2 offset, float scale);

    // Render 3D overlay (called after ImGui rendering)
    void render3DOverlay(int windowWidth, int windowHeight);

    void executeMove(Move move);
    void executeMove(Move move, bool recordHistory);  // For animated undo/redo
    void updateAnimation(float deltaTime);
    void reset();

    // Apply color configuration to the renderer
    void setCustomColors(const ColorConfig& config);
    void undo();  // Undo the last move
    void redo();  // Redo the last undone move
    std::vector<Move> scramble(int numMoves = 20);  // Scramble the cube
    bool isAnimating() const { return isAnimating_; }
    float animationProgress() const { return animationProgress_; }
    void resetView();  // Reset 3D view parameters to defaults
    bool isSolved() const;
    void dump() const;  // Dump cube state to console
    const std::vector<Move>& getMoveHistory() const { return cube_.getMoveHistory(); }
    const std::vector<Move>& getRedoHistory() const { return cube_.getRedoHistory(); }
    bool canRedo() const { return cube_.canRedo(); }

    float rotationX = 30.0f;
    float rotationY = -30.0f;
    float rotationZ = 0.0f;

    // Target rotation angles for smooth view rotation animation
    float targetRotationX = 30.0f;
    float targetRotationY = -30.0f;
    float targetRotationZ = 0.0f;
    float viewRotationSpeed = 8.0f;  // Speed factor for view rotation interpolation
    float scale = 3.1f;
    float scale2D = 0.8f;  // 2D view zoom level
    float animationSpeed = 1.0f;  // Animation speed multiplier (1.0 = normal)
    bool enableAnimation = true;  // Enable/disable animation

    // Custom color overrides for each face (public for UI access)
    std::array<float, 3> customFront = {0.0f, 1.0f, 0.0f};
    std::array<float, 3> customBack = {0.0f, 0.0f, 1.0f};
    std::array<float, 3> customLeft = {1.0f, 0.5f, 0.0f};
    std::array<float, 3> customRight = {1.0f, 0.0f, 0.0f};
    std::array<float, 3> customUp = {1.0f, 1.0f, 1.0f};
    std::array<float, 3> customDown = {1.0f, 1.0f, 0.0f};
    bool useCustomColors = false;

private:
    // OpenGL 3D rendering
    Model* cubeModel;
    bool initGL3D();

    // Helper function to draw a single cube
    void drawCube(int cubeIndex, bool usePreAnimationState = false);

    // Draw a circular canvas beneath the cube
    void drawCircleCanvas();

    RubiksCube cube_;

    // Animation state
    bool isAnimating_ = false;
    float animationProgress_ = 0.0f;  // 0.0 to 1.0
    Move currentMove_ = Move::U;
    RubiksCube preAnimationCube_;  // Cube state before animation
    std::queue<Move> moveQueue_;
    bool recordCurrentMoveHistory_ = true;  // Whether current animation should record history
    float rotationAngle_ = 90.0f;  // Animation angle: 90° for single moves, 180° for double moves

    // Project 3D point to 2D screen coordinates
    ImVec2 project(float x, float y, float z, ImVec2 center, float scale);

    // Draw a 2D face (unfolded cube net view)
    // flipVertical: flip the face vertically (for Back face in unfolded view)
    void drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                 ImVec2 offset, float size, float gap, bool flipVertical = false, Color faceType = Color::WHITE);

    // Draw a 3D face
    void draw3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                    const ImVec2 (&vertices)[4], ImVec2 center, float scale);

    // Draw a 3D face with animation transformation applied
    void drawAnimated3DFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                            const ImVec2 (&faceVerts)[4], ImVec2 center, float size,
                            Face faceIndex);

    // Get face color for 3D drawing
    ImU32 getFaceColor(Color color);

    // Get face color as RGB array for OpenGL rendering
    std::array<float, 3> getFaceColorRgb(Color color);

    // Test: draw a single simple cube
    void drawTestCube(ImDrawList* drawList, ImVec2 center, float size);

    // Step 2: draw a cube with one colored face
    void drawTestCubeWithColor(ImDrawList* drawList, ImVec2 center, float size, ImU32 faceColor);

    // Animation helpers
    void startNextAnimation();
    bool isStickerAnimating(Move move, Face faceIndex, int stickerIndex) const;
    std::array<float, 3> rotateSticker(const std::array<float, 3>& pos, Move move, float angle) const;
    bool isDoubleMove(Move move) const;

    // View rotation animation helper - smooth interpolation considering 360 degree wraparound
    void lerpRotation(float& current, float target, float deltaTime);

    // 3D cube rotation animation helpers
    bool isCubeAnimating(int cubeIndex) const;  // Check if a small cube is in the rotating slice
    void applyRotationTransform(float angle, Move move);  // Apply rotation transformation to current cube
};

#endif // RENDERER_H
