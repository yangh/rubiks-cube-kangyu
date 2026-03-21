#define GLFW_INCLUDE_NONE
#include "app.h"
#include "config.h"
#include "animator.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <cmath>

#include "gl_loader.h"

Application::~Application() {
    renderer_.reset();

    if (window_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

int Application::run() {
    if (!initGlfw()) {
        return -1;
    }

    if (!initImGui()) {
        return -1;
    }

    loadFonts();
    initApp();

    // Dump initial cube state if enabled
    if (this->enableDump_) {
        std::cout << "\n=== Initial Cube State ===" << std::endl;
        this->cube_.dump();
    }

    // Main loop
    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(this->window_)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        // Get current window size
        int windowWidth, windowHeight;
        glfwGetWindowSize(this->window_, &windowWidth, &windowHeight);

        // Calculate responsive layout dimensions (using member variables)
        // sidebarWidth_ and netViewHeight_ are defined in app.h

        // Start Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get ImGuiIO for keyboard shortcuts (works globally, even when mouse is over UI)
        ImGuiIO& io = ImGui::GetIO();

        handleKeyboardShortcuts();

        // ===== Window 1: 3D View (Left side) =====
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(windowWidth - sidebarWidth_ - 20, windowHeight - 20), ImGuiCond_Always);
        ImGui::Begin("3D View", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        renderMenuBar();

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
                this->renderer_->viewState_.targetRotationY += io.MouseDelta.x * 0.2f;
                this->renderer_->viewState_.targetRotationX += io.MouseDelta.y * 0.2f;
            }
            // Right mouse button: rotate around Z axis
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                this->renderer_->viewState_.targetRotationZ += io.MouseDelta.x * 0.3f;
            }
            // Mouse wheel: also rotate around Z axis
            if (io.MouseWheel != 0.0f) {
                this->renderer_->viewState_.targetRotationZ += io.MouseWheel * 15.0f;
            }
        }

        // Update animation
        this->renderer_->updateAnimation(deltaTime);

        // Draw light blue background for 3D view
        ImVec2 bgMin = cursor;
        ImVec2 bgMax = ImVec2(cursor.x + size.x, cursor.y + size.y);
        ImU32 lightBlue = IM_COL32(217, 235, 255, 64);  // Light blue RGB (0.85, 0.92, 1.0)
        drawList->AddRectFilled(bgMin, bgMax, lightBlue);

        // Add dummy space
        ImGui::Dummy(size);

        showAbout();

        ImGui::End();

        // ===== Window 2: 2D Unfolded View (Top Right) =====
        // Responsive height based on window size (25% of window height, min 250px)
        ImGui::SetNextWindowPos(ImVec2(windowWidth - sidebarWidth_ + 10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth_ - 20, netViewHeight_), ImGuiCond_Always);
        ImGui::Begin("2D Net View", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        drawList = ImGui::GetWindowDrawList();
        cursor = ImGui::GetCursorScreenPos();
        size = ImGui::GetContentRegionAvail();
        center = ImVec2(cursor.x + size.x / 2.0f, cursor.y + size.y / 2.0f);

        // Handle mouse wheel for 2D view zoom
        if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
            this->renderer_->viewState_.scale2D += io.MouseWheel * 0.2f;
            // Clamp scale2D between 0.3f and 3.0f
            this->renderer_->viewState_.scale2D = fmaxf(0.3f, fminf(3.0f, this->renderer_->viewState_.scale2D));
        }

        // Draw 2D unfolded cube
        this->renderer_->draw2D(drawList, center, this->renderer_->viewState_.scale2D);

        ImGui::Dummy(size);
        ImGui::End();

        // ===== Window 3: Controls (Bottom Right) =====
        // Start after 2D Net View window (fixed height + 10px gap)
        float controlsY = 10.0f + netViewHeight_ + 10.0f;
        float controlsHeight = windowHeight - controlsY - 10.0f;
        ImGui::SetNextWindowPos(ImVec2(windowWidth - sidebarWidth_ + 10, controlsY), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth_ - 20, controlsHeight), ImGuiCond_Always);
        ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        renderControls();

        ImGui::End();

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(this->window_, &display_w, &display_h);

        // Clear the screen
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render ImGui first (2D interface)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        this->renderer_->render3DOverlay(display_w, display_h, sidebarWidth_);

        glfwSwapBuffers(this->window_);
    }

    return 0;
}

