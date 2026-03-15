#include "view_state.h"
#include <cmath>

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
