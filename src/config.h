#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "color.h"

enum class RendererType {
    OpenGL = 0,
    Shader = 1
};

class CubeConfig {
private:
    RgbColor front_;
    RgbColor back_;
    RgbColor left_;
    RgbColor right_;
    RgbColor up_;
    RgbColor down_;
    bool usingDefaults_;
    bool enableAnimation_;
    float animationSpeed_;
    int easingType_;
    RendererType rendererType_;

public:
    CubeConfig();

    RgbColor front() const { return front_; }
    RgbColor back()  const { return back_; }
    RgbColor left()  const { return left_; }
    RgbColor right() const { return right_; }
    RgbColor up()    const { return up_; }
    RgbColor down()  const { return down_; }

    void setFront(const RgbColor& color) { front_ = color; }
    void setBack(const RgbColor& color)  { back_ = color; }
    void setLeft(const RgbColor& color)  { left_ = color; }
    void setRight(const RgbColor& color) { right_ = color; }
    void setUp(const RgbColor& color)    { up_ = color; }
    void setDown(const RgbColor& color)  { down_ = color; }

    bool isUsingDefaults() const { return usingDefaults_; }
    void setUsingDefaults(bool value) { usingDefaults_ = value; }

    bool  getEnableAnimation() const { return enableAnimation_; }
    void  setEnableAnimation(bool value) { enableAnimation_ = value; }
    float getAnimationSpeed() const { return animationSpeed_; }
    void  setAnimationSpeed(float value) { animationSpeed_ = value; }
    int   getEasingType() const { return easingType_; }
    void  setEasingType(int value) { easingType_ = value; }

    RendererType getRendererType() const { return rendererType_; }
    void setRendererType(RendererType value) { rendererType_ = value; }
};

// Load cube configuration from ~/.rubiks-cube/config.ini
// Returns default configuration if file doesn't exist or is invalid
CubeConfig loadCubeConfig();

// Save cube configuration to ~/.rubiks-cube/config.ini
// Returns true on success, false on failure
bool saveCubeConfig(const CubeConfig& config);

// Get config file path (for informational purposes)
std::string getConfigFilePath();

#endif // CONFIG_H
