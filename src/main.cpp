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

// Global variable to store the last scramble sequence
std::string g_lastScramble = "No scramble generated";

// Global formula manager
FormulaManager g_formulaManager;

void showHelp(const char* programName) {
    std::cout << "Rubik's Cube Simulator\n\n";
    std::cout << "Usage: " << programName << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -d, --dump    Enable cube state dump to console\n";
    std::cout << "  -h, --help    Show this help message\n\n";
    std::cout << "Keyboard Shortcuts:\n";
    std::cout << "  U/D/L/R/F/B/M/E/S - Execute corresponding move (clockwise)\n";
    std::cout << "  Shift+Key           - Execute prime move (counter-clockwise)\n";
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
    GLFWwindow* window = glfwCreateWindow(1600, 1200, "Rubik's Cube Simulator", nullptr, nullptr);
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
    if (!config.isUsingDefaults()) {
        std::cout << "Loaded custom color configuration from: " << getConfigFilePath() << std::endl;
    }

    // Load formulas from directory
    g_formulaManager.loadFormulas();

    // Dump initial cube state if enabled
    if (g_enableDump) {
        std::cout << "\n=== Initial Cube State ===" << std::endl;
        renderer.dump();
    }

    // Window dimensions
    int sidebarWidth = 600;

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

        // Start Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get ImGuiIO for keyboard shortcuts (works globally, even when mouse is over UI)
        ImGuiIO& io = ImGui::GetIO();

        // Handle keyboard shortcuts for cube moves
        // These work globally, even when mouse is over UI

        // U moves (Up face)
        if (ImGui::IsKeyPressed(ImGuiKey_U)) {
            // Check for shift modifier for prime move
            bool isPrime = io.KeyShift;
            renderer.executeMove(isPrime ? Move::UP : Move::U);
        }

        // D moves (Down face)
        if (ImGui::IsKeyPressed(ImGuiKey_D)) {
            // Check for shift modifier for prime move
            bool isPrime = io.KeyShift;
            renderer.executeMove(isPrime ? Move::DP : Move::D);
        }

        // L moves (Left face)
        if (ImGui::IsKeyPressed(ImGuiKey_L)) {
            // Check for shift modifier for prime move
            bool isPrime = io.KeyShift;
            renderer.executeMove(isPrime ? Move::LP : Move::L);
        }

        // R moves (Right face)
        if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            // Check for shift modifier for prime move
            bool isPrime = io.KeyShift;
            renderer.executeMove(isPrime ? Move::RP : Move::R);
        }

        // F moves (Front face)
        if (ImGui::IsKeyPressed(ImGuiKey_F)) {
            // Check for shift modifier for prime move
            bool isPrime = io.KeyShift;
            renderer.executeMove(isPrime ? Move::FP : Move::F);
        }

        // B moves (Back face)
        if (ImGui::IsKeyPressed(ImGuiKey_B)) {
            // Check for shift modifier for prime move
            bool isPrime = io.KeyShift;
            renderer.executeMove(isPrime ? Move::BP : Move::B);
        }

        // M moves (Middle slice)
        if (ImGui::IsKeyPressed(ImGuiKey_M)) {
            // Check for shift modifier for prime move
            bool isPrime = io.KeyShift;
            renderer.executeMove(isPrime ? Move::MP : Move::M);
        }

        // E moves (Equator slice)
        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            // Check for shift modifier for prime move
            bool isPrime = io.KeyShift;
            renderer.executeMove(isPrime ? Move::EP : Move::E);
        }

        // S moves (Standing slice)
        if (ImGui::IsKeyPressed(ImGuiKey_S)) {
            // Check for shift modifier for prime move
            bool isPrime = io.KeyShift;
            renderer.executeMove(isPrime ? Move::SP : Move::S);
        }

        // Spacebar: reset view to default angles
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            renderer.resetView();
        }

        // ===== Window 1: 3D View (Left side) =====
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(windowWidth - sidebarWidth - 20, windowHeight - 20), ImGuiCond_Always);
        ImGui::Begin("3D View", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Menu bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Reset")) {
                    renderer.reset();
                    renderer.resetView();
                }
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
        // Fixed height based on 2D cube net content (3 face rows + 2 gaps + padding)
        // With scale2D = 0.8: faceSize = 96, gap = 2.4, content = 3*96 + 2*2.4 = 292.8
        // Add 40px for padding (20 top + 20 bottom) and window title bar
        float netViewHeight = 314.0f;  // Content height + padding + title bar (one grid lower)
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

        // Tab bar for Moves, History, and Animation controls
        if (ImGui::BeginTabBar("ControlsTabBar", ImGuiTabBarFlags_None)) {
            // Moves tab
            if (ImGui::BeginTabItem("Moves")) {
                // Scramble and Reset buttons
                if (ImGui::Button("Scramble", ImVec2(120, 0))) {
                    // Disable animation for instant scramble
                    bool oldAnimation = renderer.enableAnimation;
                    renderer.enableAnimation = false;

                    // Generate and execute scramble
                    std::vector<Move> scrambleMoves = renderer.scramble(20);

                    // Restore animation setting
                    renderer.enableAnimation = oldAnimation;
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset Cube", ImVec2(120, 0))) {
                    renderer.reset();
                    renderer.resetView();
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
                ImGui::Text("Keyboard Shortcuts:");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  U/D/L/R/F/B/M/E/S - Move (clockwise)");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Shift+Key - Prime move (counter-clockwise)");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Space - Reset view to default angles");

                ImGui::EndTabItem();
            }

            // History tab
            if (ImGui::BeginTabItem("History")) {
                ImGui::Separator();

                // Undo button
                bool canUndo = !renderer.getMoveHistory().empty();
                if (canUndo) {
                    if (ImGui::Button("Undo Last Move", ImVec2(200, 0))) {
                        renderer.undo();
                    }
                } else {
                    // Disabled button when no history
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    ImGui::Button("Undo Last Move", ImVec2(160, 0));
                    ImGui::PopStyleVar();
                    ImGui::PopItemFlag();
                }

                ImGui::SameLine();

                // Redo button
                bool canRedo = renderer.canRedo();
                if (canRedo) {
                    if (ImGui::Button("Redo Next Move", ImVec2(200, 0))) {
                        renderer.redo();
                    }
                } else {
                    // Disabled button when no redo history
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    ImGui::Button("Redo Next Move", ImVec2(160, 0));
                    ImGui::PopStyleVar();
                    ImGui::PopItemFlag();
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                    "Total moves: %zu", renderer.getMoveHistory().size());
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                    "Redo available: %zu", renderer.getRedoHistory().size());

                ImGui::Separator();

                // Display move history in a scrollable list with auto-wrapping
                const std::vector<Move>& history = renderer.getMoveHistory();
                if (!history.empty()) {
                    // Use a child window for scrolling without horizontal scrollbar
                    ImGui::BeginChild("MoveHistory", ImVec2(0, 150), true);

                    // Group moves in sets of 6 per line for better readability
                    const int movesPerLine = 6;
                    int movesOnCurrentLine = 0;

                    for (size_t i = 0; i < history.size(); ++i) {
                        // Start a new line every 6 moves (except for the first move)
                        if (movesOnCurrentLine > 0 && movesOnCurrentLine % movesPerLine == 0) {
                            ImGui::NewLine();
                            movesOnCurrentLine = 0;
                        }

                        // Color the move text
                        ImVec4 moveColor;
                        if (history[i] == Move::U || history[i] == Move::D ||
                            history[i] == Move::UP || history[i] == Move::DP) {
                            moveColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Orange for U/D
                        } else if (history[i] == Move::L || history[i] == Move::R ||
                                   history[i] == Move::LP || history[i] == Move::RP) {
                            moveColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green for L/R
                        } else if (history[i] == Move::F || history[i] == Move::B ||
                                   history[i] == Move::FP || history[i] == Move::BP) {
                            moveColor = ImVec4(0.0f, 0.8f, 1.0f, 1.0f); // Cyan for F/B
                        } else {
                            moveColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White for slice moves
                        }

                        // Display move with proper spacing
                        if (movesOnCurrentLine > 0) {
                            ImGui::SameLine(0, 5); // 5px spacing between moves
                        }

                        ImGui::TextColored(moveColor, "%zu. %s", i + 1,
                                        moveToString(history[i]).c_str());
                        movesOnCurrentLine++;
                    }

                    // Auto-scroll to bottom when new moves are added
                    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                        ImGui::SetScrollHereY(1.0f);
                    }

                    ImGui::EndChild();
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                        "No moves in history");
                }

                ImGui::EndTabItem();
            }

            // Animation tab
            if (ImGui::BeginTabItem("Animation")) {
                ImGui::Separator();

                // Enable/Disable animation checkbox
                ImGui::Checkbox("Enable Animation", &renderer.enableAnimation);

                ImGui::Spacing();

                // Animation speed slider
                ImGui::Text("Animation Speed:");
                ImGui::SliderFloat("Speed", &renderer.animationSpeed, 0.1f, 3.0f, "%.1fx", ImGuiSliderFlags_Logarithmic);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("1.0x = 200ms per move");
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Animation queue status:");
                if (renderer.isAnimating()) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "  Playing... (%.0f%%)",
                                      renderer.animationProgress() * 100.0f);
                } else {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "  Idle");
                }

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
                float listHeight = availableHeight - 80.0f;

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
                if (selectedItem != nullptr && !selectedItem->moves.empty()) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Selected: %s",
                        selectedItem->name.c_str());
                    ImGui::Separator();

                    // Display moves with formatting (no line wrapping)
                    const int movesPerLine = 100;  // Large number to prevent wrapping
                    int movesOnCurrentLine = 0;

                    for (size_t i = 0; i < selectedItem->moves.size(); ++i) {
                        if (movesOnCurrentLine > 0 && movesOnCurrentLine % movesPerLine == 0) {
                            ImGui::NewLine();
                            movesOnCurrentLine = 0;
                        }

                        // Color code moves by type
                        ImVec4 moveColor;
                        const Move move = selectedItem->moves[i];
                        if (move == Move::U || move == Move::UP ||
                            move == Move::D || move == Move::DP) {
                            moveColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Orange
                        } else if (move == Move::L || move == Move::LP ||
                                   move == Move::R || move == Move::RP) {
                            moveColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
                        } else if (move == Move::F || move == Move::FP ||
                                   move == Move::B || move == Move::BP) {
                            moveColor = ImVec4(0.0f, 0.8f, 1.0f, 1.0f); // Cyan
                        } else {
                            moveColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
                        }

                        if (movesOnCurrentLine > 0) {
                            ImGui::SameLine(0, 5);
                        }

                        ImGui::TextColored(moveColor, "%s", moveToString(move).c_str());
                        movesOnCurrentLine++;
                    }

                    ImGui::Separator();

                    // Display move count
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                        "Total moves: %zu", selectedItem->moves.size());
                }

                ImGui::Spacing();

                // Execute buttons
                bool hasFormula = (selectedItem != nullptr && !selectedItem->moves.empty());

                if (hasFormula) {
                    // Execute button
                    if (ImGui::Button("Execute", ImVec2(180, 0))) {
                        // Execute all moves in formula with animation
                        for (const Move& move : selectedItem->moves) {
                            renderer.executeMove(move);
                        }
                    }

                    ImGui::SameLine();

                    // Execute Reverse button
                    if (ImGui::Button("Execute Reverse", ImVec2(180, 0))) {
                        // Execute moves in reverse order with inverse moves
                        for (auto it = selectedItem->moves.rbegin(); it != selectedItem->moves.rend(); ++it) {
                            Move move = *it;
                            Move inverseMove;

                            // Get inverse move
                            switch (move) {
                                case Move::U:  inverseMove = Move::UP; break;
                                case Move::UP: inverseMove = Move::U; break;
                                case Move::D:  inverseMove = Move::DP; break;
                                case Move::DP: inverseMove = Move::D; break;
                                case Move::L:  inverseMove = Move::LP; break;
                                case Move::LP: inverseMove = Move::L; break;
                                case Move::R:  inverseMove = Move::RP; break;
                                case Move::RP: inverseMove = Move::R; break;
                                case Move::F:  inverseMove = Move::FP; break;
                                case Move::FP: inverseMove = Move::F; break;
                                case Move::B:  inverseMove = Move::BP; break;
                                case Move::BP: inverseMove = Move::B; break;
                                case Move::M:  inverseMove = Move::MP; break;
                                case Move::MP: inverseMove = Move::M; break;
                                case Move::E:  inverseMove = Move::EP; break;
                                case Move::EP: inverseMove = Move::E; break;
                                case Move::S:  inverseMove = Move::SP; break;
                                case Move::SP: inverseMove = Move::S; break;
                                default: continue;
                            }

                            renderer.executeMove(inverseMove);
                        }
                    }
                } else {
                    // Disabled buttons when no formula selected
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    ImGui::Button("Execute", ImVec2(200, 0));
                    ImGui::PopStyleVar();
                    ImGui::PopItemFlag();

                    ImGui::SameLine();

                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    ImGui::Button("Execute Reverse", ImVec2(180, 0));
                    ImGui::PopStyleVar();
                    ImGui::PopItemFlag();
                }

                ImGui::Spacing();

                // Refresh and Reset buttons
                if (ImGui::Button("Refresh Formulas", ImVec2(180, 0))) {
                    g_formulaManager.refresh();
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset Cube", ImVec2(180, 0))) {
                    renderer.reset();
                    renderer.resetView();
                }

                ImGui::EndTabItem();
            }

            // View tab
            if (ImGui::BeginTabItem("View")) {
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
                ImGui::Text("Front (Green):");
                ImGui::SameLine();
                if (ImGui::ColorEdit3("##FrontColor", renderer.customFront.data())) {
                    renderer.useCustomColors = true;
                }

                ImGui::Text("Back (Blue):");
                ImGui::SameLine();
                if (ImGui::ColorEdit3("##BackColor", renderer.customBack.data())) {
                    renderer.useCustomColors = true;
                }

                ImGui::Text("Left (Orange):");
                ImGui::SameLine();
                if (ImGui::ColorEdit3("##LeftColor", renderer.customLeft.data())) {
                    renderer.useCustomColors = true;
                }

                ImGui::Text("Right (Red):");
                ImGui::SameLine();
                if (ImGui::ColorEdit3("##RightColor", renderer.customRight.data())) {
                    renderer.useCustomColors = true;
                }

                ImGui::Text("Up (White):");
                ImGui::SameLine();
                if (ImGui::ColorEdit3("##UpColor", renderer.customUp.data())) {
                    renderer.useCustomColors = true;
                }

                ImGui::Text("Down (Yellow):");
                ImGui::SameLine();
                if (ImGui::ColorEdit3("##DownColor", renderer.customDown.data())) {
                    renderer.useCustomColors = true;
                }

                ImGui::Spacing();

                // Save and Reset buttons
                if (ImGui::Button("Save Colors", ImVec2(120, 0))) {
                    ColorConfig config;
                    config.setFront(renderer.customFront);
                    config.setBack(renderer.customBack);
                    config.setLeft(renderer.customLeft);
                    config.setRight(renderer.customRight);
                    config.setUp(renderer.customUp);
                    config.setDown(renderer.customDown);
                    config.setUsingDefaults(false);
                    saveColorConfig(config);
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset to Defaults", ImVec2(140, 0))) {
                    renderer.customFront = {0.0f, 1.0f, 0.0f};  // Green
                    renderer.customBack = {0.0f, 0.0f, 1.0f};   // Blue
                    renderer.customLeft = {1.0f, 0.5f, 0.0f};  // Orange
                    renderer.customRight = {1.0f, 0.0f, 0.0f}; // Red
                    renderer.customUp = {1.0f, 1.0f, 1.0f};    // White
                    renderer.customDown = {1.0f, 1.0f, 0.0f};  // Yellow
                    renderer.useCustomColors = false;
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Config file: %s",
                                  getConfigFilePath().c_str());

                ImGui::Spacing();
                ImGui::Separator();

                // Status
                const char* status = renderer.isSolved() ? "Solved" : "Unsolved";
                ImU32 statusColor = renderer.isSolved() ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 100, 0, 255);
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Cube status: %s", status);

                ImGui::EndTabItem();
            }

            // Colors tab
            if (ImGui::BeginTabItem("Colors")) {
                ImGui::Separator();

                ImGui::Text("Select custom colors for each face:");
                ImGui::Spacing();

                // Front face color picker (Green by default)
                ImGui::Text("Front:");
                ImGui::ColorEdit3("##FrontColor", &renderer.customFront[0]);

                ImGui::Spacing();

                // Back face color picker (Blue by default)
                ImGui::Text("Back:");
                ImGui::ColorEdit3("##BackColor", &renderer.customBack[0]);

                ImGui::Spacing();

                // Left face color picker (Orange by default)
                ImGui::Text("Left:");
                ImGui::ColorEdit3("##LeftColor", &renderer.customLeft[0]);

                ImGui::Spacing();

                // Right face color picker (Red by default)
                ImGui::Text("Right:");
                ImGui::ColorEdit3("##RightColor", &renderer.customRight[0]);

                ImGui::Spacing();

                // Up face color picker (White by default)
                ImGui::Text("Up:");
                ImGui::ColorEdit3("##UpColor", &renderer.customUp[0]);

                ImGui::Spacing();

                // Down face color picker (Yellow by default)
                ImGui::Text("Down:");
                ImGui::ColorEdit3("##DownColor", &renderer.customDown[0]);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Apply button
                if (ImGui::Button("Apply", ImVec2(180, 0))) {
                    renderer.useCustomColors = true;
                }

                ImGui::SameLine();

                // Reset to Default button
                if (ImGui::Button("Reset to Default", ImVec2(180, 0))) {
                    // Reset colors to standard Rubik's cube colors
                    renderer.customFront = {0.0f, 1.0f, 0.0f};  // Green
                    renderer.customBack = {0.0f, 0.0f, 1.0f};   // Blue
                    renderer.customLeft = {1.0f, 0.5f, 0.0f};  // Orange
                    renderer.customRight = {1.0f, 0.0f, 0.0f}; // Red
                    renderer.customUp = {1.0f, 1.0f, 1.0f};    // White
                    renderer.customDown = {1.0f, 1.0f, 0.0f};  // Yellow
                    renderer.useCustomColors = false;
                }

                ImGui::Spacing();

                // Display current mode
                if (renderer.useCustomColors) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Mode: Using custom colors");
                } else {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Mode: Using default colors");
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

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
