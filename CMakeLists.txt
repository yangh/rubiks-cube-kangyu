cmake_minimum_required(VERSION 3.15)
project(RubiksCube VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Add ImGUI subdirectory
add_subdirectory(third_party)

# Create main executable
add_executable(rubiks-cube
    src/main.cpp
    src/cube.cpp
    src/renderer.cpp
    src/shader.cpp
    src/model.cpp
    src/formula.cpp
)

# Link libraries for main executable
target_link_libraries(rubiks-cube
    PRIVATE
    OpenGL::GL
    glfw
    imgui::imgui
    glm::glm
)

# Include directories
target_include_directories(rubiks-cube
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add ImGLAD loader from ImGUI (the OpenGL loader included in ImGUI backends)
target_link_libraries(rubiks-cube PRIVATE ${CMAKE_DL_LIBS})

# Enable testing
enable_testing()

# Create test executable
add_executable(test_cube
    tests/test_cube.cpp
    src/cube.cpp
)

target_include_directories(test_cube
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add test
add_test(NAME cube_logic COMMAND test_cube)