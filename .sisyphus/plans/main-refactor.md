# Refactor main.cpp into Application Class

## TL;DR

> **Quick Summary**: Extract the monolithic main.cpp (961 lines) into a clean Application class pattern. Create app.h/cpp with all state as private members, UI rendering as methods, and reduce main.cpp to a thin entry point.
> 
> **Deliverables**:
> - `src/app.h` - Application class declaration
> - `src/app.cpp` - Application class implementation (~600 lines)
> - Modified `src/main.cpp` - Thin entry point (~40 lines)
> - Modified `CMakeLists.txt` - Add app.cpp to build
> 
> **Estimated Effort**: Medium
> **Parallel Execution**: NO - sequential refactoring
> **Critical Path**: Create app.h → Create app.cpp → Update main.cpp → Update CMakeLists.txt → Build & Test

---

## Context

### Original Request
Refactor main.cpp which has a huge while loop, and some functions may be moved to other proper places.

### Interview Summary
**Key Discussions**:
- **Approach**: Class-based refactoring with Application class
- **File naming**: Short names (app.h, not application.h)
- **UI location**: Application methods (renderMovesTab, renderFormulasTab, etc.)
- **State locality**: Private members (encapsulated, clean OOP)
- **Test strategy**: Manual QA only

**Research Findings**:
- Existing codebase is well-modularized (cube.h, renderer.h, formula.h, config.h, etc.)
- CubeRenderer takes RubiksCube& - Application will own both
- FormulaManager already exists as standalone class
- Current main.cpp has 12+ global variables, 9 helper functions

### Metis Review
**Identified Gaps** (addressed):
- **GLFWwindow ownership**: Application owns window (sensible default)
- **ImGui context ownership**: Application owns context (matches GLFW)
- **Tab method extraction**: 4 separate private methods (user confirmed)
- **main.cpp target size**: ~30-50 lines (arg parsing + app creation)
- **Scope creep prevention**: Explicit guardrails against over-engineering

---

## Work Objectives

### Core Objective
Transform main.cpp from a 961-line monolith into a clean class-based architecture while preserving exact application behavior.

### Concrete Deliverables
- `src/app.h` - Application class header with private members and method declarations
- `src/app.cpp` - Application implementation with all logic extracted from main.cpp
- `src/main.cpp` - Reduced to ~40 lines (arg parsing + Application instantiation)
- `CMakeLists.txt` - Updated to compile app.cpp

### Definition of Done
- [ ] Application compiles without errors: `cmake -S . -B build && make -C build`
- [ ] Binary runs with same behavior: `./build/rubiks-cube`
- [ ] All CLI args work: `-d`, `-h`, `--dump`, `--help`
- [ ] All keyboard shortcuts work identically
- [ ] All UI tabs render identically
- [ ] Config persistence works (colors, animation settings)

### Must Have
- Application class with private members for all state
- UI rendering as private methods
- Exact same runtime behavior

### Must NOT Have (Guardrails)
- NO new files other than app.h and app.cpp
- NO modifications to existing modules (cube.h, renderer.h, config.h, formula.h, etc.)
- NO new classes besides Application
- NO RAII wrappers for GLFW/ImGui
- NO data type changes (keep char[1024], not std::string)
- NO added error handling, logging, or validation
- NO code "improvements" beyond organization
- NO new dependencies

---

## Verification Strategy (MANDATORY)

### Test Decision
- **Infrastructure exists**: NO (no unit test framework)
- **Automated tests**: None (manual QA only)
- **Framework**: N/A

### QA Policy
Every task MUST include agent-executed QA scenarios.
Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

- **Desktop GUI App**: Use interactive_bash (tmux) — Run application, verify startup, check exit code
- **Manual verification**: Agent documents steps to verify functionality

---

## Execution Strategy

### Sequential Execution (Refactoring)

