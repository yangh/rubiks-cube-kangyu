# Rubik's Cube Simulator

A 3D Rubik's cube simulator built with C++, ImGUI, and OpenGL.

## Features

- Interactive 2D unfolded cube visualization
- Full set of Rubik's cube moves (U, D, L, R, F, B and their primes)
- Real-time cube state tracking
- Solvable state detection
- Adjustable viewing scale

## Requirements

- **CMake**: 3.15 or later
- **C++ Compiler**: Supporting C++17 (GCC, Clang, or MSVC)
- **GLFW3**: For window management
- **OpenGL**: For 3D rendering

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
# If you need a proxy for git:
git -c http.proxy=http://127.0.0.1:20170 clone https://github.com/ocornut/imgui.git
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

- Use the move buttons (R, R', L, L', etc.) to rotate cube faces
- Adjust Scale slider to zoom in/out
- Click "Reset Cube" to return to the solved state

## Cube Notation

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

## Project Structure

```
src/
├── main.cpp      - Application entry point and main loop
├── cube.h        - Cube state representation and move logic
├── cube.cpp      - Cube implementation
├── renderer.h    - ImGui 2D rendering
└── renderer.cpp  - Renderer implementation
```

## License

MIT
