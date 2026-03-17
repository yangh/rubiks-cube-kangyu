#include "animator.h"
#include "move.h"
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
        std::cout << "\n=== Queued " << moveToStringFast(move) << " ===" << std::endl;
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
                std::cout << "\n=== Completed " << moveToStringFast(currentMove_) << " ===" << std::endl;
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
    return MoveLookup::isInSlice(cubeIndex, getAnimationSlice(currentMove_));
}

float CubeAnimator::getCurrentAngle() const {
    if (!isAnimating_) return 0.0f;
    
    float easeProgress = applyEasing(easingType, animationProgress_);
    float baseAngle = isDoubleMove(currentMove_) ? 180.0f : 90.0f;
    return baseAngle * easeProgress;
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
                std::cout << "\n=== Completed " << moveToStringFast(move) << " ===" << std::endl;
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
    
    if (g_enableDump) {
        std::cout << "\n=== Starting " << moveToStringFast(currentMove_) << " ===" << std::endl;
    }
}