```
Step 1: Create app.h
├── Define Application class
├── Declare all private members (from globals)
├── Declare all private methods (from helpers + UI sections)
└── Include necessary headers

Step 2: Create app.cpp
├── Implement constructor (GLFW + ImGui init)
├── Implement destructor (cleanup)
├── Implement run() method (main loop)
├── Implement all UI rendering methods
├── Implement all helper methods
└── Copy logic from main.cpp verbatim (no changes)

Step 3: Update main.cpp
├── Include app.h
├── Keep showHelp() function
├── Parse CLI args
├── Create Application instance
└── Return app.run()

Step 4: Update CMakeLists.txt
└── Add app.cpp to source list

Step 5: Build and Verify
├── Compile project
├── Run manual QA checklist
└── Verify all features work
```

### Dependency Matrix

- **1**: — 2, 3, 4 (app.h blocks all)
- **2**: 1 — 5 (app.cpp depends on app.h)
- **3**: 1 — 5 (main.cpp depends on app.h)
- **4**: — 5 (can run parallel with 2, 3)
- **5**: 2, 3, 4 — (final verification)

### Agent Dispatch Summary

- **1**: **1** — app.h creation → `quick`
- **2**: **1** — app.cpp creation → `unspecified-high` (large file, careful extraction)
- **3**: **1** — main.cpp update → `quick`
- **4**: **1** — CMakeLists.txt update → `quick`
- **5**: **1** — Build and verify → `unspecified-high`

---

## TODOs

- [x] 1. Create app.h - Application Class Declaration

  **What to do**:
  - Create `src/app.h` with Application class declaration
  - Add all private members (converted from global variables):
    - `GLFWwindow* window_ = nullptr`
    - `RubiksCube cube_`
    - `CubeRenderer* renderer_ = nullptr` (pointer due to init order)
    - `FormulaManager formulaManager_`
    - Fullscreen state: `isFullscreen_`, `windowedX_/Y_/Width_/Height_`
    - Step-by-step state: `stepByStepMoves_`, `currentStepIndex_`, `isStepByStepMode_`
    - Formula input: `formulaInput_[1024]`, `formulaInputDirty_`
    - Other: `enableDump_`, `showAboutDialog_`, `lastScramble_`
  - Add public methods: `Application()`, `~Application()`, `int run()`
  - Add private initialization methods: `initGlfw()`, `initImGui()`, `loadFonts()`
  - Add private UI methods: `renderMenuBar()`, `render3DView()`, `render2DNetView()`, `renderControls()`, `renderMovesTab()`, `renderFormulasTab()`, `renderSettingsTab()`, `renderShortcutsTab()`, `showAbout()`
  - Add private helper methods: `resetStepByStepMode()`, `saveRendererConfig()`, `resetCube()`, `scrambleCube()`, `toggleFullscreen()`, `handleKeyboardShortcuts()`, `handleMoveShortcut()`, `addColorPicker()`, `drawDisabledButton()`, `buildMoveHistoryString()`

  **Must NOT do**:
  - Do NOT modify any existing header files
  - Do NOT add new dependencies in includes
  - Do NOT change any type signatures

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Straightforward header file creation with clear structure
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential (first task)
  - **Blocks**: Task 2, Task 3
  - **Blocked By**: None

  **References**:
  - `src/main.cpp:16-41` - Global variables to convert to members
  - `src/main.cpp:43-159` - Helper functions to convert to methods
  - `src/renderer.h` - Pattern for member organization (renderer_, animator_, etc.)
  - `src/formula.h:22-74` - FormulaManager class interface

  **Acceptance Criteria**:
  - [ ] File created: src/app.h
  - [ ] All 12+ global variables converted to private members
  - [ ] All 9 helper functions declared as private methods
  - [ ] 4 UI tab methods declared
  - [ ] Public constructor, destructor, run() declared
  - [ ] Compilation check: Header is syntactically valid

  **QA Scenarios**:

  ```
  Scenario: Header file is syntactically valid
    Tool: Bash
    Preconditions: src/app.h exists
    Steps:
      1. g++ -fsyntax-only -std=c++17 -I/usr/include -Ithird_party/imgui src/app.h
      2. Check exit code is 0
    Expected Result: No syntax errors
    Failure Indicators: Compilation errors in header
    Evidence: .sisyphus/evidence/task-1-header-syntax.txt
  ```

  **Commit**: YES
  - Message: `refactor: add app.h with Application class declaration`
  - Files: `src/app.h`
  - Pre-commit: syntax check

