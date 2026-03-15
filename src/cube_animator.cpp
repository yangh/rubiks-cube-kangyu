#include "cube_animator.h"
#include <cmath>
#include <iostream>

extern bool g_enableDump;

const char* getEasingTypeName(EasingType type) {
    switch (type) {
        case EasingType::SmoothStep:    return "Smoothstep";
        case EasingType::EaseOutCubic:  return "Ease-Out Cubic";
        case EasingType::EaseOutBack:   return "Ease-Out Back";
        default: return "Unknown";
    }
}

static float applyEasing(EasingType type, float t) {
    switch (type) {
        case EasingType::SmoothStep:
            return t * t * (3.0f - 2.0f * t);

        case EasingType::EaseOutCubic:
            return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);

        case EasingType::EaseOutBack: {
            // 1.70158f for 10% overshot;
            // 1.00000f for  5% overshot;
            // 0.50000f for  2% overshot;
            const float c1 = 1.0f;
            const float c3 = c1 + 1.0f;
            return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
        }

        default:
            return t;
    }
}

CubeAnimator::CubeAnimator() 
    : preAnimationCube_() {
}

void CubeAnimator::queueMove(Move move, bool recordHistory) {
    moveQueue_.push(move);
    recordCurrentMoveHistory_ = recordHistory;
    
    if (!isAnimating_ && moveQueue_.size() == 1) {
        startNextAnimation();
    }
    
    if (g_enableDump) {
        std::cout << "\n=== Queued " << moveToString(move) << " ===" << std::endl;
    }
}

void CubeAnimator::update(float deltaTime) {
    if (isAnimating_) {
        const float ANIMATION_DURATION = 0.2f / animationSpeed;
        animationProgress_ += deltaTime / ANIMATION_DURATION;
        
        if (animationProgress_ >= 1.0f) {
            animationProgress_ = 1.0f;
            isAnimating_ = false;
            
            if (moveCompleteCallback_) {
                moveCompleteCallback_(currentMove_, recordCurrentMoveHistory_);
            }
            
            if (g_enableDump) {
                std::cout << "\n=== Completed " << moveToString(currentMove_) << " ===" << std::endl;
            }
            
            startNextAnimation();
        }
    }
}

void CubeAnimator::reset() {
    isAnimating_ = false;
    animationProgress_ = 0.0f;
    while (!moveQueue_.empty()) {
        moveQueue_.pop();
    }
}

bool CubeAnimator::isCubeInAnimatingSlice(int cubeIndex) const {
    if (!isAnimating_) return false;
    
    int layer = cubeIndex / 9;
    int posInLayer = cubeIndex % 9;
    int row = posInLayer / 3;
    int col = posInLayer % 3;
    
    switch (currentMove_) {
        case Move::U:
        case Move::UP:
        case Move::U2:
            return (row == 2);
            
        case Move::D:
        case Move::DP:
        case Move::D2:
            return (row == 0);
            
        case Move::L:
        case Move::LP:
        case Move::L2:
            return (col == 0);
            
        case Move::R:
        case Move::RP:
        case Move::R2:
            return (col == 2);
            
        case Move::F:
        case Move::FP:
        case Move::F2:
            return (layer == 2);
            
        case Move::B:
        case Move::BP:
        case Move::B2:
            return (layer == 0);
            
        case Move::M:
        case Move::MP:
        case Move::M2:
            return (col == 1);
            
        case Move::E:
        case Move::EP:
        case Move::E2:
            return (row == 1);
            
        case Move::S:
        case Move::SP:
        case Move::S2:
            return (layer == 1);
            
        case Move::X:
        case Move::XP:
        case Move::X2:
        case Move::Y:
        case Move::YP:
        case Move::Y2:
        case Move::Z:
        case Move::ZP:
        case Move::Z2:
            return true;
            
        default:
            return false;
    }
}

float CubeAnimator::getCurrentAngle() const {
    if (!isAnimating_) return 0.0f;
    
    float easeProgress = applyEasing(easingType, animationProgress_);
    return rotationAngle_ * easeProgress;
}

void CubeAnimator::setMoveCompleteCallback(MoveCallback callback) {
    moveCompleteCallback_ = callback;
}

void CubeAnimator::startNextAnimation() {
    if (moveQueue_.empty()) {
        isAnimating_ = false;
        return;
    }
    
    if (!enableAnimation) {
        while (!moveQueue_.empty()) {
            Move move = moveQueue_.front();
            moveQueue_.pop();
            if (moveCompleteCallback_) {
                moveCompleteCallback_(move, recordCurrentMoveHistory_);
            }
            if (g_enableDump) {
                std::cout << "\n=== Completed " << moveToString(move) << " ===" << std::endl;
            }
        }
        isAnimating_ = false;
        return;
    }
    
    isAnimating_ = true;
    animationProgress_ = 0.0f;
    currentMove_ = moveQueue_.front();
    moveQueue_.pop();
    if (cubeGetter_) {
        preAnimationCube_ = cubeGetter_();
    }
    rotationAngle_ = isDoubleMove(currentMove_) ? 180.0f : 90.0f;
    
    if (g_enableDump) {
        std::cout << "\n=== Starting " << moveToString(currentMove_) << " ===" << std::endl;
    }
}

bool CubeAnimator::isDoubleMove(Move move) const {
    switch (move) {
        case Move::U2: case Move::D2: case Move::L2: case Move::R2:
        case Move::F2: case Move::B2: case Move::M2: case Move::E2: case Move::S2:
        case Move::X2: case Move::Y2: case Move::Z2:
            return true;
        default:
            return false;
    }
}