bool Application::initGlfw() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure GLFW (OpenGL 3.3 for modern shader-based rendering)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    // Enable multisampling for anti-aliasing (8 samples)
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Create window - wider to accommodate T-shaped layout
    this->window_ = glfwCreateWindow(1400, 900, "Rubik's Cube Simulator", nullptr, nullptr);
    if (!this->window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(this->window_);

    if (!GL_LOADER.load(this->window_)) {
        std::cerr << "Failed to initialize OpenGL functions" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSwapInterval(1);
    glEnable(GL_MULTISAMPLE);

    return true;
}

bool Application::initImGui() {
    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui::GetStyle().FrameRounding = 4.0f;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(this->window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

void Application::loadFonts() {
    ImGuiIO& io = ImGui::GetIO();

    // Load Chinese font (Noto Sans CJK)
    // Try multiple font paths in case one is not available
    const char* fontPaths[] = {
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/arphic/uming.ttc",
        "/usr/share/fonts/truetype/arphic/ukai.ttc",
        "./data/NotoSansCJK-Regular.ttc",
        "../data/NotoSansCJK-Regular.ttc",
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
}

void Application::initApp() {
    this->renderer_ = std::make_unique<CubeRenderer>(this->cube_);

    // Load color configuration from file
    ColorConfig config = loadColorConfig();
    this->renderer_->setCustomColors(config);

    // Load animation settings from config
    this->renderer_->animator_.enableAnimation = config.getEnableAnimation();
    this->renderer_->animator_.animationSpeed = config.getAnimationSpeed();
    this->renderer_->animator_.easingType = static_cast<EasingType>(config.getEasingType());

    // Load renderer type and switch to it
    this->renderer_->switchRenderer(config.getRendererType());

    if (!config.isUsingDefaults()) {
        std::cout << "Loaded custom configuration from: " << getConfigFilePath() << std::endl;
    }

    // Load formulas from directory
    this->formulaManager_.loadFormulas();
}

void Application::handleKeyboardShortcuts() {
    ImGuiIO& io = ImGui::GetIO();

    // Handle keyboard shortcuts for cube moves
    // These work globally, even when mouse is over UI
    handleMoveShortcut(ImGuiKey_R, Move::R, Move::RP, io);
    handleMoveShortcut(ImGuiKey_U, Move::U, Move::UP, io);
    handleMoveShortcut(ImGuiKey_D, Move::D, Move::DP, io);
    handleMoveShortcut(ImGuiKey_L, Move::L, Move::LP, io);
    handleMoveShortcut(ImGuiKey_F, Move::F, Move::FP, io);
    handleMoveShortcut(ImGuiKey_B, Move::B, Move::BP, io);
    handleMoveShortcut(ImGuiKey_M, Move::M, Move::MP, io);
    handleMoveShortcut(ImGuiKey_E, Move::E, Move::EP, io);
    handleMoveShortcut(ImGuiKey_S, Move::S, Move::SP, io);
    handleMoveShortcut(ImGuiKey_X, Move::X, Move::XP, io);
    handleMoveShortcut(ImGuiKey_Y, Move::Y, Move::YP, io);
    handleMoveShortcut(ImGuiKey_Z, Move::Z, Move::ZP, io);

    // Spacebar: reset view to default angles
    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        this->renderer_->resetView();
        this->renderer_->viewState_.celebrationMode = false;
    }

    // ESC: reset cube to solved state
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        resetStepByStepMode();
        resetCube();
        this->renderer_->viewState_.celebrationMode = false;
    }

    // Ctrl+S: scramble cube
    if (ImGui::IsKeyPressed(ImGuiKey_S) && io.KeyCtrl) {
        scrambleCube();
    }

    // Ctrl+Q: quit application
    if (ImGui::IsKeyPressed(ImGuiKey_Q) && io.KeyCtrl) {
        glfwSetWindowShouldClose(this->window_, 1);
    }

    // Ctrl+Z: undo
    if (ImGui::IsKeyPressed(ImGuiKey_Z) && io.KeyCtrl) {
        if (this->cube_.canUndo()) {
            Move lastMove = this->cube_.getLastMove();
            this->renderer_->executeMove(getInverseMove(lastMove), false);
            this->cube_.undo();
        }
    }

    // Ctrl+R: redo
    if (ImGui::IsKeyPressed(ImGuiKey_R) && io.KeyCtrl) {
        if (this->cube_.canRedo()) {
            Move moveToRedo = this->cube_.getLastRedo();
            this->renderer_->executeMove(moveToRedo, false);
            this->cube_.redo();
        }
    }

    // F11: toggle fullscreen
    if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
        toggleFullscreen();
    }

    // Ctrl+P: toggle celebration mode
    if (ImGui::IsKeyPressed(ImGuiKey_P) && io.KeyCtrl) {
        this->renderer_->viewState_.celebrationMode = !this->renderer_->viewState_.celebrationMode;
    }

    // Plus/Minus: adjust cube scale
        if (ImGui::IsKeyPressed(ImGuiKey_Equal) && !io.KeyCtrl) {
            this->renderer_->setCubeScale(std::min(2.0f, this->renderer_->getCubeScale() + 0.1f));
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Minus) && !io.KeyCtrl) {
            this->renderer_->setCubeScale(std::max(0.2f, this->renderer_->getCubeScale() - 0.1f));
        }

        // Ctrl+Plus/Ctrl+Minus: adjust gap
        if (ImGui::IsKeyPressed(ImGuiKey_Equal) && io.KeyCtrl) {
            this->renderer_->setGap(std::min(0.5f, this->renderer_->getGap() + 0.05f));
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Minus) && io.KeyCtrl) {
            this->renderer_->setGap(std::max(0.0f, this->renderer_->getGap() - 0.05f));
        }
}

