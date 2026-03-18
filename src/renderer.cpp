#include "renderer.h"
#include "config.h"
#include "move.h"
#include <cmath>
#include <iostream>

extern bool g_enableDump;

void ViewState::lerpRotation(float& current, float target, float deltaTime) {
    float diff = target - current;

    // Handle angle wraparound (e.g., going from 350 to 10 should be +20, not -340)
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

void ViewState::reset() {
    rotationX = 30.0f;
    rotationY = -30.0f;
    rotationZ = 0.0f;
    targetRotationX = 30.0f;
    targetRotationY = -30.0f;
    targetRotationZ = 0.0f;
    scale3D = 3.1f;
    scale2D = 0.8f;
}

static std::array<float, 3> rotateAroundAxis(const std::array<float, 3>& pos, 
                                              const RotationAxis& axis, float angle) {
    float rad = angle * M_PI / 180.0f;
    float c = cosf(rad);
    float s = sinf(rad);
    float x = pos[0], y = pos[1], z = pos[2];
    
    if (axis.x != 0) {
        float sign = axis.x > 0 ? 1.0f : -1.0f;
        return {x, y * c - sign * z * s, sign * y * s + z * c};
    } else if (axis.y != 0) {
        float sign = axis.y > 0 ? 1.0f : -1.0f;
        return {x * c + sign * z * s, y, -sign * x * s + z * c};
    } else {
        float sign = axis.z > 0 ? 1.0f : -1.0f;
        return {x * c - sign * y * s, sign * x * s + y * c, z};
    }
}

CubeRenderer::CubeRenderer(RubiksCube& cube)
    : cube_(cube), animator_()
{
    animator_.setMoveCompleteCallback([this](Move move, bool recordHistory) {
        cube_.executeMove(move, recordHistory);
        if (g_enableDump) {
            cube_.dump();
        }
    });
    animator_.setCubeGetter([this]() -> const RubiksCube& {
        return cube_;
    });
    
    renderer3D_.setViewState(&viewState_);
    renderer3D_.setColorProvider(&colorProvider_);
    renderer3D_.setAnimator(&animator_);
    renderer3D_.setCube(&cube_);
}

void CubeRenderer::setCustomColors(const ColorConfig& config) {
    colorProvider_.setCustomColors(config);
}

void CubeRenderer::executeMove(Move move) {
    executeMove(move, true);
}

void CubeRenderer::executeMove(Move move, bool recordHistory) {
    animator_.queueMove(move, recordHistory);

    if (g_enableDump) {
        std::cout << "\n=== Queued " << moveToString(move) << " ===" << std::endl;
    }
}

void CubeRenderer::reset() {
    cube_.reset();
    animator_.reset();
}

void CubeRenderer::undo() {
    if (cube_.getMoveHistory().empty()) {
        return;
    }

    Move lastMove = cube_.getMoveHistory().back();
    Move inverseMove = getInverseMove(lastMove);

    executeMove(inverseMove, false);

    cube_.popMoveHistory();
    cube_.pushToRedoHistory(lastMove);
}

void CubeRenderer::redo() {
    if (cube_.getRedoHistory().empty()) {
        return;
    }

    Move moveToRedo = cube_.getRedoHistory().back();

    executeMove(moveToRedo, false);

    cube_.popRedoHistory();
    cube_.pushToMoveHistory(moveToRedo);
}

std::vector<Move> CubeRenderer::scramble(int numMoves) {
    std::vector<Move> scrambleMoves = cube_.scramble(numMoves);
    animator_.reset();
    return scrambleMoves;
}

void CubeRenderer::resetView() {
    viewState_.reset();
}

bool CubeRenderer::isSolved() const {
    return cube_.isSolved();
}

void CubeRenderer::dump() const {
    cube_.dump();
}

void CubeRenderer::draw2D(ImDrawList* drawList, ImVec2 offset, float scale) {
    renderer2D_.draw(drawList, offset, scale, cube_, colorProvider_);
}

void CubeRenderer::render3DOverlay(int windowWidth, int windowHeight, float sidebarWidth) {
    renderer3D_.render(windowWidth, windowHeight, sidebarWidth);
}

void CubeRenderer::updateAnimation(float deltaTime) {
    animator_.update(deltaTime);

    if (viewState_.celebrationMode) {
        viewState_.rotationX += deltaTime * 25.0f;
        viewState_.rotationY += deltaTime * 30.0f;
        viewState_.targetRotationX = viewState_.rotationX;
        viewState_.targetRotationY = viewState_.rotationY;
        if (viewState_.rotationX >= 360.0f) viewState_.rotationX -= 360.0f;
        if (viewState_.rotationY >= 360.0f) viewState_.rotationY -= 360.0f;
    } else {
        viewState_.lerpRotation(viewState_.rotationX, viewState_.targetRotationX, deltaTime);
        viewState_.lerpRotation(viewState_.rotationY, viewState_.targetRotationY, deltaTime);
        viewState_.lerpRotation(viewState_.rotationZ, viewState_.targetRotationZ, deltaTime);
    }
}

