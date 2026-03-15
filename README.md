# Rubik's Cube Simulator

A 3D Rubik's cube simulator built with C++, [Dear ImGui](https://github.com/ocornut/imgui), and OpenGL with advanced features including animations, formula execution, and undo/redo capabilities.

![Rubik's Cube Screenshot](data/rubiks-cube-kangyu-v1.2.png)

## Version 1.2 Highlights

- **Modular Architecture**: CubeRenderer refactored into separate components (ViewState, ColorProvider, CubeAnimator, Renderer2D, Renderer3DOpenGL)
- **Double Moves**: Full support for U2/D2/L2/R2/F2/B2/M2/E2/S2/X2/Y2/Z2 (180° rotations)
- **Improved 3D Rendering**: Rounded corners on stickers, better viewport centering, circle shadow canvas
- **Configurable Animation Easing**: Choose from linear, ease-in, ease-out, ease-in-out animation curves
- **Cleaner Codebase**: Application class extracted, main.cpp reduced to thin entry point

## Features

### Core Functionality
- Interactive 2D unfolded cube visualization
- Interactive 3D isometric view with mouse controls
- Full set of Rubik's cube moves (U, D, L, R, F, B and their primes, plus slice moves M, E, S)
- Complete axis rotations (X, Y, Z and their primes) for cube orientation
- Real-time cube state tracking and solvable state detection
- Undo/Redo system with move history management
- Scramble function with random move generation

### Advanced Features
- **3D Animation System**: Smooth rotation animations for all moves with adjustable speed
- **Formula System**:
  - Load and execute formulas from files
  - Execute formulas in forward or reverse
  - Step-by-step execution mode
  - Loop syntax support for repeated sequences
- **Customization**:
  - Adjustable 2D and 3D view scales
  - Custom color settings for each face
  - Persistent configuration saving
- **Mouse Controls**:
  - 3D View: Left-click drag (XY rotation), Right-click drag (Z rotation), Scroll wheel (Z rotation + zoom)
  - 2D View: Mouse wheel zoom
- **Keyboard Shortcuts**: Comprehensive keyboard support with Shift+Key for prime moves, fullscreen toggle
- **Settings Persistence**: All preferences saved to config.json

## Requirements

- **CMake**: 3.15 or later
- **C++ Compiler**: Supporting C++17 (GCC, Clang, or MSVC)
- **GLFW3**: For window management
- **OpenGL**: For 3D rendering
- **[Dear ImGui](https://github.com/ocornut/imgui)**: Immediate mode GUI library

### Installing Dependencies

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install cmake libglfw3-dev libgl1-mesa-dev
```

#### Arch Linux:
```bash
sudo pacman -S cmake glfw mesa
```

#### macOS:
```bash
brew install cmake glfw
```

## Building

1. Clone ImGUI (required, first time only):
```bash
cd third_party
git clone https://github.com/ocornut/imgui.git
cd ..
```

2. Build the project:
```bash
mkdir build && cd build
cmake ..
make
```

3. Run the application:
```bash
./rubiks-cube
```

Or from project root:
```bash
cmake -S . -B build
make -C build
./build/rubiks-cube
```

## Usage

### Quick Start
- Use the move buttons (R, R', L, L', etc.) to rotate cube faces
- Adjust Scale sliders to zoom in/out
- Click "Scramble" to generate random moves
- Click "Reset Cube" to return to the solved state
- Use "Undo" and "Redo" buttons to navigate move history
- Click "Copy" to copy move history to clipboard

### Keyboard Shortcuts
- **U/D/L/R/F/B/M/E/S**: Execute corresponding move (clockwise)
- **X/Y/Z**: Execute axis rotation (clockwise)
- **Shift+Key**: Execute prime move (counter-clockwise)
- **Space**: Reset 3D view to default angles
- **ESC**: Reset cube to solved state
- **Ctrl+Z**: Undo last move
- **Ctrl+R**: Redo last undone move
- **Ctrl+S**: Scramble cube
- **Ctrl+Q**: Quit application
- **F11**: Toggle fullscreen mode
- **Example**: 'U' = U move, 'Shift+U' = U' move, 'X' = X axis rotation

### Formula System
1. Create formulas in the 'formula' directory (one file per category)
2. Each file contains multiple formula items with names and move sequences
3. Supports special syntax:
   - Regular moves: "R U R' U'"
   - Loop syntax: "R U R' U'" loop 3 (repeats sequence 3 times)
4. Formula commands:
   - **Execute**: Runs the formula sequence
   - **Execute Reverse**: Runs moves in reverse with inverse
   - **Step**: Executes one move at a time
   - **Reset Step**: Exits step-by-step mode

### Settings and Configuration
- **Animation**: Enable/disable animations and adjust speed (0.1x to 3.0x)
- **Colors**: Customize each face color (persisted to ~/.rubiks-cube/config.json)
- **Views**: Adjust 2D and 3D scale, rotation angles
- **Reset to Defaults**: Restore default colors and settings

## Cube Notation

### Basic Moves
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

### Advanced Slice Moves
- **M**: Middle slice (between L and R) clockwise
- **M'**: Middle slice counter-clockwise
- **E**: Equator slice (between U and D) clockwise
- **E'**: Equator slice counter-clockwise
- **S**: Standing slice (between F and B) clockwise
- **S'**: Standing slice counter-clockwise

### Double Moves (180° Rotation)
- **U2/D2/L2/R2/F2/B2**: 180° rotation of corresponding face
- **M2/E2/S2**: 180° rotation of corresponding slice
- **X2/Y2/Z2**: 180° rotation around corresponding axis

Example: "U2" rotates the Up face 180 degrees (same as "U U").

### Axis Rotations (Whole Cube)
- **X**: Rotate entire cube around X-axis (right-left axis), equivalent to R M' L'
- **X'**: Rotate entire cube counter-clockwise around X-axis
- **Y**: Rotate entire cube around Y-axis (up-down axis), equivalent to U E' D'
- **Y'**: Rotate entire cube counter-clockwise around Y-axis
- **Z**: Rotate entire cube around Z-axis (front-back axis), equivalent to F S B'
- **Z'**: Rotate entire cube counter-clockwise around Z-axis

## Project Structure

```
src/
├── main.cpp              - Thin entry point (64 lines)
├── app.h / app.cpp       - Application class with main loop and UI
├── cube.h / cube.cpp     - Cube state representation and move logic
├── renderer.h / cpp      - CubeRenderer facade class
├── view_state.h / cpp    - View rotation/scale state
├── color_provider.h/cpp  - Color configuration management
├── cube_animator.h/cpp   - Animation controller with easing
├── renderer_2d.h / cpp   - 2D unfolded cube view
├── irenderer_3d.h        - 3D renderer interface
├── renderer_3d_opengl.h/cpp - OpenGL 3D implementation
├── formula.h / cpp       - Formula system for move sequences
├── config.h / cpp        - Configuration management
├── model.h / cpp         - 3D model loader
└── shader.h / cpp        - Shader utilities

third_party/
└── imgui/                - ImGUI library

formula/                  - User formula files (created automatically)
```

## Architecture

The application follows a modular architecture with clear separation of concerns:

- **CubeRenderer**: Facade class that coordinates all rendering components
- **RubiksCube**: Pure data model (injected into renderer via reference)
- **CubeAnimator**: Manages animation state, timing, and easing functions
- **Renderer2D**: Stateless 2D unfolded cube visualization
- **Renderer3DOpenGL**: OpenGL-based 3D rendering with IRenderer3D interface
- **ColorProvider**: Centralized color configuration
- **ViewState**: View rotation angles and scale factors

## Configuration File

Settings are saved to `~/.rubiks-cube/config.json` including:
- Custom colors for each face
- Animation preferences (enabled/disabled, speed)
- View parameters (scales, rotations)

## Formula File Format

Example formula file (`formula/basics.txt`):
```
# Simple algorithms
OLL: F R U R' U' F'
PLL: U R U' L' U R' U' L2 U R' U' L'
# Loop example
Sexy Move: R U R' U' loop 3
```

Each line should be in format: `name: move_sequence` or `name: move_sequence loop N`

## License

MIT

## Changelog

### v1.2.0 (2026-03-16)
- **Architecture Refactoring**: CubeRenderer split into modular components (ViewState, ColorProvider, CubeAnimator, Renderer2D, Renderer3DOpenGL)
- **Double Moves**: Full support for U2/D2/L2/R2/F2/B2/M2/E2/S2/X2/Y2/Z2
- **3D Rendering Improvements**: Rounded sticker corners, circle shadow canvas, proper viewport centering
- **Configurable Animation Easing**: Linear, ease-in, ease-out, ease-in-out curves
- **Code Quality**: Application class extracted, main.cpp reduced to thin entry point
- **UI Polish**: Bigger stickers, softer colors, adjusted 2D sticker gaps