void Application::handleMoveShortcut(ImGuiKey key, Move normalMove, Move primeMove, ImGuiIO& io) {
    if (ImGui::IsKeyPressed(key)) {
        // Don't execute move if Ctrl is pressed (for Ctrl+Z, Ctrl+R, Ctrl+Q)
        if (!io.KeyCtrl) {
            this->renderer_->executeMove(io.KeyShift ? primeMove : normalMove);
        }
    }
}

void Application::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Reset")) {
                resetCube();
            }
            if (ImGui::MenuItem("About")) {
                this->showAboutDialog_ = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(this->window_, 1);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Application::renderControls() {
    // Tab bar for Moves, Shortcuts, and Formulas
    if (ImGui::BeginTabBar("ControlsTabBar", ImGuiTabBarFlags_None)) {
        renderMovesTab();
        renderFormulasTab();
        renderSettingsTab();
        renderShortcutsTab();
        ImGui::EndTabBar();
    }
}

void Application::renderMovesTab() {
    // Moves tab
    if (ImGui::BeginTabItem("Moves")) {
        // Scramble and Reset buttons
        if (ImGui::Button("Scramble", ImVec2(160, 0))) {
            scrambleCube();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Cube", ImVec2(160, 0))) {
            resetStepByStepMode();
            resetCube();
        }

        ImGui::Separator();

        // Move buttons (3 rows: 6 buttons each)

        // Row 1: R, L, M, X
        if (ImGui::Button("R", ImVec2(40, 0))) this->renderer_->executeMove(Move::R);
        ImGui::SameLine();
        if (ImGui::Button("R'", ImVec2(40, 0))) this->renderer_->executeMove(Move::RP);
        ImGui::SameLine(0, 20);
        if (ImGui::Button("L", ImVec2(40, 0))) this->renderer_->executeMove(Move::L);
        ImGui::SameLine();
        if (ImGui::Button("L'", ImVec2(40, 0))) this->renderer_->executeMove(Move::LP);
        ImGui::SameLine(0, 20);
        if (ImGui::Button("M", ImVec2(40, 0))) this->renderer_->executeMove(Move::M);
        ImGui::SameLine();
        if (ImGui::Button("M'", ImVec2(40, 0))) this->renderer_->executeMove(Move::MP);
        ImGui::SameLine(0, 20);
        if (ImGui::Button("X", ImVec2(40, 0))) this->renderer_->executeMove(Move::X);
        ImGui::SameLine();
        if (ImGui::Button("X'", ImVec2(40, 0))) this->renderer_->executeMove(Move::XP);

        // Row 2: U, D, E, Y
        if (ImGui::Button("U", ImVec2(40, 0))) this->renderer_->executeMove(Move::U);
        ImGui::SameLine();
        if (ImGui::Button("U'", ImVec2(40, 0))) this->renderer_->executeMove(Move::UP);
        ImGui::SameLine(0, 20);
        if (ImGui::Button("D", ImVec2(40, 0))) this->renderer_->executeMove(Move::D);
        ImGui::SameLine();
        if (ImGui::Button("D'", ImVec2(40, 0))) this->renderer_->executeMove(Move::DP);
        ImGui::SameLine(0, 20);
        if (ImGui::Button("E", ImVec2(40, 0))) this->renderer_->executeMove(Move::E);
        ImGui::SameLine();
        if (ImGui::Button("E'", ImVec2(40, 0))) this->renderer_->executeMove(Move::EP);
        ImGui::SameLine(0, 20);
        if (ImGui::Button("Y", ImVec2(40, 0))) this->renderer_->executeMove(Move::Y);
        ImGui::SameLine();
        if (ImGui::Button("Y'", ImVec2(40, 0))) this->renderer_->executeMove(Move::YP);

        // Row 3: F, B, S, Z
        if (ImGui::Button("F", ImVec2(40, 0))) this->renderer_->executeMove(Move::F);
        ImGui::SameLine();
        if (ImGui::Button("F'", ImVec2(40, 0))) this->renderer_->executeMove(Move::FP);
        ImGui::SameLine(0, 20);
        if (ImGui::Button("B", ImVec2(40, 0))) this->renderer_->executeMove(Move::B);
        ImGui::SameLine();
        if (ImGui::Button("B'", ImVec2(40, 0))) this->renderer_->executeMove(Move::BP);
        ImGui::SameLine(0, 20);
        if (ImGui::Button("S", ImVec2(40, 0))) this->renderer_->executeMove(Move::S);
        ImGui::SameLine();
        if (ImGui::Button("S'", ImVec2(40, 0))) this->renderer_->executeMove(Move::SP);
        ImGui::SameLine(0, 20);
        if (ImGui::Button("Z", ImVec2(40, 0))) this->renderer_->executeMove(Move::Z);
        ImGui::SameLine();
        if (ImGui::Button("Z'", ImVec2(40, 0))) this->renderer_->executeMove(Move::ZP);

        ImGui::Spacing();
        ImGui::Separator();

        // Undo, Redo, and Copy buttons (grouped together)
        bool canUndo = this->cube_.canUndo();
        bool canRedo = this->cube_.canRedo();

        // Undo button
        if (canUndo) {
            if (ImGui::Button("Undo", ImVec2(100, 0))) {
                Move lastMove = this->cube_.getLastMove();
                Move inverseMove = getInverseMove(lastMove);
                this->renderer_->animator_.queueMove(inverseMove, false);
                this->cube_.undo();
            }
        } else {
            drawDisabledButton("Undo", ImVec2(100, 0));
        }

        ImGui::SameLine();

        // Redo button
        if (canRedo) {
            if (ImGui::Button("Redo", ImVec2(100, 0))) {
                Move moveToRedo = this->cube_.getRedoHistory().back();
                this->renderer_->animator_.queueMove(moveToRedo, false);
                this->cube_.redo();
            }
        } else {
            drawDisabledButton("Redo", ImVec2(100, 0));
        }

        ImGui::SameLine();

        // Copy button
        if (canUndo) {
            if (ImGui::Button("Copy", ImVec2(100, 0))) {
                const std::vector<Move>& history = this->cube_.getMoveHistory();
                glfwSetClipboardString(this->window_, buildMoveHistoryString().c_str());
            }
        } else {
            drawDisabledButton("Copy", ImVec2(100, 0));
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "Total moves: %zu", this->cube_.getMoveHistory().size());
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "Redo available: %zu", this->cube_.getRedoHistory().size());

        ImGui::Separator();

        // Display move history as auto-wrapped text
        const std::vector<Move>& history = this->cube_.getMoveHistory();
        if (!history.empty()) {
            std::string historyStr = buildMoveHistoryString();
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
        const char* status = this->cube_.isSolved() ? "Solved" : "Unsolved";
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Cube status: %s", status);

        ImGui::EndTabItem();
    }
}

void Application::renderFormulasTab() {
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
            std::vector<std::string> fileNames = this->formulaManager_.getFileNames();

            if (fileNames.empty()) {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                    "No formulas found in 'formula' directory");
            } else {
                for (const auto& filename : fileNames) {
                    const bool isSelected = (this->formulaManager_.getSelectedFileName() == filename);

                    if (ImGui::Selectable(filename.c_str(), isSelected)) {
                        this->formulaManager_.setSelectedFile(filename);
                        this->formulaInputDirty_ = true;
                    }
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Right column: Formula items in selected file
        ImGui::BeginChild("FormulaItemList", ImVec2(rightColumnWidth, listHeight), true);
        {
            const std::vector<FormulaItem>* items = this->formulaManager_.getSelectedFileItems();

            if (items != nullptr && !items->empty()) {
                for (size_t i = 0; i < items->size(); ++i) {
                    const bool isSelected = (this->formulaManager_.getSelectedItemIndex() == static_cast<int>(i));

                    if (ImGui::Selectable(items->at(i).name.c_str(), isSelected)) {
                        this->formulaManager_.setSelectedItem(i);
                        this->formulaInputDirty_ = true;  // Mark input for update
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
        const FormulaItem* selectedItem = this->formulaManager_.getSelectedItem();

        ImGui::Spacing();
        if (selectedItem != nullptr && !selectedItem->moves.empty()) {
            // Update input buffer when formula selection changes
            if (this->formulaInputDirty_) {
                // Build formula string from moves
                std::string formulaStr;
                for (size_t i = 0; i < selectedItem->moves.size(); ++i) {
                    if (i > 0) formulaStr += " ";
                    formulaStr += moveToString(selectedItem->moves[i]);
                }
                snprintf(this->formulaInput_, sizeof(this->formulaInput_), "%s", formulaStr.c_str());
                this->formulaInputDirty_ = false;
            }

            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Selected: %s",
                selectedItem->name.c_str());
            ImGui::Separator();
        }

        // Editable input text box for formula (always visible)
        if (ImGui::InputText("##FormulaInput", this->formulaInput_, sizeof(this->formulaInput_),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            // User pressed Enter, re-parse the formula
            this->formulaInputDirty_ = false;
        }

        // Display move count
        std::vector<Move> parsedMoves = parseMoveSequence(this->formulaInput_);
        // Show step progress if in step-by-step mode
        if (this->isStepByStepMode_) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f),
                "Total moves: %zu (Step: %d/%zu)",
                parsedMoves.size(), this->currentStepIndex_, this->stepByStepMoves_.size());
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
        std::vector<Move> movesFromInput = parseMoveSequence(this->formulaInput_);
        bool hasFormula = !movesFromInput.empty();

        if (hasFormula) {
            // Execute button - automatically loops if formula has loop syntax
            if (ImGui::Button("Execute", ImVec2(180, 0))) {
                resetStepByStepMode();

                // Parse moves from input
                std::vector<Move> moves = parseMoveSequence(this->formulaInput_);

                // Execute formula loopCount times if formula has loop syntax, otherwise execute once
                int loopCount = (selectedItem != nullptr && selectedItem->loopCount > 0) ? selectedItem->loopCount : 1;
                for (int i = 0; i < loopCount; ++i) {
                    for (const Move& move : moves) {
                        this->renderer_->executeMove(move);
                    }
                }
            }

            ImGui::SameLine();

            // Execute Reverse button
            if (ImGui::Button("Execute Reverse", ImVec2(180, 0))) {
                resetStepByStepMode();

                // Parse and execute moves in reverse order with inverse moves
                std::vector<Move> moves = parseMoveSequence(this->formulaInput_);
                for (auto it = moves.rbegin(); it != moves.rend(); ++it) {
                    this->renderer_->executeMove(getInverseMove(*it));
                }
            }

            ImGui::Separator();

            // Step button
            bool canStep = hasFormula && (!this->isStepByStepMode_ || this->currentStepIndex_ < static_cast<int>(this->stepByStepMoves_.size()));
            if (canStep) {
                if (ImGui::Button("Step", ImVec2(180, 0))) {
                    // Start step-by-step mode if not already active
                    if (!this->isStepByStepMode_) {
                        this->stepByStepMoves_ = parseMoveSequence(this->formulaInput_);
                        this->currentStepIndex_ = 0;
                        this->isStepByStepMode_ = true;
                    }

                    // Execute current step
                    if (this->currentStepIndex_ < static_cast<int>(this->stepByStepMoves_.size())) {
                        this->renderer_->executeMove(this->stepByStepMoves_[this->currentStepIndex_]);
                        this->currentStepIndex_++;

                        // Exit step mode when complete
                        if (this->currentStepIndex_ >= static_cast<int>(this->stepByStepMoves_.size())) {
                            this->isStepByStepMode_ = false;
                        }
                    }
                }
            } else {
                drawDisabledButton("Step", ImVec2(180, 0));
            }

            ImGui::SameLine();

            // Reset Step Mode button - only show when in step-by-step mode
            if (this->isStepByStepMode_) {
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
            this->formulaManager_.refresh();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Cube", ImVec2(180, 0))) {
            resetStepByStepMode();
            resetCube();
        }

        ImGui::EndTabItem();
    }
}

void Application::renderSettingsTab() {
    // Settings tab (consolidated View + Animation)
    if (ImGui::BeginTabItem("Settings")) {
        ImGui::Separator();

        // Animation section
        ImGui::Text("Animation:");
        bool prevEnableAnim = this->renderer_->animator_.enableAnimation;
        float prevAnimSpeed = this->renderer_->animator_.animationSpeed;
        ImGui::Checkbox("Enable Animation", &this->renderer_->animator_.enableAnimation);
        ImGui::SliderFloat("Speed", &this->renderer_->animator_.animationSpeed, 0.1f, 3.0f, "%.1fx", ImGuiSliderFlags_Logarithmic);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("1.0x = 200ms per move");
        }

        int prevEasingType = static_cast<int>(this->renderer_->animator_.easingType);
        const char* easingTypes[] = { "Smooth Step", "Ease-Out Cubic", "Ease-Out Back" };
        if (ImGui::Combo("Easing", &prevEasingType, easingTypes, IM_ARRAYSIZE(easingTypes))) {
            this->renderer_->animator_.easingType = static_cast<EasingType>(prevEasingType);
            saveRendererConfig();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Smoothstep: smooth start/end\nEase-Out Cubic: magnetic feel\nEase-Out Back: subtle overshoot snap");
        }

        // Save animation settings when slider is deactivated (user released it)
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            saveRendererConfig();
        }

        // Also save when checkbox changes
        if (this->renderer_->animator_.enableAnimation != prevEnableAnim) {
            saveRendererConfig();
        }

        ImGui::Spacing();

        // Renderer selection
        RendererType prevRendererType = this->renderer_->getRendererType();
        const char* rendererTypes[] = { "OpenGL", "Shader" };
        int comboIndex = static_cast<int>(prevRendererType);
        if (ImGui::Combo("Renderer", &comboIndex, rendererTypes, IM_ARRAYSIZE(rendererTypes))) {
            this->renderer_->switchRenderer(static_cast<RendererType>(comboIndex));
            saveRendererConfig();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("OpenGL: Fixed-pipeline renderer\nShader: GLSL 330 shader renderer (restart required for full effect)");
        }

        ImGui::Spacing();
        ImGui::Separator();

        // 2D view controls
        ImGui::Text("2D View Controls:");
        ImGui::SliderFloat("2D Scale", &this->renderer_->viewState_.scale2D, 0.3f, 3.0f, "%.2f");

        ImGui::Spacing();

        // 3D view controls
        ImGui::Text("3D View Controls:");
        ImGui::SliderFloat("Rotation X", &this->renderer_->viewState_.rotationX, -180.0f, 180.0f);
        ImGui::SliderFloat("Rotation Y", &this->renderer_->viewState_.rotationY, -180.0f, 180.0f);
        ImGui::SliderFloat("Rotation Z", &this->renderer_->viewState_.rotationZ, -180.0f, 180.0f);
        ImGui::SliderFloat("3D Scale", &this->renderer_->viewState_.scale3D, 2.0f, 7.0f, "%.2f");

        ImGui::Spacing();
        ImGui::Separator();

        // Custom colors section
        ImGui::Text("Custom Colors:");

        // Color pickers for each face
        addColorPicker("##FrontColor", "Front (Green):", this->renderer_->colorProvider_.customFront);
        addColorPicker("##BackColor", "Back (Blue):", this->renderer_->colorProvider_.customBack);
        addColorPicker("##LeftColor", "Left (Orange):", this->renderer_->colorProvider_.customLeft);
        addColorPicker("##RightColor", "Right (Red):", this->renderer_->colorProvider_.customRight);
        addColorPicker("##UpColor", "Up (White):", this->renderer_->colorProvider_.customUp);
        addColorPicker("##DownColor", "Down (Yellow):", this->renderer_->colorProvider_.customDown);

        ImGui::Spacing();

        // Save and Reset buttons
        // Note: Colors are now auto-saved when modified, but we keep this button for explicit save
        if (ImGui::Button("Save Colors", ImVec2(160, 0))) {
            saveRendererConfig();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset to Defaults", ImVec2(180, 0))) {
            this->renderer_->colorProvider_.resetToDefaults();
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
}

void Application::renderShortcutsTab() {
    // Shortcuts tab
    if (ImGui::BeginTabItem("Shortcuts")) {
        ImVec4 color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

        ImGui::Spacing();
        ImGui::Text("Keyboard Shortcuts:");
        ImGui::TextColored(color, "  U/D/L/R/F/B/M/E/S - Move (clockwise)");
        ImGui::TextColored(color, "  Shift+Key - Prime move (counter-clockwise)");
        ImGui::TextColored(color, "  X/Y/Z - Axis rotation (clockwise)");
        ImGui::TextColored(color, "  Shift+X/Y/Z - Axis rotation (counter-clockwise)");
        ImGui::TextColored(color, "  Space - Reset view to default angles");
        ImGui::TextColored(color, "  ESC - Reset cube to solved state");
        ImGui::TextColored(color, "  Ctrl+S - Scramble cube");
        ImGui::TextColored(color, "  Ctrl+Z - Undo last move");
        ImGui::TextColored(color, "  Ctrl+R - Redo last undone move");
        ImGui::TextColored(color, "  Ctrl+Q - Quit application");
        ImGui::TextColored(color, "  F11 - Toggle fullscreen mode");

        ImGui::EndTabItem();
    }
}

void Application::showAbout() {
    if (!this->showAboutDialog_) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(
        ImVec2(ImGui::GetIO().DisplaySize.x * 2 / 3, ImGui::GetIO().DisplaySize.y / 2),
        ImGuiCond_Appearing,
        ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowFocus();

    if (ImGui::Begin("About", &this->showAboutDialog_, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Rubik's Cube Simulator");
        ImGui::Separator();
        ImGui::Text("Version: 1.2.0");
        ImGui::Text("Author: Walter");
        ImGui::Spacing();
        ImGui::TextWrapped("A 3D Rubik's cube simulator built with C++, Dear ImGui, and OpenGL.");
        ImGui::Spacing();
        ImGui::TextWrapped("Dedicated for my mother, Kangyu Qiu. @2026");
        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            this->showAboutDialog_ = false;
        }
        ImGui::End();
    }
}

void Application::resetStepByStepMode() {
    this->isStepByStepMode_ = false;
    this->currentStepIndex_ = 0;
    this->stepByStepMoves_.clear();
}

void Application::saveRendererConfig() {
    ColorConfig config;
    config.setFront(this->renderer_->colorProvider_.customFront);
    config.setBack(this->renderer_->colorProvider_.customBack);
    config.setLeft(this->renderer_->colorProvider_.customLeft);
    config.setRight(this->renderer_->colorProvider_.customRight);
    config.setUp(this->renderer_->colorProvider_.customUp);
    config.setDown(this->renderer_->colorProvider_.customDown);
    config.setEnableAnimation(this->renderer_->animator_.enableAnimation);
    config.setAnimationSpeed(this->renderer_->animator_.animationSpeed);
    config.setEasingType(static_cast<int>(this->renderer_->animator_.easingType));
    config.setRendererType(this->renderer_->getRendererType());
    config.setUsingDefaults(false);
    saveColorConfig(config);
}

void Application::resetCube() {
    this->renderer_->reset();
    this->renderer_->resetView();
}

void Application::scrambleCube() {
    bool oldAnimation = this->renderer_->animator_.enableAnimation;
    this->renderer_->animator_.enableAnimation = false;

    this->cube_.scramble(20);
    this->renderer_->animator_.reset();

    this->renderer_->animator_.enableAnimation = oldAnimation;
}

void Application::toggleFullscreen() {
    if (this->isFullscreen_) {
        // Switch back to windowed mode
        glfwSetWindowMonitor(this->window_, nullptr, this->windowedX_, this->windowedY_, this->windowedWidth_, this->windowedHeight_, 0);
        this->isFullscreen_ = false;
    } else {
        // Save current window position and size
        glfwGetWindowPos(this->window_, &this->windowedX_, &this->windowedY_);
        glfwGetWindowSize(this->window_, &this->windowedWidth_, &this->windowedHeight_);

        // Switch to fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(this->window_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        this->isFullscreen_ = true;
    }
}

std::string Application::buildMoveHistoryString() const {
    std::string result;
    const std::vector<Move>& history = this->cube_.getMoveHistory();
    for (size_t i = 0; i < history.size(); ++i) {
        if (i > 0) result += " ";
        result += moveToString(history[i]);
    }
    return result;
}

void Application::addColorPicker(const char* id, const char* label, std::array<float, 3>& color) {
    if (ImGui::ColorEdit3(id, color.data())) {
        this->renderer_->colorProvider_.useCustomColors = true;
        saveRendererConfig();
    }
    ImGui::SameLine();
    ImGui::Text("%s", label);
}

void Application::drawDisabledButton(const char* label, ImVec2 size) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    ImGui::Button(label, size);
    ImGui::PopStyleVar();
    ImGui::PopItemFlag();
}

void Application::addMoveButton(const char* label, Move move, ImVec2 size) {
    if (ImGui::Button(label, size)) {
        this->renderer_->executeMove(move);
    }
}

void Application::addMoveButtonPair(const char* label, Move normalMove, Move primeMove, ImVec2 size) {
    std::string normalLabel = std::string(label);
    std::string primeLabel = std::string(label) + "'";
    
    addMoveButton(normalLabel.c_str(), normalMove, size);
    ImGui::SameLine();
    addMoveButton(primeLabel.c_str(), primeMove, size);
}
