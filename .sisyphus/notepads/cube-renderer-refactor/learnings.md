
## 2026-03-15: ViewState Extraction

### Pattern: Extracting State into Separate Structure

**Context**: Extracting view state (rotation angles, scale factors) from CubeRenderer into dedicated ViewState struct.

**Approach**:
1. Create new struct with all view-related state variables
2. Move methods that operate only on view state (lerpRotation, reset) into the struct
3. Replace individual member variables with single `viewState_` member
4. Update all access sites using search/replace pattern

**Naming Convention**:
- Member in renderer: `viewState_` (underscore suffix for private-style public member)
- Struct fields: `rotationX`, `rotationY`, `rotationZ`, `scale3D`, `scale2D`, etc.
- Method: `lerpRotation(float& current, float target, float deltaTime)` - takes current by reference

**Files Modified**:
- `src/view_state.h` (new) - struct definition
- `src/view_state.cpp` (new) - lerpRotation and reset implementations
- `src/renderer.h` - add include, replace members with viewState_
- `src/renderer.cpp` - update all references, remove lerpRotation method
- `src/main.cpp` - update all UI access points
- `CMakeLists.txt` - add view_state.cpp to build

**CMake Integration**:
```cmake
add_executable(rubiks-cube
    ...
    src/view_state.cpp
    ...
)
```

**Key Insight**: The `lerpRotation` method takes current by reference because it needs to modify it during smooth interpolation. This pattern was preserved when moving to ViewState.

## 2026-03-15: CubeAnimator Extraction

### Pattern: Extracting Animation Logic with Callback Pattern

**Context**: Extracting animation state and logic from CubeRenderer into dedicated CubeAnimator class.

**Approach**:
1. Create new class with animation state (progress, queue, current move)
2. Use callbacks instead of direct dependencies:
   - `MoveCallback` for when animation completes (renderer executes actual move)
   - `CubeGetter` for getting current cube state (animator snapshots for pre-animation state)
3. Animator manages timing/progress, renderer executes side effects

**Key Design Decisions**:
- Animator does NOT own the cube - uses `CubeGetter` callback to get state when needed
- Animator does NOT execute moves - calls `MoveCompleteCallback` instead
- Renderer sets up callbacks in constructor:
  ```cpp
  animator_.setMoveCompleteCallback([this](Move move, bool recordHistory) {
      cube_.executeMove(move, recordHistory);
  });
  animator_.setCubeGetter([this]() -> const RubiksCube& {
      return cube_;
  });
  ```

**Methods Moved**:
- `updateAnimation()` -> `animator_.update()`
- `startNextAnimation()` -> private `startNextAnimation()`
- `isCubeAnimating()` -> `isCubeInAnimatingSlice()`
- `isDoubleMove()` -> private `isDoubleMove()`

**Access Pattern**:
- `renderer.animator_.enableAnimation` - public member access for UI
- `renderer.animator_.animationSpeed` - public member access for UI
- `renderer.isAnimating()` - inline delegation to animator
- `renderer.animationProgress()` - inline delegation to animator

**Files Created**:
- `src/cube_animator.h` - class definition with callbacks
- `src/cube_animator.cpp` - animation logic implementation

**Files Modified**:
- `src/renderer.h` - add CubeAnimator member, remove animation state
- `src/renderer.cpp` - use animator_, set up callbacks
- `src/main.cpp` - access via `renderer.animator_.xxx`
- `CMakeLists.txt` - add cube_animator.cpp

**Key Insight**: Using callbacks for dependencies allows the animator to be self-contained while still coordinating with the renderer. The animator is a "pure" animation controller that doesn't know about cube internals.

## 2026-03-15: Renderer2D Extraction

### Pattern: Extracting Stateless Renderer via Parameter Injection

**Context**: Extracting 2D rendering logic from CubeRenderer into dedicated Renderer2D class.

**Approach**:
1. Create new class that receives dependencies via method parameters (not members)
2. Renderer2D does NOT hold state - cube and colors passed as const references to `draw()`
3. CubeRenderer owns a `Renderer2D renderer2D_` member and delegates to it

**Key Design Decisions**:
- Renderer2D is stateless - all data comes from parameters
- `draw(ImDrawList*, ImVec2, float, const RubiksCube&, const ColorProvider&)`
- `drawFace()` is private helper, takes colors by const reference
- No ownership of cube or colors - just renders what it's given

**Methods Moved**:
- `draw2D()` logic -> `Renderer2D::draw()`
- `drawFace()` -> `Renderer2D::drawFace()` (private)

**Implementation Note**:
The original `drawFace()` method used `colorProvider_.getFaceColor()`. After extraction, it receives `colors` parameter and calls `colors.getFaceColor()`.

**Files Created**:
- `src/renderer_2d.h` - class definition
- `src/renderer_2d.cpp` - draw() and drawFace() implementations

**Files Modified**:
- `src/renderer.h` - add `#include "renderer_2d.h"`, add `Renderer2D renderer2D_` member
- `src/renderer.cpp` - delegate `draw2D()` to `renderer2D_.draw()`, remove old `drawFace()` implementation
- `CMakeLists.txt` - add renderer_2d.cpp