---

- [x] 2. Create app.cpp - Application Implementation

  **What to do**:
  - Create `src/app.cpp` with Application class implementation
  - Implement constructor:
    - Store `enableDump_` from CLI args (pass via constructor or setter)
    - Call `initGlfw()` - GLFW window creation (lines 186-210 from main.cpp)
    - Call `initImGui()` - ImGui setup (lines 212-224)
    - Call `loadFonts()` - Chinese font loading (lines 226-250)
    - Initialize cube and renderer (lines 252-275)
    - Load formula manager
  - Implement destructor:
    - ImGui shutdown (lines 952-955)
    - GLFW cleanup (lines 957-958)
  - Implement `run()`:
    - Main loop (lines 277-950)
    - Call `handleKeyboardShortcuts()`
    - Call `renderMenuBar()`, `render3DView()`, `render2DNetView()`, `renderControls()`
    - Call `showAbout()`
    - Handle frame rendering
  - Implement `renderMovesTab()` (lines 467-601)
  - Implement `renderFormulasTab()` (lines 604-810)
  - Implement `renderSettingsTab()` (lines 813-901)
  - Implement `renderShortcutsTab()` (lines 904-922)
  - Implement helper methods (copy logic verbatim from main.cpp)
  - **CRITICAL**: Copy code EXACTLY, no modifications

  **Must NOT do**:
  - Do NOT "improve" code while copying
  - Do NOT add error handling
  - Do NOT change variable names
  - Do NOT extract additional functions

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: Large file (~600 lines), requires careful extraction without errors
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential (after Task 1)
  - **Blocks**: Task 5
  - **Blocked By**: Task 1 (app.h)

  **References**:
  - `src/main.cpp:186-275` - Initialization code for constructor
  - `src/main.cpp:277-950` - Main loop for run() method
  - `src/main.cpp:467-601` - Moves tab UI code
  - `src/main.cpp:604-810` - Formulas tab UI code
  - `src/main.cpp:813-901` - Settings tab UI code
  - `src/main.cpp:904-922` - Shortcuts tab UI code
  - `src/main.cpp:43-159` - Helper function implementations
  - `src/main.cpp:952-958` - Cleanup code for destructor

  **Acceptance Criteria**:
  - [ ] File created: src/app.cpp
  - [ ] Constructor implemented with GLFW + ImGui init
  - [ ] Destructor implemented with proper cleanup order
  - [ ] run() implemented with complete main loop
  - [ ] All 4 tab methods implemented
  - [ ] All helper methods implemented
  - [ ] Code is copied verbatim (no logic changes)

  **QA Scenarios**:

  ```
  Scenario: Implementation compiles without errors
    Tool: Bash
    Preconditions: src/app.h and src/app.cpp exist
    Steps:
      1. g++ -c -std=c++17 -I/usr/include -Ithird_party/imgui src/app.cpp -o /tmp/app.o 2>&1
      2. Check exit code and output
    Expected Result: Compilation succeeds (may have linker warnings, that's OK)
    Failure Indicators: Syntax errors, undefined references to declared methods
    Evidence: .sisyphus/evidence/task-2-compile.txt
  ```

  **Commit**: YES
  - Message: `refactor: add app.cpp with Application implementation`
  - Files: `src/app.cpp`
  - Pre-commit: compilation check

---

