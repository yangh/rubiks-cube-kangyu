#ifndef CUBE_ANIMATOR_H
#define CUBE_ANIMATOR_H

#include "cube.h"
#include <functional>
#include <queue>

// Easing function types for animation
enum class EasingType {
    SmoothStep = 0,
    EaseOutCubic = 1,
    EaseOutBack = 2
};

// Get easing type name for display
const char* getEasingTypeName(EasingType type);

class CubeAnimator {
public:
    using MoveCallback = std::function<void(Move, bool)>;
    using CubeGetter = std::function<const RubiksCube&()>;
    
    CubeAnimator();
    
    // Animation control
    void queueMove(Move move, bool recordHistory = true);
    void update(float deltaTime);
    void reset();
    
    // State query
    bool isAnimating() const { return isAnimating_; }
    float progress() const { return animationProgress_; }
    Move currentMove() const { return currentMove_; }
    bool isCubeInAnimatingSlice(int cubeIndex) const;
    
    EasingType easingType = EasingType::SmoothStep;

    float animationSpeed = 1.0f;
    bool enableAnimation = true;
    
    // Animation state snapshot (for renderer use)
    const RubiksCube& getPreAnimationCube() const { return preAnimationCube_; }
    float getCurrentAngle() const;  // Current animation angle
    
    // Set callback (called when animation completes)
    void setMoveCompleteCallback(MoveCallback callback);
    void setCubeGetter(CubeGetter getter) { cubeGetter_ = getter; }
    
private:
    bool isAnimating_ = false;
    float animationProgress_ = 0.0f;
    Move currentMove_ = Move::U;
    float rotationAngle_ = 90.0f;
    std::queue<Move> moveQueue_;
    RubiksCube preAnimationCube_;
    bool recordCurrentMoveHistory_ = true;
    MoveCallback moveCompleteCallback_;
    CubeGetter cubeGetter_;
    
    void startNextAnimation();
    bool isDoubleMove(Move move) const;
};

#endif // CUBE_ANIMATOR_H
