#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <getopt.h>

#include "cube.h"
#include "renderer.h"
#include "formula.h"
#include "config.h"

// Global flag to enable/disable cube dump
bool g_enableDump = false;

// Global variables for fullscreen toggle
bool g_isFullscreen = false;
int g_windowedX = 0, g_windowedY = 0;
int g_windowedWidth = 1400, g_windowedHeight = 900;

// Global variable to store the last scramble sequence
std::string g_lastScramble = "No scramble generated";

// Global formula manager
FormulaManager g_formulaManager;

// Global flag to show About dialog
bool g_showAboutDialog = false;

// Global variables for step-by-step formula execution
std::vector<Move> g_stepByStepMoves;
int g_currentStepIndex = 0;
bool g_isStepByStepMode = false;

// Global variable for editable formula input
char g_formulaInput[1024] = "";  // Buffer for editable formula input
bool g_formulaInputDirty = true;  // Flag to indicate input needs update from selected formula

// Helper function to reset step-by-step mode
void resetStepByStepMode() {
    g_isStepByStepMode = false;
    g_currentStepIndex = 0;
    g_stepByStepMoves.clear();
}

// Helper function to save current renderer configuration
void saveRendererConfig(CubeRenderer& renderer) {
    ColorConfig config;
    config.setFront(renderer.customFront);
    config.setBack(renderer.customBack);
    config.setLeft(renderer.customLeft);
    config.setRight(renderer.customRight);
    config.setUp(renderer.customUp);
    config.setDown(renderer.customDown);
    config.setEnableAnimation(renderer.enableAnimation);
    config.setAnimationSpeed(renderer.animationSpeed);
    config.setUsingDefaults(false);
    saveColorConfig(config);
}

// Helper function to create color picker with auto-save
void addColorPicker(const char* id, const char* label, std::array<float, 3>& color, CubeRenderer& renderer) {
    if (ImGui::ColorEdit3(id, color.data())) {
        renderer.useCustomColors = true;
        saveRendererConfig(renderer);
    }
    ImGui::SameLine();
    ImGui::Text("%s", label);
}

// Helper function to handle keyboard shortcuts for moves
void handleMoveShortcut(ImGuiKey key, Move normalMove, Move primeMove, CubeRenderer& renderer, ImGuiIO& io) {
    if (ImGui::IsKeyPressed(key)) {
        // Don't execute move if Ctrl is pressed (for Ctrl+Z, Ctrl+R, Ctrl+Q)
        if (!io.KeyCtrl) {
            renderer.executeMove(io.KeyShift ? primeMove : normalMove);
        }
    }
}

// Helper function to build move history string
std::string buildMoveHistoryString(const std::vector<Move>& history) {
    std::string result;
    for (size_t i = 0; i < history.size(); ++i) {
        if (i > 0) result += " ";
        result += moveToString(history[i]);
    }
    return result;
}

// Helper function to draw disabled button
void drawDisabledButton(const char* label, ImVec2 size) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    ImGui::Button(label, size);
    ImGui::PopStyleVar();
    ImGui::PopItemFlag();
}

// Helper function to reset cube
void resetCube(CubeRenderer& renderer) {
    renderer.reset();
    renderer.resetView();
}

