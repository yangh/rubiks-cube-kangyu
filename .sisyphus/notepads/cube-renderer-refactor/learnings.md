
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
