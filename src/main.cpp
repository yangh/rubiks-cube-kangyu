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

// Global flag to enable/disable cube dump
bool g_enableDump = false;

// Global variable to store the last scramble sequence
std::string g_lastScramble = "No scramble generated";

void showHelp(const char* programName) {
    std::cout << "Rubik's Cube Simulator\n\n";
    std::cout << "Usage: " << programName << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -d, --dump    Enable cube state dump to console\n";
    std::cout << "  -h, --help    Show this help message\n\n";
    std::cout << "Keyboard Shortcuts:\n";
    std::cout << "  U/D/L/R/F/B  - Execute corresponding face move (clockwise)\n";
    std::cout << "  Shift+Key    - Execute prime move (counter-clockwise)\n";
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
    GLFWwindow* window = glfwCreateWindow(1400, 800, "Rubik's Cube Simulator", nullptr, nullptr);
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

    // Scale up default system font
    io.FontGlobalScale = 1.3f;

    // Initialize cube renderer
    CubeRenderer renderer;

    // Dump initial cube state if enabled
    if (g_enableDump) {
        std::cout << "\n=== Initial Cube State ===" << std::endl;
        renderer.dump();
    }

    // Window dimensions
    int sidebarWidth = 400;

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
                renderer.rotationY -= io.MouseDelta.x * 0.2f;
                renderer.rotationX += io.MouseDelta.y * 0.2f;
            }
            // Right mouse button: rotate around Z axis
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                renderer.rotationZ += io.MouseDelta.x * 0.3f;
            }
            // Mouse wheel: also rotate around Z axis
            if (io.MouseWheel != 0.0f) {
                renderer.rotationZ += io.MouseWheel * 15.0f;
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
                // Scramble button
                if (ImGui::Button("Scramble", ImVec2(120, 0))) {
                    // Disable animation for instant scramble
                    bool oldAnimation = renderer.enableAnimation;
                    renderer.enableAnimation = false;

                    // Generate and execute scramble
                    std::vector<Move> scrambleMoves = renderer.scramble(20);

                    // Restore animation setting
                    renderer.enableAnimation = oldAnimation;

                    // Build scramble sequence string for display
                    std::ostringstream ss;
                    for (size_t i = 0; i < scrambleMoves.size(); ++i) {
                        ss << moveToString(scrambleMoves[i]);
                        if (i < scrambleMoves.size() - 1) {
                            ss << " ";
                        }
                    }
                    g_lastScramble = ss.str();
                }
                ImGui::SameLine();

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

                ImGui::EndTabItem();
            }

            // History tab
            if (ImGui::BeginTabItem("History")) {
                ImGui::Separator();

                // Undo button
                bool canUndo = !renderer.getMoveHistory().empty();
                if (canUndo) {
                    if (ImGui::Button("Undo Last Move", ImVec2(150, 0))) {
                        renderer.undo();
                    }
                } else {
                    // Disabled button when no history
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    ImGui::Button("Undo Last Move", ImVec2(150, 0));
                    ImGui::PopStyleVar();
                    ImGui::PopItemFlag();
                }

                ImGui::SameLine();

                // Redo button
                bool canRedo = renderer.canRedo();
                if (canRedo) {
                    if (ImGui::Button("Redo Next Move", ImVec2(150, 0))) {
                        renderer.redo();
                    }
                } else {
                    // Disabled button when no redo history
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    ImGui::Button("Redo Next Move", ImVec2(150, 0));
                    ImGui::PopStyleVar();
                    ImGui::PopItemFlag();
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                    "Total moves: %zu", renderer.getMoveHistory().size());
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                    "Redo available: %zu", renderer.getRedoHistory().size());

                ImGui::Separator();

                // Display move history in a scrollable list
                const std::vector<Move>& history = renderer.getMoveHistory();
                if (!history.empty()) {
                    ImGui::BeginChild("MoveHistory", ImVec2(0, 150), true,
                                    ImGuiWindowFlags_HorizontalScrollbar);

                    // Group moves in sets of 6 for better readability
                    for (size_t i = 0; i < history.size(); ++i) {
                        ImGui::SameLine(0, 2);

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

                        ImGui::TextColored(moveColor, "%zu. %s", i + 1,
                                        moveToString(history[i]).c_str());
                    }

                    // Auto-scroll to bottom
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

            ImGui::EndTabBar();
        }

        ImGui::Separator();

        // Keyboard shortcuts help
        ImGui::Text("Keyboard Shortcuts:");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  U/D/L/R/F/B - Move (clockwise)");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Shift+Key - Prime move (counter-clockwise)");

        ImGui::Separator();

        // 2D view controls
        ImGui::Text("2D View Controls:");
        ImGui::SliderFloat("2D Scale", &renderer.scale2D, 0.3f, 3.0f, "%.2f");

        ImGui::Separator();

        // 3D view controls
        ImGui::Text("3D View Controls:");
        ImGui::SliderFloat("Rotation X", &renderer.rotationX, -180.0f, 180.0f);
        ImGui::SliderFloat("Rotation Y", &renderer.rotationY, -180.0f, 180.0f);
        ImGui::SliderFloat("Rotation Z", &renderer.rotationZ, -180.0f, 180.0f);
        ImGui::SliderFloat("3D Scale", &renderer.scale, 2.0f, 7.0f, "%.2f");

        ImGui::Separator();

        // Reset button
        if (ImGui::Button("Reset Cube", ImVec2(120, 0))) {
            renderer.reset();
            renderer.resetView();
        }

        ImGui::Separator();

        // Status
        const char* status = renderer.isSolved() ? "Solved" : "Unsolved";
        ImU32 statusColor = renderer.isSolved() ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 100, 0, 255);
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Cube status: %s", status);

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
