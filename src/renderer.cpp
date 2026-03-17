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

void CubeRenderer::render3DOverlay(int windowWidth, int windowHeight) {
    renderer3D_.render(windowWidth, windowHeight);
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

bool CubeRenderer::isStickerAnimating(Move move, Face faceIndex, int stickerIndex) const {
    switch (move) {
        case Move::U:
        case Move::UP:
        case Move::U2:
            if (faceIndex == Face::UP) return true;
            if (stickerIndex >= 0 && stickerIndex <= 2) {
                return faceIndex == Face::FRONT || faceIndex == Face::LEFT || faceIndex == Face::BACK || faceIndex == Face::RIGHT;
            }
            return false;

        case Move::D:
        case Move::DP:
        case Move::D2:
            if (faceIndex == Face::DOWN) return true;
            if (stickerIndex >= 6 && stickerIndex <= 8) {
                return faceIndex == Face::FRONT || faceIndex == Face::LEFT || faceIndex == Face::BACK || faceIndex == Face::RIGHT;
            }
            return false;

        case Move::L:
        case Move::LP:
        case Move::L2:
            if (faceIndex == Face::LEFT) return true;
            if (stickerIndex == 0 || stickerIndex == 3 || stickerIndex == 6) {
                return faceIndex == Face::UP || faceIndex == Face::FRONT || faceIndex == Face::DOWN || faceIndex == Face::BACK;
            }
            return false;

        case Move::R:
        case Move::RP:
        case Move::R2:
            if (faceIndex == Face::RIGHT) return true;
            if (stickerIndex == 2 || stickerIndex == 5 || stickerIndex == 8) {
                return faceIndex == Face::UP || faceIndex == Face::FRONT || faceIndex == Face::DOWN || faceIndex == Face::BACK;
            }
            return false;

        case Move::F:
        case Move::FP:
        case Move::F2:
            if (faceIndex == Face::FRONT) return true;
            if (faceIndex == Face::UP && stickerIndex >= 6 && stickerIndex <= 8) return true;
            if (faceIndex == Face::LEFT && (stickerIndex == 2 || stickerIndex == 5 || stickerIndex == 8)) return true;
            if (faceIndex == Face::DOWN && stickerIndex >= 0 && stickerIndex <= 2) return true;
            if (faceIndex == Face::RIGHT && (stickerIndex == 0 || stickerIndex == 3 || stickerIndex == 6)) return true;
            return false;

        case Move::B:
        case Move::BP:
        case Move::B2:
            if (faceIndex == Face::BACK) return true;
            if (faceIndex == Face::UP && stickerIndex >= 0 && stickerIndex <= 2) return true;
            if (faceIndex == Face::LEFT && (stickerIndex == 0 || stickerIndex == 3 || stickerIndex == 6)) return true;
            if (faceIndex == Face::DOWN && stickerIndex >= 6 && stickerIndex <= 8) return true;
            if (faceIndex == Face::RIGHT && (stickerIndex == 2 || stickerIndex == 5 || stickerIndex == 8)) return true;
            return false;

        case Move::M:
        case Move::MP:
        case Move::M2:
            if (stickerIndex == 1 || stickerIndex == 4 || stickerIndex == 7) {
                return faceIndex == Face::UP || faceIndex == Face::DOWN || faceIndex == Face::FRONT || faceIndex == Face::BACK;
            }
            return false;

        case Move::E:
        case Move::EP:
        case Move::E2:
            if (stickerIndex >= 3 && stickerIndex <= 5) {
                return faceIndex == Face::FRONT || faceIndex == Face::BACK || faceIndex == Face::LEFT || faceIndex == Face::RIGHT;
            }
            return false;

        case Move::S:
        case Move::SP:
        case Move::S2:
            if (faceIndex == Face::UP || faceIndex == Face::DOWN) {
                return stickerIndex >= 3 && stickerIndex <= 5;
            }
            if (faceIndex == Face::LEFT || faceIndex == Face::RIGHT) {
                return stickerIndex == 1 || stickerIndex == 4 || stickerIndex == 7;
            }
            return false;

        case Move::X:
        case Move::XP:
        case Move::X2:
            return true;

        case Move::Y:
        case Move::YP:
        case Move::Y2:
            return true;

        case Move::Z:
        case Move::ZP:
        case Move::Z2:
            return true;

        default:
            return false;
    }
}

std::array<float, 3> CubeRenderer::rotateSticker(const std::array<float, 3>& pos, Move move, float angle) const {
    RotationAxis axis = getRotationAxis(move);
    return rotateAroundAxis(pos, axis, angle);
}
