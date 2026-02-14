cmake_minimum_required(VERSION 3.15)
project(RubiksCube VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Add ImGUI subdirectory
add_subdirectory(third_party)

# Create executable
add_executable(rubiks-cube
    src/main.cpp
    src/cube.cpp
    src/renderer.cpp
)

# Link libraries
target_link_libraries(rubiks-cube
    PRIVATE
    OpenGL::GL
    glfw
    imgui::imgui
)

# Include directories
target_include_directories(rubiks-cube
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add ImGLAD loader from ImGUI (the OpenGL loader included in ImGUI backends)
target_link_libraries(rubiks-cube PRIVATE ${CMAKE_DL_LIBS})