void showAbout() {
    if (g_showAboutDialog) {
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("About", &g_showAboutDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Rubik's Cube Simulator");
            ImGui::Separator();
            ImGui::Text("Version: 1.0.0");
            ImGui::Text("Author: Walter");
            ImGui::Spacing();
            ImGui::TextWrapped("A 3D Rubik's cube simulator built with C++, ImGUI, and OpenGL. Rotate the cube using standard Rubik's cube notation formulas.");
            ImGui::Spacing();
            ImGui::TextWrapped("Dedicated for my mother, Kangyu Qiu. @2026");
            ImGui::Spacing();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                g_showAboutDialog = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void showHelp(const char* programName) {
    std::cout << "Rubik's Cube Simulator\n\n";
    std::cout << "Usage: " << programName << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -d, --dump    Enable cube state dump to console\n";
    std::cout << "  -h, --help    Show this help message\n\n";
    std::cout << "Keyboard Shortcuts:\n";
    std::cout << "  U/D/L/R/F/B/M/E/S - Execute corresponding move (clockwise)\n";
    std::cout << "  Shift+Key           - Execute prime move (counter-clockwise)\n";
    std::cout << "  Space               - Reset view to default angles\n";
    std::cout << "  ESC                 - Reset cube to solved state\n";
    std::cout << "  Ctrl+Z              - Undo last move\n";
    std::cout << "  Ctrl+R              - Redo last undone move\n";
    std::cout << "  Ctrl+Q              - Quit application\n";
    std::cout << "  F11                 - Toggle fullscreen mode\n";
    std::cout << "  Example: 'U' = U move, 'Shift+U' = U' move\n";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments using getopt_long
    static struct option long_options[] = {
        {"dump", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "dh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'd':
                g_enableDump = true;
                break;
            case 'h':
                showHelp(argv[0]);
                return 0;
            default:
                std::cerr << "Unknown option\n\n";
                showHelp(argv[0]);
                return 1;
        }
    }

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW (OpenGL 2.1 compatible for simpler rendering)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    // Enable multisampling for anti-aliasing (4 samples)
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window - wider to accommodate T-shaped layout
    GLFWwindow* window = glfwCreateWindow(1400, 900, "Rubik's Cube Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Load Chinese font (Noto Sans CJK)
    // Try multiple font paths in case one is not available
    const char* fontPaths[] = {
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/arphic/uming.ttc",
        "/usr/share/fonts/truetype/arphic/ukai.ttc",
        nullptr
    };

    ImFont* chineseFont = nullptr;
    for (int i = 0; fontPaths[i] != nullptr; i++) {
        chineseFont = io.Fonts->AddFontFromFileTTF(fontPaths[i], 32.0f, nullptr, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        if (chineseFont != nullptr) {
            break;
        }
    }

    // Fallback to default font if Chinese font loading fails
    if (chineseFont == nullptr) {
        std::cerr << "Warning: Failed to load Chinese font, falling back to default font." << std::endl;
        io.FontGlobalScale = 1.3f;
    } else {
        // Set Chinese font as default
        io.FontDefault = chineseFont;
    }

    // Initialize cube renderer
    CubeRenderer renderer;

    // Load color configuration from file
    ColorConfig config = loadColorConfig();
    renderer.setCustomColors(config);

    // Load animation settings from config
    renderer.enableAnimation = config.getEnableAnimation();
    renderer.animationSpeed = config.getAnimationSpeed();

    if (!config.isUsingDefaults()) {
        std::cout << "Loaded custom configuration from: " << getConfigFilePath() << std::endl;
    }

    // Load formulas from directory
    g_formulaManager.loadFormulas();

    // Dump initial cube state if enabled
    if (g_enableDump) {
        std::cout << "\n=== Initial Cube State ===" << std::endl;
        renderer.dump();
    }

    // Main loop
    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        // Get current window size
        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        // Calculate responsive layout dimensions
        float sidebarWidth = 480.0f;
        float netViewHeight = 300.0f;

        // Start Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get ImGuiIO for keyboard shortcuts (works globally, even when mouse is over UI)
        ImGuiIO& io = ImGui::GetIO();

        // Handle keyboard shortcuts for cube moves
        // These work globally, even when mouse is over UI
        handleMoveShortcut(ImGuiKey_R, Move::R, Move::RP, renderer, io);
        handleMoveShortcut(ImGuiKey_U, Move::U, Move::UP, renderer, io);
        handleMoveShortcut(ImGuiKey_D, Move::D, Move::DP, renderer, io);
        handleMoveShortcut(ImGuiKey_L, Move::L, Move::LP, renderer, io);
        handleMoveShortcut(ImGuiKey_F, Move::F, Move::FP, renderer, io);
        handleMoveShortcut(ImGuiKey_B, Move::B, Move::BP, renderer, io);
        handleMoveShortcut(ImGuiKey_M, Move::M, Move::MP, renderer, io);
        handleMoveShortcut(ImGuiKey_E, Move::E, Move::EP, renderer, io);
        handleMoveShortcut(ImGuiKey_S, Move::S, Move::SP, renderer, io);
        handleMoveShortcut(ImGuiKey_X, Move::X, Move::XP, renderer, io);
        handleMoveShortcut(ImGuiKey_Y, Move::Y, Move::YP, renderer, io);
        handleMoveShortcut(ImGuiKey_Z, Move::Z, Move::ZP, renderer, io);

        // Spacebar: reset view to default angles
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            renderer.resetView();
        }

        // ESC: reset cube to solved state
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            resetStepByStepMode();
            resetCube(renderer);
        }

        // Ctrl+Q: quit application
        if (ImGui::IsKeyPressed(ImGuiKey_Q) && io.KeyCtrl) {
            glfwSetWindowShouldClose(window, 1);
        }

        // Ctrl+Z: undo
        if (ImGui::IsKeyPressed(ImGuiKey_Z) && io.KeyCtrl) {
            renderer.undo();
        }

        // Ctrl+R: redo
        if (ImGui::IsKeyPressed(ImGuiKey_R) && io.KeyCtrl) {
            renderer.redo();
        }

        // F11: toggle fullscreen
        if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
            if (g_isFullscreen) {
                // Switch back to windowed mode
                glfwSetWindowMonitor(window, nullptr, g_windowedX, g_windowedY, g_windowedWidth, g_windowedHeight, 0);
                g_isFullscreen = false;
            } else {
                // Save current window position and size
                glfwGetWindowPos(window, &g_windowedX, &g_windowedY);
                glfwGetWindowSize(window, &g_windowedWidth, &g_windowedHeight);

                // Switch to fullscreen
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                g_isFullscreen = true;
            }
        }

        // ===== Window 1: 3D View (Left side) =====
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(windowWidth - sidebarWidth - 20, windowHeight - 20), ImGuiCond_Always);
        ImGui::Begin("3D View", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Menu bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Reset")) {
                    resetCube(renderer);
                }
                if (ImGui::MenuItem("About")) {
                    g_showAboutDialog = true;
                    ImGui::OpenPopup("About");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, 1);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Get draw list
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Calculate center position
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::GetContentRegionAvail();
        ImVec2 center(cursor.x + size.x / 2.0f, cursor.y + size.y / 2.0f);

        // Handle mouse drag for rotation
        // Use MouseDelta instead of GetMouseDragDelta to get per-frame delta, not accumulated
        if (ImGui::IsWindowHovered()) {
            // Left mouse button: rotate around X and Y axes
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                renderer.targetRotationY += io.MouseDelta.x * 0.2f;
                renderer.targetRotationX += io.MouseDelta.y * 0.2f;
            }
            // Right mouse button: rotate around Z axis
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                renderer.targetRotationZ += io.MouseDelta.x * 0.3f;
            }
            // Mouse wheel: also rotate around Z axis
            if (io.MouseWheel != 0.0f) {
                renderer.targetRotationZ += io.MouseWheel * 15.0f;
            }
        }

        // Update animation
        renderer.updateAnimation(deltaTime);

        // Draw light blue background for 3D view
        ImVec2 bgMin = cursor;
        ImVec2 bgMax = ImVec2(cursor.x + size.x, cursor.y + size.y);
        ImU32 lightBlue = IM_COL32(217, 235, 255, 64);  // Light blue RGB (0.85, 0.92, 1.0)
        drawList->AddRectFilled(bgMin, bgMax, lightBlue);

        // Draw 3D cube
        renderer.draw3D(drawList, center, renderer.scale);

        // Add dummy space
        ImGui::Dummy(size);

        ImGui::End();

        // ===== Window 2: 2D Unfolded View (Top Right) =====
        // Responsive height based on window size (25% of window height, min 250px)
        ImGui::SetNextWindowPos(ImVec2(windowWidth - sidebarWidth + 10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth - 20, netViewHeight), ImGuiCond_Always);
        ImGui::Begin("2D Net View", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        drawList = ImGui::GetWindowDrawList();
        cursor = ImGui::GetCursorScreenPos();
        size = ImGui::GetContentRegionAvail();
        center = ImVec2(cursor.x + size.x / 2.0f, cursor.y + size.y / 2.0f);

        // Handle mouse wheel for 2D view zoom
        if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
            renderer.scale2D += io.MouseWheel * 0.2f;
            // Clamp scale2D between 0.3f and 3.0f
            renderer.scale2D = fmaxf(0.3f, fminf(3.0f, renderer.scale2D));
        }

        // Draw 2D unfolded cube
        renderer.draw2D(drawList, center, renderer.scale2D);

        ImGui::Dummy(size);
        ImGui::End();

        // ===== Window 3: Controls (Bottom Right) =====
        // Start after 2D Net View window (fixed height + 10px gap)
        float controlsY = 10.0f + netViewHeight + 10.0f;
        float controlsHeight = windowHeight - controlsY - 10.0f;
        ImGui::SetNextWindowPos(ImVec2(windowWidth - sidebarWidth + 10, controlsY), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth - 20, controlsHeight), ImGuiCond_Always);
        ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Tab bar for Moves, Shortcuts, and Formulas
        if (ImGui::BeginTabBar("ControlsTabBar", ImGuiTabBarFlags_None)) {
            // Moves tab
            if (ImGui::BeginTabItem("Moves")) {
                // Scramble and Reset buttons
                if (ImGui::Button("Scramble", ImVec2(160, 0))) {
                    // Disable animation for instant scramble
                    bool oldAnimation = renderer.enableAnimation;
                    renderer.enableAnimation = false;

                    // Generate and execute scramble
                    std::vector<Move> scrambleMoves = renderer.scramble(20);

                    // Restore animation setting
                    renderer.enableAnimation = oldAnimation;
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset Cube", ImVec2(160, 0))) {
                    resetStepByStepMode();
                    resetCube(renderer);
                }

                ImGui::Separator();

                // Move buttons (3 rows: 6 buttons each)

                // Row 1: R, L, M
                if (ImGui::Button("R", ImVec2(40, 0))) renderer.executeMove(Move::R);
                ImGui::SameLine();
                if (ImGui::Button("R'", ImVec2(40, 0))) renderer.executeMove(Move::RP);
                ImGui::SameLine(0, 20);
                if (ImGui::Button("L", ImVec2(40, 0))) renderer.executeMove(Move::L);
                ImGui::SameLine();
                if (ImGui::Button("L'", ImVec2(40, 0))) renderer.executeMove(Move::LP);
                ImGui::SameLine(0, 20);
                if (ImGui::Button("M", ImVec2(40, 0))) renderer.executeMove(Move::M);
                ImGui::SameLine();
                if (ImGui::Button("M'", ImVec2(40, 0))) renderer.executeMove(Move::MP);

                // Row 2: U, D, E
                if (ImGui::Button("U", ImVec2(40, 0))) renderer.executeMove(Move::U);
                ImGui::SameLine();
                if (ImGui::Button("U'", ImVec2(40, 0))) renderer.executeMove(Move::UP);
                ImGui::SameLine(0, 20);
                if (ImGui::Button("D", ImVec2(40, 0))) renderer.executeMove(Move::D);
                ImGui::SameLine();
                if (ImGui::Button("D'", ImVec2(40, 0))) renderer.executeMove(Move::DP);
                ImGui::SameLine(0, 20);
                if (ImGui::Button("E", ImVec2(40, 0))) renderer.executeMove(Move::E);
                ImGui::SameLine();
                if (ImGui::Button("E'", ImVec2(40, 0))) renderer.executeMove(Move::EP);

                // Row 3: F, B, S
                if (ImGui::Button("F", ImVec2(40, 0))) renderer.executeMove(Move::F);
                ImGui::SameLine();
                if (ImGui::Button("F'", ImVec2(40, 0))) renderer.executeMove(Move::FP);
                ImGui::SameLine(0, 20);
                if (ImGui::Button("B", ImVec2(40, 0))) renderer.executeMove(Move::B);
                ImGui::SameLine();
                if (ImGui::Button("B'", ImVec2(40, 0))) renderer.executeMove(Move::BP);
                ImGui::SameLine(0, 20);
                if (ImGui::Button("S", ImVec2(40, 0))) renderer.executeMove(Move::S);
                ImGui::SameLine();
                if (ImGui::Button("S'", ImVec2(40, 0))) renderer.executeMove(Move::SP);

                ImGui::Spacing();
                ImGui::Separator();

                // Undo, Redo, and Copy buttons (grouped together)
                bool canUndo = !renderer.getMoveHistory().empty();
                bool canRedo = renderer.canRedo();

                // Undo button
                if (canUndo) {
                    if (ImGui::Button("Undo", ImVec2(100, 0))) {
                        renderer.undo();
                    }
                } else {
                    drawDisabledButton("Undo", ImVec2(100, 0));
                }

                ImGui::SameLine();

                // Redo button
                if (canRedo) {
                    if (ImGui::Button("Redo", ImVec2(100, 0))) {
                        renderer.redo();
                    }
                } else {
                    drawDisabledButton("Redo", ImVec2(100, 0));
                }

                ImGui::SameLine();

                // Copy button
                if (canUndo) {
                    if (ImGui::Button("Copy", ImVec2(100, 0))) {
                        const std::vector<Move>& history = renderer.getMoveHistory();
                        glfwSetClipboardString(window, buildMoveHistoryString(history).c_str());
                    }
                } else {
                    drawDisabledButton("Copy", ImVec2(100, 0));
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                    "Total moves: %zu", renderer.getMoveHistory().size());
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                    "Redo available: %zu", renderer.getRedoHistory().size());

                ImGui::Separator();

                // Display move history as auto-wrapped text
                const std::vector<Move>& history = renderer.getMoveHistory();
                if (!history.empty()) {
                    std::string historyStr = buildMoveHistoryString(history);
                    ImGui::BeginChild("MoveHistory", ImVec2(0, 120), true);
                    ImGui::TextWrapped("%s", historyStr.c_str());
                    ImGui::EndChild();
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                        "No moves in history");
                }

                ImGui::Separator();
                ImGui::Spacing();

                // Cube status
                const char* status = renderer.isSolved() ? "Solved" : "Unsolved";
                ImU32 statusColor = renderer.isSolved() ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 100, 0, 255);
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Cube status: %s", status);

                ImGui::EndTabItem();
            }

            // Formula tab
            if (ImGui::BeginTabItem("Formulas")) {
                ImGui::Separator();

                // Two-column layout for file list and formula items list
                float availableWidth = ImGui::GetContentRegionAvail().x;
                float availableHeight = ImGui::GetContentRegionAvail().y;
                float leftColumnWidth = availableWidth * 0.4f;  // 40% for file list
                float rightColumnWidth = availableWidth * 0.6f; // 60% for formula items

                // Reserve space for buttons at bottom (about 80px)
                float listHeight = availableHeight - 270.0f;

                // Left column: Formula file list
                ImGui::BeginChild("FormulaFileList", ImVec2(leftColumnWidth, listHeight), true);
                {
                    std::vector<std::string> fileNames = g_formulaManager.getFileNames();

                    if (fileNames.empty()) {
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                            "No formulas found in 'formula' directory");
                    } else {
                        for (const auto& filename : fileNames) {
                            const bool isSelected = (g_formulaManager.getSelectedFileName() == filename);

                            if (ImGui::Selectable(filename.c_str(), isSelected)) {
                                g_formulaManager.setSelectedFile(filename);
                                // Auto-scroll to selected file
                                if (isSelected) {
                                    ImGui::SetScrollHereY();
                                }
                            }
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::SameLine();

                // Right column: Formula items in selected file
                ImGui::BeginChild("FormulaItemList", ImVec2(rightColumnWidth, listHeight), true);
                {
                    const std::vector<FormulaItem>* items = g_formulaManager.getSelectedFileItems();

                    if (items != nullptr && !items->empty()) {
                        for (size_t i = 0; i < items->size(); ++i) {
                            const bool isSelected = (g_formulaManager.getSelectedItemIndex() == static_cast<int>(i));

                            if (ImGui::Selectable(items->at(i).name.c_str(), isSelected)) {
                                g_formulaManager.setSelectedItem(i);
                                g_formulaInputDirty = true;  // Mark input for update
                                // Auto-scroll to selected formula item
                                if (isSelected) {
                                    ImGui::SetScrollHereY();
                                }
                            }
                        }
                    } else {
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                            "No formula items");
                    }
                }
                ImGui::EndChild();

                // Display selected formula's moves
                const FormulaItem* selectedItem = g_formulaManager.getSelectedItem();

                ImGui::Spacing();
                if (selectedItem != nullptr && !selectedItem->moves.empty()) {
                    // Update input buffer when formula selection changes
                    if (g_formulaInputDirty) {
                        // Build formula string from moves
                        std::string formulaStr;
                        for (size_t i = 0; i < selectedItem->moves.size(); ++i) {
                            if (i > 0) formulaStr += " ";
                            formulaStr += moveToString(selectedItem->moves[i]);
                        }
                        snprintf(g_formulaInput, sizeof(g_formulaInput), "%s", formulaStr.c_str());
                        g_formulaInputDirty = false;
                    }

                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Selected: %s",
                        selectedItem->name.c_str());
                    ImGui::Separator();
                }

                // Editable input text box for formula (always visible)
                if (ImGui::InputText("##FormulaInput", g_formulaInput, sizeof(g_formulaInput),
                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
                    // User pressed Enter, re-parse the formula
                    g_formulaInputDirty = false;
                }

                // Display move count
                std::vector<Move> parsedMoves = parseMoveSequence(g_formulaInput);
                // Show step progress if in step-by-step mode
                if (g_isStepByStepMode) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                        "Total moves: %zu (Step: %d/%zu)",
                        parsedMoves.size(), g_currentStepIndex, g_stepByStepMoves.size());
                } else {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                        "Total moves: %zu", parsedMoves.size());
                }

                // Display loop count if formula has loop syntax
                if (selectedItem != nullptr && selectedItem->loopCount > 0) {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                        "| Loop: %d", selectedItem->loopCount);
                }

                ImGui::Separator();

                ImGui::Spacing();

                // Execute buttons
                std::vector<Move> movesFromInput = parseMoveSequence(g_formulaInput);
                bool hasFormula = !movesFromInput.empty();

                if (hasFormula) {
                    // Execute button - automatically loops if formula has loop syntax
                    if (ImGui::Button("Execute", ImVec2(180, 0))) {
                        resetStepByStepMode();

                        // Parse moves from input
                        std::vector<Move> moves = parseMoveSequence(g_formulaInput);

                        // Execute formula loopCount times if formula has loop syntax, otherwise execute once
                        int loopCount = (selectedItem != nullptr && selectedItem->loopCount > 0) ? selectedItem->loopCount : 1;
                        for (int i = 0; i < loopCount; ++i) {
                            for (const Move& move : moves) {
                                renderer.executeMove(move);
                            }
                        }
                    }

                    ImGui::SameLine();

                    // Execute Reverse button
                    if (ImGui::Button("Execute Reverse", ImVec2(180, 0))) {
                        resetStepByStepMode();

                        // Parse and execute moves in reverse order with inverse moves
                        std::vector<Move> moves = parseMoveSequence(g_formulaInput);
                        for (auto it = moves.rbegin(); it != moves.rend(); ++it) {
                            renderer.executeMove(getInverseMove(*it));
                        }
                    }

                    ImGui::Separator();

                    // Step button
                    bool canStep = hasFormula && (!g_isStepByStepMode || g_currentStepIndex < static_cast<int>(g_stepByStepMoves.size()));
                    if (canStep) {
                        if (ImGui::Button("Step", ImVec2(180, 0))) {
                            // Start step-by-step mode if not already active
                            if (!g_isStepByStepMode) {
                                g_stepByStepMoves = parseMoveSequence(g_formulaInput);
                                g_currentStepIndex = 0;
                                g_isStepByStepMode = true;
                            }

                            // Execute current step
                            if (g_currentStepIndex < static_cast<int>(g_stepByStepMoves.size())) {
                                renderer.executeMove(g_stepByStepMoves[g_currentStepIndex]);
                                g_currentStepIndex++;

                                // Exit step mode when complete
                                if (g_currentStepIndex >= static_cast<int>(g_stepByStepMoves.size())) {
                                    g_isStepByStepMode = false;
                                }
                            }
                        }
                    } else {
                        drawDisabledButton("Step", ImVec2(180, 0));
                    }

                    ImGui::SameLine();

                    // Reset Step Mode button - only show when in step-by-step mode
                    if (g_isStepByStepMode) {
                        if (ImGui::Button("Reset Step", ImVec2(180, 0))) {
                            resetStepByStepMode();
                        }
                    } else {
                        drawDisabledButton("Reset Step", ImVec2(180, 0));
                    }
                } else {
                    drawDisabledButton("Execute", ImVec2(180, 0));
                    ImGui::SameLine();
                    drawDisabledButton("Execute Reverse", ImVec2(180, 0));
                }

                ImGui::Spacing();

                // Refresh and Reset buttons
                if (ImGui::Button("Reload Formulas", ImVec2(180, 0))) {
                    g_formulaManager.refresh();
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset Cube", ImVec2(180, 0))) {
                    resetStepByStepMode();
                    resetCube(renderer);
                }

                ImGui::EndTabItem();
            }

            // Settings tab (consolidated View + Animation)
            if (ImGui::BeginTabItem("Settings")) {
                ImGui::Separator();

                // Animation section
                ImGui::Text("Animation:");
                bool prevEnableAnim = renderer.enableAnimation;
                float prevAnimSpeed = renderer.animationSpeed;
                ImGui::Checkbox("Enable Animation", &renderer.enableAnimation);
                ImGui::SliderFloat("Speed", &renderer.animationSpeed, 0.1f, 3.0f, "%.1fx", ImGuiSliderFlags_Logarithmic);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("1.0x = 200ms per move");
                }

                // Save animation settings when slider is deactivated (user released it)
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    saveRendererConfig(renderer);
                }

                // Also save when checkbox changes
                if (renderer.enableAnimation != prevEnableAnim) {
                    saveRendererConfig(renderer);
                }

                // Show animation status
                if (renderer.isAnimating()) {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "(Playing %.0f%%)",
                                      renderer.animationProgress() * 100.0f);
                }

                ImGui::Spacing();
                ImGui::Separator();

                // 2D view controls
                ImGui::Text("2D View Controls:");
                ImGui::SliderFloat("2D Scale", &renderer.scale2D, 0.3f, 3.0f, "%.2f");

                ImGui::Spacing();

                // 3D view controls
                ImGui::Text("3D View Controls:");
                ImGui::SliderFloat("Rotation X", &renderer.rotationX, -180.0f, 180.0f);
                ImGui::SliderFloat("Rotation Y", &renderer.rotationY, -180.0f, 180.0f);
                ImGui::SliderFloat("Rotation Z", &renderer.rotationZ, -180.0f, 180.0f);
                ImGui::SliderFloat("3D Scale", &renderer.scale, 2.0f, 7.0f, "%.2f");

                ImGui::Spacing();
                ImGui::Separator();

                // Custom colors section
                ImGui::Text("Custom Colors:");

                // Color pickers for each face
                addColorPicker("##FrontColor", "Front (Green):", renderer.customFront, renderer);
                addColorPicker("##BackColor", "Back (Blue):", renderer.customBack, renderer);
                addColorPicker("##LeftColor", "Left (Orange):", renderer.customLeft, renderer);
                addColorPicker("##RightColor", "Right (Red):", renderer.customRight, renderer);
                addColorPicker("##UpColor", "Up (White):", renderer.customUp, renderer);
                addColorPicker("##DownColor", "Down (Yellow):", renderer.customDown, renderer);

                ImGui::Spacing();

                // Save and Reset buttons
                // Note: Colors are now auto-saved when modified, but we keep this button for explicit save
                if (ImGui::Button("Save Colors", ImVec2(160, 0))) {
                    saveRendererConfig(renderer);
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset to Defaults", ImVec2(180, 0))) {
                    renderer.customFront = {0.0f, 1.0f, 0.0f};  // Green
                    renderer.customBack = {0.0f, 0.0f, 1.0f};   // Blue
                    renderer.customLeft = {1.0f, 0.5f, 0.0f};  // Orange
                    renderer.customRight = {1.0f, 0.0f, 0.0f}; // Red
                    renderer.customUp = {1.0f, 1.0f, 1.0f};    // White
                    renderer.customDown = {1.0f, 1.0f, 0.0f};  // Yellow
                    renderer.useCustomColors = false;
                    // Remove config file to restore defaults on next startup
                    std::string configPath = getConfigFilePath();
                    if (!configPath.empty()) {
                        std::remove(configPath.c_str());
                    }
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Config file: %s",
                                  getConfigFilePath().c_str());

                ImGui::EndTabItem();
            }

            // Shortcuts tab
            if (ImGui::BeginTabItem("Shortcuts")) {
                ImGui::Spacing();
                ImGui::Text("Keyboard Shortcuts:");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  U/D/L/R/F/B/M/E/S - Move (clockwise)");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Shift+Key - Prime move (counter-clockwise)");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Space - Reset view to default angles");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  ESC - Reset cube to solved state");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Ctrl+Z - Undo last move");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Ctrl+R - Redo last undone move");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Ctrl+Q - Quit application");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  F11 - Toggle fullscreen mode");

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        // Show About dialog if requested
        showAbout();

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Clear the screen
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render ImGui first (2D interface)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Render 3D cube on top (in the 3D view window area)
        // Find the 3D view window and render there
        renderer.render3DOverlay(display_w, display_h);

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