- [x] 3. Update main.cpp - Thin Entry Point

  **What to do**:
  - Remove all global variables (lines 16-41)
  - Remove all helper functions (lines 43-159)
  - Remove all initialization code (lines 186-275)
  - Remove all main loop code (lines 277-950)
  - Remove cleanup code (lines 952-958)
  - Keep only:
    - Include statements (add `#include "app.h"`)
    - `showHelp()` function (lines 141-159)
    - `main()` function with CLI parsing
  - In `main()`:
    - Parse CLI args (lines 162-184)
    - If `--help`, call `showHelp()` and return 0
    - Create `Application app;`
    - If `--dump`, pass flag to app (via setter or constructor parameter)
    - Return `app.run()`

  **Must NOT do**:
  - Do NOT change CLI argument parsing logic
  - Do NOT remove showHelp() function
  - Do NOT add new CLI arguments

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Straightforward file reduction, mostly deletion
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 4)
  - **Parallel Group**: Wave 2 (with Task 4)
  - **Blocks**: Task 5
  - **Blocked By**: Task 1 (app.h)

  **References**:
  - `src/main.cpp:1-15` - Include statements to keep
  - `src/main.cpp:141-159` - showHelp() to keep
  - `src/main.cpp:162-184` - CLI parsing to keep

  **Acceptance Criteria**:
  - [ ] main.cpp reduced to ~40 lines
  - [ ] CLI args (-d, -h, --dump, --help) still work
  - [ ] Application instantiation present
  - [ ] showHelp() function preserved

  **QA Scenarios**:

  ```
  Scenario: main.cpp is syntactically valid
    Tool: Bash
    Preconditions: src/main.cpp updated
    Steps:
      1. g++ -fsyntax-only -std=c++17 -I/usr/include -Ithird_party/imgui src/main.cpp
      2. Check exit code
    Expected Result: No syntax errors
    Evidence: .sisyphus/evidence/task-3-main-syntax.txt
  ```

  **Commit**: YES
  - Message: `refactor: reduce main.cpp to thin entry point`
  - Files: `src/main.cpp`
  - Pre-commit: syntax check

---

- [x] 4. Update CMakeLists.txt - Add app.cpp

  **What to do**:
  - Find the source file list in CMakeLists.txt
  - Add `src/app.cpp` to the list
  - No other changes needed

  **Must NOT do**:
  - Do NOT add new dependencies
  - Do NOT change compiler flags
  - Do NOT modify any other build settings

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Single line addition to build file
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Task 3)
  - **Parallel Group**: Wave 2 (with Task 3)
  - **Blocks**: Task 5
  - **Blocked By**: None

  **References**:
  - `CMakeLists.txt` - Find add_executable or source list

  **Acceptance Criteria**:
  - [ ] src/app.cpp added to source files
  - [ ] CMakeLists.txt syntax valid

  **QA Scenarios**:

  ```
  Scenario: CMake configuration succeeds
    Tool: Bash
    Preconditions: CMakeLists.txt updated
    Steps:
      1. cmake -S . -B build_test 2>&1
      2. Check exit code
      3. rm -rf build_test
    Expected Result: CMake configuration succeeds
    Evidence: .sisyphus/evidence/task-4-cmake.txt
  ```

  **Commit**: YES
  - Message: `refactor: add app.cpp to CMakeLists.txt`
  - Files: `CMakeLists.txt`
  - Pre-commit: cmake check

---

