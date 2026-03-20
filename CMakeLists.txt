cmake_minimum_required(VERSION 3.15)
project(RubiksCube VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(glsl_to_header GLSL_FILE VAR_NAME OUTPUT_NAME OUTPUT_DIR)
    file(READ ${GLSL_FILE} CONTENT)
    string(REPLACE "\\" "\\\\" CONTENT "${CONTENT}")
    string(REPLACE "\"" "\\\"" CONTENT "${CONTENT}")
    string(REPLACE "\n" "\\n\"\n\"" CONTENT "${CONTENT}")
    set(HEADER_FILE "${OUTPUT_DIR}/${OUTPUT_NAME}.glsl.h")
    file(WRITE ${HEADER_FILE}
        "#pragma once\n"
        "static const char* ${VAR_NAME} =\n\"${CONTENT}\";\n"
    )
endfunction()

set(GLSL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/glsl")
file(MAKE_DIRECTORY ${GLSL_OUTPUT_DIR})

glsl_to_header("${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/cubie.vert.glsl" "cubieVertShader" "cubie.vert" ${GLSL_OUTPUT_DIR})
glsl_to_header("${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/cubie.frag.glsl" "cubieFragShader" "cubie.frag" ${GLSL_OUTPUT_DIR})

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Add ImGUI subdirectory
add_subdirectory(third_party)

# Create main executable
add_executable(rubiks-cube
    src/main.cpp
    src/app.cpp
    src/cube.cpp
    src/move.cpp
    src/animator.cpp
    src/renderer.cpp
    src/renderer_2d.cpp
    src/renderer_3d_shader.cpp
    src/renderer_3d_opengl.cpp
    src/color.cpp
    src/shader.cpp
    src/model.cpp
    src/formula.cpp
    src/config.cpp
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
    ${GLSL_OUTPUT_DIR}
)

# Add ImGLAD loader from ImGUI (the OpenGL loader included in ImGUI backends)
target_link_libraries(rubiks-cube PRIVATE ${CMAKE_DL_LIBS})

# Enable FetchContent
include(FetchContent)

# Download ImGui
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.h")
    FetchContent_Declare(
        ImGui
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
    )
else()
    FetchContent_Declare(
        ImGui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.6
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
        GIT_PROGRESS ON
        GIT_SHALLOW 1
    )
endif()

FetchContent_MakeAvailable(ImGui)

# Enable testing
enable_testing()

# Create test executable
add_executable(test_cube
    tests/test_cube.cpp
    src/cube.cpp
    src/color.cpp
    src/move.cpp
)

target_include_directories(test_cube
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add test
add_test(NAME cube_logic COMMAND test_cube)

# Create 2-step move combination test executable
add_executable(test_cube_2step
    tests/test_cube_2step.cpp
    src/cube.cpp
    src/color.cpp
    src/move.cpp
)

target_include_directories(test_cube_2step
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add 2-step test
add_test(NAME cube_2step_combos COMMAND test_cube_2step)

# Create reference verification test executable
add_executable(test_ref_verify
    tests/test_ref_verify.cpp
    src/cube.cpp
    src/color.cpp
    src/move.cpp
    tests/ref/ref_cube.cpp
)

target_include_directories(test_ref_verify
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ref
)

# Add reference verification test
add_test(NAME reference_verify COMMAND test_ref_verify)

# Create formula verification test executable
add_executable(test_formula
    tests/test_formula.cpp
    src/cube.cpp
    src/color.cpp
    src/move.cpp
    src/formula.cpp
)

target_include_directories(test_formula
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add formula verification test
add_test(NAME formula_verify COMMAND test_formula)

# Create formula vs reference test executable
add_executable(test_formula_ref
    tests/test_formula_ref.cpp
    src/cube.cpp
    src/color.cpp
    src/move.cpp
    src/formula.cpp
    tests/ref/ref_cube.cpp
)

target_include_directories(test_formula_ref
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ref
)

# Add formula vs reference test
add_test(NAME formula_ref_verify COMMAND test_formula_ref)

# Create axis rotation test executable
add_executable(test_axis_rotate
    tests/test_axis_rotate.cpp
    src/cube.cpp
    src/color.cpp
    src/move.cpp
)

target_include_directories(test_axis_rotate
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add axis rotation test
add_test(NAME axis_rotate COMMAND test_axis_rotate)

# Create color validation test executable
add_executable(test_color_validation
    tests/test_color_validation.cpp
    src/cube.cpp
    src/color.cpp
    src/move.cpp
)

target_include_directories(test_color_validation
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add color validation test
add_test(NAME color_validation COMMAND test_color_validation)
