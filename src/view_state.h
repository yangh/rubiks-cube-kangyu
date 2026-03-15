#ifndef VIEW_STATE_H
#define VIEW_STATE_H

struct ViewState {
    float rotationX = 30.0f;
    float rotationY = -30.0f;
    float rotationZ = 0.0f;
    float targetRotationX = 30.0f;
    float targetRotationY = -30.0f;
    float targetRotationZ = 0.0f;
    float viewRotationSpeed = 8.0f;
    
    float scale3D = 3.1f;
    float scale2D = 0.8f;
    
    bool celebrationMode = false;
    
    void lerpRotation(float& current, float target, float deltaTime);
    void reset();
};

#endif // VIEW_STATE_H