**Delegation Pattern**:
```cpp
void CubeRenderer::draw2D(ImDrawList* drawList, ImVec2 offset, float scale) {
    renderer2D_.draw(drawList, offset, scale, cube_, colorProvider_);
}
```

**Key Insight**: For stateless rendering, passing dependencies as method parameters keeps the class clean and testable. The renderer just needs data to render, not ownership of that data.

## 2026-03-15: Renderer3DOpenGL Extraction

### Pattern: Extracting 3D Renderer with Interface Implementation

**Context**: Extracting OpenGL 3D rendering logic from CubeRenderer into Renderer3DOpenGL that implements IRenderer3D interface.

**Approach**:
1. Create Renderer3DOpenGL class that implements IRenderer3D interface
2. Use setter injection for dependencies (viewState_, colorProvider_, animator_, cube_)
3. CubeRenderer owns a `Renderer3DOpenGL renderer3D_` member and delegates to it

**Key Design Decisions**:
- Renderer3DOpenGL implements IRenderer3D interface (polymorphic)
- Dependencies set via setters: `setViewState()`, `setColorProvider()`, `setAnimator()`, `setCube()`
- Uses pointer members internally (not owning, just referencing)
- Owns Model* for the cube mesh

**Methods Moved**:
- `initGL3D()` -> `initGL()` (private)
- `render3DOverlay()` -> `render()`
- `drawCube()` (private helper)
- `drawCircleCanvas()` (private helper)
- `applyRotationTransform()` (private helper)

**Dependency Setup Pattern**:
```cpp
CubeRenderer::CubeRenderer() {
    renderer3D_.setViewState(&viewState_);
    renderer3D_.setColorProvider(&colorProvider_);
    renderer3D_.setAnimator(&animator_);
    renderer3D_.setCube(&cube_);
}
```

**Delegation Pattern**:
```cpp
void CubeRenderer::render3DOverlay(int windowWidth, int windowHeight) {
    renderer3D_.render(windowWidth, windowHeight);
}
```

**Files Created**:
- `src/renderer_3d_opengl.h` - class definition implementing IRenderer3D
- `src/renderer_3d_opengl.cpp` - render() and helper implementations

**Files Modified**:
- `src/renderer.h` - add include, add Renderer3DOpenGL member, remove OpenGL methods
- `src/renderer.cpp` - delegate render3DOverlay(), initialize renderer3D_, remove OpenGL implementations
- `CMakeLists.txt` - add renderer_3d_opengl.cpp

**Key Insight**: Using an interface (IRenderer3D) allows future implementation of different 3D backends (e.g., Vulkan, Metal) that can be swapped at runtime. The setter injection pattern keeps dependencies explicit while avoiding circular dependencies.

## 2026-03-15: CubeRenderer as Facade with Dependency Injection

### Pattern: Converting Ownership to Reference Injection

**Context**: Changing CubeRenderer from owning a RubiksCube instance to receiving a reference to an external cube.

**Approach**:
1. Change constructor from `CubeRenderer()` to `explicit CubeRenderer(RubiksCube& cube)`
2. Change member from `RubiksCube cube_` (owned) to `RubiksCube& cube_` (reference)
3. Initialize reference in constructor initializer list: `cube_(cube)`
4. Update main.cpp to create cube externally and pass to renderer

**Key Design Decisions**:
- Use non-const reference (not pointer) as per plan specification
- Reference must be initialized in constructor initializer list (C++ requirement)
- The cube is now owned by main(), not CubeRenderer
- All existing methods that use `cube_` continue to work unchanged

**Before (Ownership)**:
```cpp
// renderer.h
class CubeRenderer {
public:
    CubeRenderer();
private:
    RubiksCube cube_;  // OWNS the cube
};

// renderer.cpp
CubeRenderer::CubeRenderer()
    : cube_(), animator_() { ... }

// main.cpp
CubeRenderer renderer;  // Cube created internally
```

**After (Reference Injection)**:
```cpp
// renderer.h
class CubeRenderer {
public:
    explicit CubeRenderer(RubiksCube& cube);
private:
    RubiksCube& cube_;  // REFERENCE to external cube
};

// renderer.cpp
CubeRenderer::CubeRenderer(RubiksCube& cube)
    : cube_(cube), animator_() { ... }

// main.cpp
RubiksCube cube;
CubeRenderer renderer(cube);  // Cube passed from outside
```

**Files Modified**:
- `src/renderer.h` - constructor signature, member type
- `src/renderer.cpp` - constructor implementation
- `src/main.cpp` - create cube externally, pass to renderer

**Key Insight**: Using dependency injection with references allows:
1. External ownership of the cube (e.g., for multiple views on same cube)
2. Better testability (can inject mock cubes)
3. Clearer ownership semantics (renderer doesn't own data, just renders it)
4. The `explicit` keyword prevents implicit conversions that could create dangling references