- [x] 5. Build and Verify - Complete System Test

  **What to do**:
  - Clean build directory: `rm -rf build`
  - Configure: `cmake -S . -B build`
  - Build: `make -C build`
  - Verify binary exists: `ls -la build/rubiks-cube`
  - Run manual QA checklist (all features)
  - Document any issues found

  **Must NOT do**:
  - Do NOT skip any QA items
  - Do NOT proceed if build fails

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: Critical verification step, requires thorough testing
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential (final task)
  - **Blocks**: None
  - **Blocked By**: Task 2, Task 3, Task 4

  **References**:
  - `README.md` - Build instructions
  - `src/main.cpp` - CLI args to test

  **Acceptance Criteria**:
  - [ ] Build succeeds without errors
  - [ ] Binary produced at build/rubiks-cube
  - [ ] All CLI args work (--help, -d)
  - [ ] Application launches and runs
  - [ ] No segmentation faults or crashes

  **QA Scenarios**:

  ```
  Scenario: Build succeeds
    Tool: Bash
    Preconditions: All source files ready
    Steps:
      1. rm -rf build
      2. cmake -S . -B build 2>&1 | tee .sisyphus/evidence/task-5-cmake.txt
      3. make -C build 2>&1 | tee .sisyphus/evidence/task-5-make.txt
      4. ls -la build/rubiks-cube
    Expected Result: Binary exists and is executable
    Failure Indicators: Compilation errors, linker errors
    Evidence: .sisyphus/evidence/task-5-build.txt

  Scenario: CLI help works
    Tool: Bash
    Preconditions: Binary built
    Steps:
      1. ./build/rubiks-cube --help 2>&1
      2. Check output contains "Rubik's Cube Simulator"
      3. Check output contains "Usage:"
    Expected Result: Help text displayed
    Evidence: .sisyphus/evidence/task-5-cli-help.txt

  Scenario: Application launches (quick check)
    Tool: interactive_bash
    Preconditions: Binary built, X11 display available
    Steps:
      1. tmux new-session -d -s rubiks-test "./build/rubiks-cube"
      2. sleep 2
      3. tmux capture-pane -t rubiks-test -p
      4. tmux send-keys -t rubiks-test C-c
      5. tmux kill-session -t rubiks-test
    Expected Result: Application launches without crash
    Failure Indicators: Immediate crash, segfault
    Evidence: .sisyphus/evidence/task-5-launch.txt
  ```

  **Commit**: YES
  - Message: `refactor: complete main.cpp to Application class refactoring`
  - Files: (all changes together)
  - Pre-commit: full build + manual QA

---

## Final Verification Wave (MANDATORY)

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Verify all tasks completed: app.h created, app.cpp created, main.cpp reduced, CMakeLists.txt updated. Check no extra files created. Verify guardrails respected (no new classes, no modified existing modules).

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Build project from clean state. Run `cppcheck` or similar static analysis if available. Review code organization: members properly encapsulated, methods properly named.

- [ ] F3. **Real Manual QA** — `unspecified-high`
  Execute full QA checklist:
  1. Launch application
  2. Test all keyboard shortcuts (R, U, D, L, F, B, M, E, S, X, Y, Z with Shift variants)
  3. Test Ctrl+Z (undo), Ctrl+R (redo), Ctrl+Q (quit), F11 (fullscreen)
  4. Test all UI buttons (Scramble, Reset, Execute, Step, etc.)
  5. Navigate all 4 tabs
  6. Change settings (animation speed, colors)
  7. Verify config persistence (restart app)
  8. Test CLI args (--help, -d)

- [ ] F4. **Scope Fidelity Check** — `deep`
  Compare line counts: main.cpp should be ~40 lines, app.cpp should be ~600 lines. Verify no logic was added or removed, only reorganized. Check git diff shows only intended changes.

---

## Commit Strategy

- **Single commit** at end (all changes together):
  ```
  refactor: extract Application class from main.cpp
  
  - Create app.h with Application class declaration
  - Create app.cpp with Application implementation
  - Reduce main.cpp to thin entry point (~40 lines)
  - Update CMakeLists.txt to include app.cpp
  
  No behavior changes, pure code reorganization.
  ```

---

## Success Criteria

### Verification Commands
```bash
# Build succeeds
cmake -S . -B build && make -C build
# Expected: Binary at build/rubiks-cube

# Help works
./build/rubiks-cube --help
# Expected: Usage text displayed

# Application runs (requires display)
./build/rubiks-cube
# Expected: Window opens with Rubik's cube UI
```

### Final Checklist
- [ ] app.h created with Application class
- [ ] app.cpp created with all logic
- [ ] main.cpp reduced to ~40 lines
- [ ] CMakeLists.txt updated
- [ ] Build succeeds
- [ ] All CLI args work
- [ ] All keyboard shortcuts work
- [ ] All UI tabs work
- [ ] No behavior changes
