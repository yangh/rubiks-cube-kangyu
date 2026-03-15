#ifndef VIEW_STATE_H
#define VIEW_STATE_H

struct ViewState {
    // 3D 视角旋转
    float rotationX = 30.0f;
    float rotationY = -30.0f;
    float rotationZ = 0.0f;
    float targetRotationX = 30.0f;
    float targetRotationY = -30.0f;
    float targetRotationZ = 0.0f;
    float viewRotationSpeed = 8.0f;
    
    // 缩放
    float scale3D = 3.1f;
    float scale2D = 0.8f;
    
    // 平滑插值
    void lerpRotation(float& current, float target, float deltaTime);
    void reset();
};

#endif // VIEW_STATE_H
