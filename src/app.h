#ifndef APP_H
#define APP_H

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

#include "cube.h"
#include "renderer.h"
#include "formula.h"

class Application {
public:
    Application() = default;
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    int run();
    void setEnableDump(bool enable) { enableDump_ = enable; }

private:
    bool initGlfw();
    bool initImGui();
    void loadFonts();
    void initApp();

    void handleKeyboardShortcuts();
    void handleMoveShortcut(ImGuiKey key, Move normalMove, Move primeMove, ImGuiIO& io);

    void renderMenuBar();
    void renderControls();

    void renderMovesTab();
    void renderFormulasTab();
    void renderSettingsTab();
    void renderShortcutsTab();

    void showAbout();

    void resetStepByStepMode();
    void saveRendererConfig();
    void resetCube();
    void scrambleCube();
    void toggleFullscreen();
    std::string buildMoveHistoryString() const;
    void addColorPicker(const char* id, const char* label, std::array<float, 3>& color);
    void drawDisabledButton(const char* label, ImVec2 size);
    void addMoveButton(const char* label, Move move, ImVec2 size = ImVec2(40, 0));
    void addMoveButtonPair(const char* label, Move normalMove, Move primeMove, ImVec2 size = ImVec2(40, 0));

    GLFWwindow* window_ = nullptr;
    RubiksCube cube_;
    std::unique_ptr<CubeRenderer> renderer_;
    FormulaManager formulaManager_;

    bool isFullscreen_ = false;
    int windowedX_ = 0;
    int windowedY_ = 0;
    int windowedWidth_ = 1400;
    int windowedHeight_ = 900;

    std::vector<Move> stepByStepMoves_;
    int currentStepIndex_ = 0;
    bool isStepByStepMode_ = false;

    char formulaInput_[1024] = "";
    bool formulaInputDirty_ = true;

    bool enableDump_ = false;
    bool showAboutDialog_ = false;
    std::string lastScramble_ = "No scramble generated";

    float sidebarWidth_ = 320.0f;
    float netViewHeight_ = 250.0f;
};

#endif
