#ifndef CONFIG_H
#define CONFIG_H

#include <array>
#include <string>

enum class RendererType {
    OpenGL = 0,
    Shader = 1
};

struct RgbColor {
    float r;
    float g;
    float b;

    RgbColor() : r(0.0f), g(0.0f), b(0.0f) {}
    RgbColor(float red, float green, float blue) : r(red), g(green), b(blue) {}
    RgbColor(std::array<float, 3> a) : r(a[0]), g(a[1]), b(a[2]) {}

    std::array<float, 3> toArray() const {
        return {r, g, b};
    }
};

class ColorConfig {
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
    ColorConfig();

    // Getters for renderer compatibility
    std::array<float, 3> getFrontColor() const { return front_.toArray(); }
    std::array<float, 3> getBackColor()  const { return back_.toArray(); }
    std::array<float, 3> getLeftColor()  const { return left_.toArray(); }
    std::array<float, 3> getRightColor() const { return right_.toArray(); }
    std::array<float, 3> getUpColor()    const { return up_.toArray(); }
    std::array<float, 3> getDownColor()  const { return down_.toArray(); }

    // Getters for individual colors
    const RgbColor& front() const { return front_; }
    const RgbColor& back()  const { return back_; }
    const RgbColor& left()  const { return left_; }
    const RgbColor& right() const { return right_; }
    const RgbColor& up()    const { return up_; }
    const RgbColor& down()  const { return down_; }

    // Setters for individual colors
    void setFront(const RgbColor& color) { front_ = color; }
    void setBack(const RgbColor& color)  { back_ = color; }
    void setLeft(const RgbColor& color)  { left_ = color; }
    void setRight(const RgbColor& color) { right_ = color; }
    void setUp(const RgbColor& color)    { up_ = color; }
    void setDown(const RgbColor& color)  { down_ = color; }

    // Check if using default colors
    bool isUsingDefaults() const { return usingDefaults_; }
    void setUsingDefaults(bool value) { usingDefaults_ = value; }

    // Animation settings
    bool getEnableAnimation() const { return enableAnimation_; }
    void setEnableAnimation(bool value) { enableAnimation_ = value; }
    float getAnimationSpeed() const { return animationSpeed_; }
    void setAnimationSpeed(float value) { animationSpeed_ = value; }
    int getEasingType() const { return easingType_; }
    void setEasingType(int value) { easingType_ = value; }

    RendererType getRendererType() const { return rendererType_; }
    void setRendererType(RendererType value) { rendererType_ = value; }

    // Set from array (for UI convenience)
    void setFront(const std::array<float, 3>& color) { front_ = RgbColor(color); }
    void setBack(const std::array<float, 3>& color)  { back_  = RgbColor(color); }
    void setLeft(const std::array<float, 3>& color)  { left_  = RgbColor(color); }
    void setRight(const std::array<float, 3>& color) { right_ = RgbColor(color); }
    void setUp(const std::array<float, 3>& color)    { up_    = RgbColor(color); }
    void setDown(const std::array<float, 3>& color)  { down_  = RgbColor(color); }
};

// Get default color configuration
ColorConfig getDefaultColorConfig();

// Load color configuration from ~/.rubiks-cube/config.ini
// Returns default configuration if file doesn't exist or is invalid
ColorConfig loadColorConfig();

// Save color configuration to ~/.rubiks-cube/config.ini
// Returns true on success, false on failure
bool saveColorConfig(const ColorConfig& config);

// Get config file path (for informational purposes)
std::string getConfigFilePath();

#endif // CONFIG_H
