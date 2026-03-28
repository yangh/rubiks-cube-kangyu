#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "cube.h"
#include <functional>
#include <queue>

enum class EasingType {
    SmoothStep = 0,
    EaseOutCubic = 1,
    EaseOutBack = 2
};

const char* getEasingTypeName(EasingType type);

struct PendingMove {
    Move move;
    bool recordHistory;
};

class CubeAnimator {
public:
    using MoveCallback = std::function<void(Move, bool)>;
    using CubeGetter = std::function<const RubiksCube&()>;

    CubeAnimator();

    void queueMove(Move move, bool recordHistory = true);
    void update(float deltaTime);
    void reset();

    bool isAnimating() const { return isAnimating_; }
    float progress() const { return animationProgress_; }
    Move currentMove() const { return currentMove_.move; }
    bool isCubeInAnimatingSlice(int cubeIndex) const;

    EasingType easingType = EasingType::SmoothStep;

    float animationSpeed = 1.0f;
    bool enableAnimation = true;
    bool enableDump = false;

    const RubiksCube& getPreAnimationCube() const { return preAnimationCube_; }
    float getCurrentAngle() const;

    void setMoveCompleteCallback(MoveCallback callback);
    void setCubeGetter(CubeGetter getter) { cubeGetter_ = getter; }

private:
    bool isAnimating_ = false;
    float animationProgress_ = 0.0f;
    PendingMove currentMove_{Move::U, true};
    std::queue<PendingMove> moveQueue_;
    RubiksCube preAnimationCube_;
    MoveCallback moveCompleteCallback_;
    CubeGetter cubeGetter_;

    void startNextAnimation();
};

#endif // ANIMATOR_H
