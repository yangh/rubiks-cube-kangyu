#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

#include "cube.h"
#include "renderer.h"

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Rubik's Cube Simulator", nullptr, nullptr);
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

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create main window
        ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_FirstUseEver);
        ImGui::Begin("Rubik's Cube Simulator", nullptr, ImGuiWindowFlags_MenuBar);

        // Menu bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Reset")) {
                    renderer.reset();
                }
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, 1);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Split into two columns: controls on left, cube view on right
        ImGui::Columns(2, nullptr, true);

        // Left column: Controls
        {
            ImGui::Text("Move Controls:");
            ImGui::Separator();

            if (ImGui::Button("R", ImVec2(40, 0))) renderer.executeMove(Move::R);
            ImGui::SameLine();
            if (ImGui::Button("R'", ImVec2(40, 0))) renderer.executeMove(Move::RP);
            ImGui::SameLine(0, 20);
            if (ImGui::Button("L", ImVec2(40, 0))) renderer.executeMove(Move::L);
            ImGui::SameLine();
            if (ImGui::Button("L'", ImVec2(40, 0))) renderer.executeMove(Move::LP);

            if (ImGui::Button("U", ImVec2(40, 0))) renderer.executeMove(Move::U);
            ImGui::SameLine();
            if (ImGui::Button("U'", ImVec2(40, 0))) renderer.executeMove(Move::UP);
            ImGui::SameLine(0, 20);
            if (ImGui::Button("D", ImVec2(40, 0))) renderer.executeMove(Move::D);
            ImGui::SameLine();
            if (ImGui::Button("D'", ImVec2(40, 0))) renderer.executeMove(Move::DP);

            if (ImGui::Button("F", ImVec2(40, 0))) renderer.executeMove(Move::F);
            ImGui::SameLine();
            if (ImGui::Button("F'", ImVec2(40, 0))) renderer.executeMove(Move::FP);
            ImGui::SameLine(0, 20);
            if (ImGui::Button("B", ImVec2(40, 0))) renderer.executeMove(Move::B);
            ImGui::SameLine();
            if (ImGui::Button("B'", ImVec2(40, 0))) renderer.executeMove(Move::BP);

            ImGui::Separator();

            // View controls
            ImGui::Text("View Controls:");
            ImGui::SliderFloat("Rotation X", &renderer.rotationX, -180.0f, 180.0f);
            ImGui::SliderFloat("Rotation Y", &renderer.rotationY, -180.0f, 180.0f);
            ImGui::SliderFloat("Scale", &renderer.scale, 0.3f, 2.0f, "%.2f");

            ImGui::Separator();

            // Reset button
            if (ImGui::Button("Reset Cube", ImVec2(120, 0))) {
                renderer.reset();
            }

            ImGui::Separator();

            // Status
            const char* status = renderer.isSolved() ? "Solved" : "Unsolved";
            ImGui::Text("Cube status: %s", status);

            // Add spacing at bottom
            ImGui::Spacing();
            ImGui::Spacing();
        }

        ImGui::NextColumn();

        // Right column: Cube view
        {
            // Get content region for cube view
            ImVec2 content_region = ImGui::GetContentRegionAvail();

            // Create child window for drawing
            ImGui::BeginChild("CubeViewport", content_region, true);

            // Get draw list
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // Calculate center position
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            ImVec2 size = ImGui::GetContentRegionAvail();
            ImVec2 center(cursor.x + size.x / 2.0f, cursor.y + size.y / 2.0f);

            // Draw the cube using ImGui's draw list
            renderer.draw(drawList, center, renderer.scale);

            // Add dummy space
            ImGui::Dummy(size);

            ImGui::EndChild();
        }

        ImGui::Columns(1);
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
