#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <cmath>
#include <string>
#include <getopt.h>

#include "cube.h"
#include "renderer.h"

// Global flag to enable/disable cube dump
bool g_enableDump = false;

void showHelp(const char* programName) {
    std::cout << "Rubik's Cube Simulator\n\n";
    std::cout << "Usage: " << programName << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -d, --dump    Enable cube state dump to console\n";
    std::cout << "  -h, --help    Show this help message\n";
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

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

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
        ImGuiIO& io = ImGui::GetIO();
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

        // Draw 3D cube
        renderer.draw3D(drawList, center, renderer.scale);

        // Add dummy space
        ImGui::Dummy(size);

        ImGui::End();

        // ===== Window 2: 2D Unfolded View (Top Right) =====
        ImGui::SetNextWindowPos(ImVec2(windowWidth - sidebarWidth + 10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth - 20, (windowHeight - 20) / 2.0f), ImGuiCond_Always);
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
        ImGui::SetNextWindowPos(ImVec2(windowWidth - sidebarWidth + 10, (windowHeight - 20) / 2.0f + 20), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(sidebarWidth - 20, (windowHeight - 20) / 2.0f - 10), ImGuiCond_Always);
        ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Tab bar for Moves and Animation controls
        if (ImGui::BeginTabBar("ControlsTabBar", ImGuiTabBarFlags_None)) {
            // Moves tab
            if (ImGui::BeginTabItem("Moves")) {
                // Move buttons (3 rows: 6 buttons each)
                ImGui::Separator();

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
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
