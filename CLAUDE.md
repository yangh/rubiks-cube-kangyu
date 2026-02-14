# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A 3D Rubik's cube simulator built with C++, ImGUI (Dear ImGui), and OpenGL. The cube can be rotated using standard Rubik's cube notation formulas.

## Technology Stack

- **C++**: Primary programming language (C++17)
- **ImGUI**: Immediate mode GUI library
- **GLFW3**: Window and input management
- **OpenGL**: 3D graphics rendering (via ImGui drawing functions)

## Build Commands

```bash
# Clone ImGUI dependency (first time only)
cd third_party
git -c http.proxy=http://127.0.0.1:20170 clone https://github.com/ocornut/imgui.git

# Build the project
mkdir build && cd build
cmake ..
make

# Run the application
./rubiks-cube
```

Or from project root:
```bash
cmake -S . -B build
make -C build
./build/rubiks-cube
```

## Dependencies

System dependencies to install:
- **Ubuntu/Debian**: `cmake libglfw3-dev libgl1-mesa-dev`
- **Arch Linux**: `cmake glfw mesa`
- **macOS**: `cmake glfw` (via Homebrew)

## Project Structure

```
src/
├── main.cpp      - Application entry point, GLFW/ImGUI setup, main game loop
├── cube.h        - Rubik's cube state, color representation, and move logic
├── cube.cpp      - Cube implementation
├── renderer.h    - ImGui 2D rendering using draw lists
└── renderer.cpp  - Renderer implementation

third_party/
└── imgui/       - ImGUI library (needs to be cloned manually)
```

## Architecture

### cube.cpp/h
- **`RubiksCube` class**: Represents the 3x3x3 cube state with 6 faces (9 stickers each)
- **`Color` enum**: Six standard Rubik's cube colors with RGB conversion
- **`Move` enum**: All 12 standard moves (U, D, L, R, F, B and their primes)
- **Face rotation logic**: Each face rotation includes adjacent face permutations

### renderer.cpp/h
- **`CubeRenderer` class**: Manages cube visualization and rendering state
- Uses ImGui's draw list (ImDrawList) for 2D rendering
- Draws unfolded cube net layout with 6 faces in 2D
- **No OpenGL immediate mode used** - all rendering via ImGui to avoid compatibility issues

### main.cpp
- Initializes GLFW window and OpenGL context
- Sets up ImGUI with OpenGL3 and GLFW backends
- Main event loop: poll events → update ImGui → render
- Layout: Two columns - controls on left, cube view on right

## Important Implementation Notes

### Rendering Approach
The application uses **ImGui's drawing functions exclusively**, not OpenGL immediate mode. This avoids compatibility issues with OpenGL Core Profile.

- Uses `ImDrawList->AddQuadFilled()` for filled shapes
- Uses `ImDrawList->AddQuad()` for outlined shapes
- Colors converted to `IM_COL32()` format (ABGR)

### Face Layout
The cube is displayed as an unfolded 2D net:
```
    Up
Left Front Right  Back
    Down
```

Important: Draw order matters to avoid overlap. Faces are drawn with explicit offsets:
- Front: (0, 0) - center
- Up: (0, -offset) - above Front
- Down: (0, +offset) - below Front
- Left: (-offset, 0) - left of Front
- Right: (+offset, 0) - right of Front
- Back: (+offset * 2, 0) - right of Right

### Color Scheme
- Front: Red
- Back: Orange
- Left: Green
- Right: Blue
- Up: White
- Down: Yellow

## Rubik's Cube Notation Reference

Standard moves:
- **U**: Up face clockwise
- **U'**: Up face counter-clockwise
- **D**: Down face clockwise
- **D'**: Down face counter-clockwise
- **L**: Left face clockwise
- **L'**: Left face counter-clockwise
- **R**: Right face clockwise
- **R'**: Right face counter-clockwise
- **F**: Front face clockwise
- **F'**: Front face counter-clockwise
- **B**: Back face clockwise
- **B'**: Back face counter-clockwise

## Known Issues / Limitations

- 2D unfolded view only (no 3D cube visualization)
- Rotation X/Y sliders currently unused (reserved for future 3D view)
