#ifndef APP_H
#define APP_H

#include <memory>

#include "renderer.h"
#include "formula.h"

struct GLFWwindow;

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
    void setAutoScramble(bool enable) { autoScramble_ = enable; }
    void setAutoCelebrate(bool enable) { autoCelebrate_ = enable; }

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
    void addColorPicker(const char* id, const char* label, RgbColor& color);
    void drawDisabledButton(const char* label, ImVec2 size);
    void addMoveButton(const char* label, Move move, ImVec2 size = ImVec2(40, 0));
    void addMoveButtonPair(Move normalMove, ImVec2 size = ImVec2(40, 0));

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
    std::vector<Move> formulaParsedMoves_;

    bool enableDump_ = false;
    bool autoScramble_ = false;
    bool autoCelebrate_ = false;
    bool showAboutDialog_ = false;
    std::string lastScramble_ = "No scramble generated";

    mutable std::string cachedHistoryStr_;
    mutable size_t cachedHistorySize_ = static_cast<size_t>(-1);

    float sidebarWidth_ = 480.0f;
    float netViewHeight_ = 300.0f;
};

#endif
