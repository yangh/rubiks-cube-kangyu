# CubeRenderer 重构计划

## TL;DR

> **Quick Summary**: 将 1540 行的 CubeRenderer "God Class" 拆分为独立组件：2D 渲染器、可插拔 3D 渲染器、独立动画控制器。通过依赖注入 RubiksCube 实现数据与视图分离，支持运行时切换 3D 渲染风格。

> **Deliverables**:
> - `irenderer_3d.h` - 3D 渲染器抽象接口
> - `renderer_2d.h/cpp` - 独立的 2D 展开 view
> - `renderer_3d_opengl.h/cpp` - OpenGL 3D 实现
> - `cube_animator.h/cpp` - 独立动画控制器
> - `cube_renderer.h/cpp` - 重构后的门面类
> - `view_state.h` - 视角状态结构体
> - 删除 ~200 行未使用的死代码

> **Estimated Effort**: Medium (6-8 tasks, ~4-6 hours)
> **Parallel Execution**: YES - Wave 1 foundation + Wave 2 extraction
> **Critical Path**: ViewState → Animator → IRenderer3D → Renderer3DOpenGL → CubeRenderer facade

---

## Context

### Original Request
用户要求重构 CubeRenderer：
1. 将 2D/3D view 代码分成独立文件
2. 抽象一层渲染接口
3. 将 RubiksCube 实例传递给 Render 实例（依赖注入）
4. 3D view 独立出来后方便引入不同风格的实现
5. 考虑 3D 动画部分的抽象

### Interview Summary
**Key Discussions**:
- **3D 风格切换**: 运行时动态切换，需要 `IRenderer3D` 接口
- **动画系统**: 提取独立 `CubeAnimator` 类（只服务 3D，2D 无动画）
- **旧代码处理**: 删除未使用的 ImGui 3D 代码（~200行）
- **Cube 注入**: 构造函数引用注入 `CubeRenderer(const RubiksCube& cube)`
- **文件组织**: 平铺在 `src/` 目录
- **API 兼容性**: 允许重构 main.cpp，不需要保持旧接口兼容

**Research Findings**:
- RubiksCube 有干净的 getter 接口，适合依赖注入
- 项目有测试基础设施（CMake + assert 风格，8 个测试文件）
- 当前 3D 使用 OpenGL immediate mode（glBegin/glEnd）
- 动画使用队列 + easing 函数（3t² - 2t³）
- `draw3D()` 在 main.cpp 被调用但内部直接返回，可安全删除

### Metis Review
**Identified Gaps** (addressed):
- **OpenGL context 管理**: render3DOverlay 在 ImGui 渲染后调用，GL 状态需明确所有权
- **Model 资源所有权**: cubeModel 在 CubeRenderer 中，重构后需决定归属
- **动画期间 undo 行为**: 当前未定义，需要明确处理策略
- **draw3D() 删除安全性**: 已确认 main.cpp 调用但内部返回早，需先改为调用 render3DOverlay

---

## Work Objectives

### Core Objective
拆分 CubeRenderer（1540 行）为独立可测试的组件，实现：
1. **关注点分离**: 2D 渲染、3D 渲染、动画、状态管理各司其职
2. **依赖注入**: RubiksCube 通过构造函数注入，而非内部拥有
3. **可扩展性**: 通过 `IRenderer3D` 接口支持多种 3D 渲染实现
4. **代码整洁**: 删除 ~200 行未使用的死代码

### Concrete Deliverables
- `src/irenderer_3d.h` - 3D 渲染器抽象接口
- `src/renderer_2d.h` + `src/renderer_2d.cpp` - 2D 展开 view
- `src/renderer_3d_opengl.h` + `src/renderer_3d_opengl.cpp` - OpenGL 3D 实现
- `src/cube_animator.h` + `src/cube_animator.cpp` - 动画控制器
- `src/cube_renderer.h` + `src/cube_renderer.cpp` - 重构后的门面类
- `src/view_state.h` - 视角状态结构体

### Definition of Done
- [ ] `cmake -S . -B build && make -C build` 构建成功
- [ ] `./build/rubiks-cube` 启动无崩溃
- [ ] 2D 展开 view 显示正确
- [ ] 3D view 显示正确
- [ ] 所有移动操作（U/D/L/R/F/B/M/E/S/X/Y/Z）工作
- [ ] 动画播放流畅，速度正确（~200ms @ 1.0x）
- [ ] Undo/Redo 功能正常
- [ ] Scramble/Reset 功能正常
- [ ] 配置加载/保存正常
- [ ] `ctest --test-dir build` 所有现有测试通过

### Must Have
- RubiksCube 通过构造函数引用注入
- IRenderer3D 接口支持运行时切换实现
- CubeAnimator 独立管理动画状态
- 现有功能 100% 行为保持

### Must NOT Have (Guardrails)
- 不修改 RubiksCube 类
- 不添加新的渲染特性（阴影、后处理等）
- 不改变 2D 展开布局
- 不改变动画时间（200ms base）或 easing 函数
- 不改变配置文件格式或位置
- 不为单一实现创建接口（YAGNI）
- 不在重构期间"顺便"改进代码

---

## Verification Strategy (MANDATORY)

> **ZERO HUMAN INTERVENTION** — ALL verification is agent-executed. No exceptions.

### Test Decision
- **Infrastructure exists**: YES (CMake + ctest + assert-based tests)
- **Automated tests**: Tests-after (verify behavior preservation)
- **Framework**: Native CMake/ctest + existing test suite
- **Agent-Executed QA**: ALWAYS (mandatory for all tasks)

### QA Policy
Every task MUST include agent-executed QA scenarios.
Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

- **Build Verification**: Use Bash — cmake build, verify exit code 0
- **Runtime Verification**: Use Bash — timeout with app startup, verify no crash
- **GUI Verification**: Use Playwright (playwright skill) — screenshot comparison if needed
- **Test Suite**: Use Bash — ctest to verify existing tests pass

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Start Immediately — foundation types + cleanup):
├── Task 1: Delete unused code from renderer.cpp [quick]
├── Task 2: Create ViewState struct [quick]
├── Task 3: Create IRenderer3D interface [quick]
└── Task 4: Create ColorProvider class [quick]

Wave 2 (After Wave 1 — extract components):
├── Task 5: Extract CubeAnimator class [unspecified-high]
├── Task 6: Extract Renderer2D class [unspecified-high]
└── Task 7: Extract Renderer3DOpenGL class [unspecified-high]

Wave 3 (After Wave 2 — integration):
├── Task 8: Refactor CubeRenderer as facade [deep]
└── Task 9: Update main.cpp for new architecture [unspecified-high]

Wave FINAL (After ALL tasks — verification):
├── Task F1: Plan compliance audit (oracle)
├── Task F2: Code quality review (unspecified-high)
└── Task F3: Full functionality smoke test (unspecified-high)
```

### Dependency Matrix

| Task | Depends On | Blocks |
|------|------------|--------|
| 1-4  | — | 5-9 |
| 5    | 2 | 8 |
| 6    | 4 | 8 |
| 7    | 3, 4, 5 | 8 |
| 8    | 3, 5, 6, 7 | 9 |
| 9    | 8 | F1-F3 |

### Agent Dispatch Summary
- **Wave 1**: 4 tasks → `quick` × 4
- **Wave 2**: 3 tasks → `unspecified-high` × 3
- **Wave 3**: 2 tasks → `deep` + `unspecified-high`
- **Wave FINAL**: 3 tasks → `oracle` + `unspecified-high` × 2

---

## TODOs

- [x] 1. **删除未使用的死代码** ✅

  **What to do**:
  - 从 `renderer.cpp` 删除以下未使用/早返回的方法：
    - `project()` (lines 351-373) - 3D 投影函数，未被调用
    - `draw3DFace()` (lines 425-532) - 未使用的 ImGui 3D 面绘制
    - `drawAnimated3DFace()` (lines 534-625) - 未使用的动画 3D 面绘制
    - `drawTestCube()` (lines 695-790) - 测试代码
    - `drawTestCubeWithColor()` - 测试代码
  - 从 `renderer.h` 删除对应的函数声明
  - 从 `renderer.h` 删除未使用的 `draw3D()` 声明（main.cpp 调用但内部直接返回）
  - 更新 `main.cpp`：移除对 `draw3D()` 的调用（line 430），改用 `render3DOverlay()`

  **Must NOT do**:
  - 不要删除 `render3DOverlay()` - 这是实际使用的 3D 渲染方法
  - 不要删除 `drawCube()` - 被 `render3DOverlay()` 使用

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 简单的代码删除，风险低
  - **Skills**: [`git-master`]
    - `git-master`: 需要使用 `lsp_find_references` 验证删除安全性

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 2, 3, 4)
  - **Blocks**: None
  - **Blocked By**: None

  **References**:
  - `src/renderer.cpp:351-373` - project() 函数（待删除）
  - `src/renderer.cpp:425-532` - draw3DFace() 函数（待删除）
  - `src/renderer.cpp:534-625` - drawAnimated3DFace() 函数（待删除）
  - `src/renderer.cpp:695-790` - drawTestCube() 函数（待删除）
  - `src/main.cpp:430` - draw3D() 调用点（需移除或替换）

  **Acceptance Criteria**:
  - [ ] `lsp_find_references` 确认所有删除的函数无外部引用
  - [ ] `cmake --build build` 构建成功
  - [ ] `./build/rubiks-cube` 启动无崩溃

  **QA Scenarios**:
  ```
  Scenario: Build after dead code removal
    Tool: Bash
    Steps:
      1. cmake -S . -B build && make -C build
    Expected Result: Exit code 0, no compilation errors
    Evidence: .sisyphus/evidence/task-01-build.log

  Scenario: App starts after dead code removal
    Tool: Bash
    Steps:
      1. timeout 3 ./build/rubiks-cube || [ $? -eq 124 ]
    Expected Result: No crash or error output
    Evidence: .sisyphus/evidence/task-01-startup.log
  ```

  **Commit**: YES
  - Message: `refactor(renderer): remove unused ImGui 3D rendering code`
  - Files: `src/renderer.h, src/renderer.cpp, src/main.cpp`

- [x] 2. **创建 ViewState 结构体** ✅

  **What to do**:
  - 创建 `src/view_state.h` 文件
  - 定义 `ViewState` 结构体，包含：
    ```cpp
    struct ViewState {
        // 3D 视角旋转
        float rotationX = 30.0f;
        float rotationY = -30.0f;
        float rotationZ = 0.0f;
        float targetRotationX = 30.0f;
        float targetRotationY = -30.0f;
        float targetRotationZ = 0.0f;
        float viewRotationSpeed = 8.0f;
        
        // 缩放
        float scale3D = 3.1f;
        float scale2D = 0.8f;
        
        // 平滑插值
        void lerpRotation(float& current, float target, float deltaTime);
        void reset();
    };
    ```
  - 从 `renderer.h` 移除对应的 public 成员变量
  - 在 `CubeRenderer` 中添加 `ViewState viewState_` 成员
  - 更新所有访问点使用 `viewState_.rotationX` 等

  **Must NOT do**:
  - 不要改变任何行为或默认值
  - 不要添加新功能

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 简单的结构体提取
  - **Skills**: []
    - 无特殊技能需求

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 3, 4)
  - **Blocks**: Task 5 (Animator 需要 ViewState)
  - **Blocked By**: None

  **References**:
  - `src/renderer.h:43-54` - 当前的视角状态成员变量
  - `src/renderer.cpp:285-300` - lerpRotation 函数实现
  - `src/renderer.cpp:101-110` - resetView 函数实现

  **Acceptance Criteria**:
  - [ ] `src/view_state.h` 文件存在
  - [ ] `ViewState` 结构体包含所有视角状态
  - [ ] `cmake --build build` 构建成功
  - [ ] 3D 视角旋转功能正常

  **QA Scenarios**:
  ```
  Scenario: ViewState extraction compiles
    Tool: Bash
    Steps:
      1. cmake -S . -B build && make -C build
    Expected Result: Exit code 0
    Evidence: .sisyphus/evidence/task-02-build.log

  Scenario: View rotation still works
    Tool: Bash
    Steps:
      1. Start app, verify 3D view renders at default angle
    Expected Result: 3D view visible, no crash
    Evidence: .sisyphus/evidence/task-02-view.log
  ```

  **Commit**: YES
  - Message: `refactor(renderer): extract ViewState struct`
  - Files: `src/view_state.h, src/renderer.h, src/renderer.cpp`

- [x] 3. **创建 IRenderer3D 接口** ✅

  **What to do**:
  - 创建 `src/irenderer_3d.h` 文件
  - 定义抽象接口：
    ```cpp
    class IRenderer3D {
    public:
        virtual ~IRenderer3D() = default;
        
        // 渲染 3D 视图（在 ImGui 渲染后调用）
        virtual void render(int windowWidth, int windowHeight) = 0;
        
        // 设置视角状态
        virtual void setViewState(const ViewState* state) = 0;
        
        // 设置颜色配置
        virtual void setColorProvider(const ColorProvider* provider) = 0;
        
        // 设置动画状态
        virtual void setAnimator(const CubeAnimator* animator) = 0;
    };
    ```
  - 接口设计原则：
    - 轻量级，只包含渲染所需的最小接口
    - 使用指针/引用注入依赖，不拥有资源

  **Must NOT do**:
  - 不要实现具体渲染逻辑（只定义接口）
  - 不要添加虚析构函数以外的实现

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 纯接口定义，无实现
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 2, 4)
  - **Blocks**: Task 7 (Renderer3DOpenGL 需实现此接口)
  - **Blocked By**: None

  **References**:
  - `src/renderer.h:19-22` - 当前 3D 渲染方法签名
  - `src/renderer.cpp:238-349` - render3DOverlay 实现（参考接口设计）

  **Acceptance Criteria**:
  - [ ] `src/irenderer_3d.h` 文件存在
  - [ ] `IRenderer3D` 是纯虚类
  - [ ] 包含虚析构函数
  - [ ] `cmake --build build` 构建成功（接口定义不影响编译）

  **QA Scenarios**:
  ```
  Scenario: Interface file exists and compiles
    Tool: Bash
    Steps:
      1. Verify file exists: test -f src/irenderer_3d.h
      2. Check for pure virtual: grep -q "= 0" src/irenderer_3d.h
    Expected Result: Both checks pass
    Evidence: .sisyphus/evidence/task-03-interface.log
  ```

  **Commit**: YES
  - Message: `refactor(renderer): add IRenderer3D interface`
  - Files: `src/irenderer_3d.h`

- [x] 4. **创建 ColorProvider 类** ✅

  **What to do**:
  - 创建 `src/color_provider.h` 和 `src/color_provider.cpp`
  - 提取颜色管理逻辑：
    ```cpp
    class ColorProvider {
    public:
        ColorProvider();
        
        // 设置自定义颜色
        void setCustomColors(const ColorConfig& config);
        void resetToDefaults();
        
        // 获取颜色
        ImU32 getFaceColor(Color color) const;
        std::array<float, 3> getFaceColorRgb(Color color) const;
        
        // 公开状态（供 UI 直接访问）
        std::array<float, 3> customFront = {0.0f, 1.0f, 0.0f};
        std::array<float, 3> customBack = {0.0f, 0.0f, 1.0f};
        std::array<float, 3> customLeft = {1.0f, 0.5f, 0.0f};
        std::array<float, 3> customRight = {1.0f, 0.0f, 0.0f};
        std::array<float, 3> customUp = {1.0f, 1.0f, 1.0f};
        std::array<float, 3> customDown = {1.0f, 1.0f, 0.0f};
        bool useCustomColors = false;
    };
    ```
  - 从 `renderer.h` 移除颜色相关成员
  - 在 `CubeRenderer` 中添加 `ColorProvider colorProvider_` 成员
  - 更新所有颜色访问点

  **Must NOT do**:
  - 不要改变颜色值或行为

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 简单的类提取
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 2, 3)
  - **Blocks**: Task 6, 7 (Renderer2D/3D 需要 ColorProvider)
  - **Blocked By**: None

  **References**:
  - `src/renderer.h:57-64` - 当前的颜色成员变量
  - `src/renderer.cpp:18-26` - setCustomColors 实现
  - `src/renderer.cpp:627-673` - getFaceColor/getFaceColorRgb 实现

  **Acceptance Criteria**:
  - [ ] `src/color_provider.h` 和 `src/color_provider.cpp` 存在
  - [ ] 颜色配置加载/保存正常
  - [ ] 2D/3D 视图颜色显示正确

  **QA Scenarios**:
  ```
  Scenario: ColorProvider extraction compiles
    Tool: Bash
    Steps:
      1. cmake -S . -B build && make -C build
    Expected Result: Exit code 0
    Evidence: .sisyphus/evidence/task-04-build.log

  Scenario: Custom colors still work
    Tool: Bash
    Steps:
      1. Start app, verify default colors display correctly
    Expected Result: Green front, white top, etc.
    Evidence: .sisyphus/evidence/task-04-colors.log
  ```

  **Commit**: YES
  - Message: `refactor(renderer): extract ColorProvider class`
  - Files: `src/color_provider.h, src/color_provider.cpp, src/renderer.h, src/renderer.cpp, CMakeLists.txt`

- [x] 5. **提取 CubeAnimator 类** ✅

  **What to do**:
  - 创建 `src/cube_animator.h` 和 `src/cube_animator.cpp`
  - 提取动画管理逻辑：
    ```cpp
    class CubeAnimator {
    public:
        CubeAnimator();
        
        // 动画控制
        void queueMove(Move move, bool recordHistory = true);
        void update(float deltaTime);
        void reset();
        
        // 状态查询
        bool isAnimating() const { return isAnimating_; }
        float progress() const { return animationProgress_; }
        Move currentMove() const { return currentMove_; }
        bool isCubeInAnimatingSlice(int cubeIndex) const;
        
        // 配置
        float animationSpeed = 1.0f;
        bool enableAnimation = true;
        
        // 动画状态快照（供渲染器使用）
        const RubiksCube& getPreAnimationCube() const { return preAnimationCube_; }
        float getCurrentAngle() const;  // 当前动画角度
        
        // 设置回调（动画完成时调用）
        using MoveCallback = std::function<void(Move, bool)>;
        void setMoveCompleteCallback(MoveCallback callback);
        
    private:
        bool isAnimating_ = false;
        float animationProgress_ = 0.0f;
        Move currentMove_ = Move::U;
        float rotationAngle_ = 90.0f;
        std::queue<Move> moveQueue_;
        RubiksCube preAnimationCube_;
        bool recordCurrentMoveHistory_ = true;
        MoveCallback moveCompleteCallback_;
        
        void startNextAnimation();
        bool isDoubleMove(Move move) const;
    };
    ```
  - 从 `renderer.h/cpp` 移除动画相关代码
  - **关键设计决策**：
    - `CubeAnimator` 不直接调用 `RubiksCube::executeMove()`
    - 通过回调通知外部"动画完成，请执行移动"
    - 这样保持 Animator 与 Cube 状态的解耦

  **Must NOT do**:
  - 不要改变动画时间（200ms base）或 easing 函数
  - 不要让 Animator 直接修改 Cube 状态

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: 复杂的状态管理提取，需要仔细处理回调
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Tasks 6, 7)
  - **Parallel Group**: Wave 2
  - **Blocks**: Task 7, 8
  - **Blocked By**: Task 2 (ViewState)

  **References**:
  - `src/renderer.h:79-86` - 当前动画状态成员
  - `src/renderer.cpp:28-52` - executeMove 实现
  - `src/renderer.cpp:837-895` - updateAnimation/startNextAnimation 实现
  - `src/renderer.cpp:1313-1399` - isCubeAnimating 实现
  - `src/renderer.cpp:1302-1310` - isDoubleMove 实现

  **Acceptance Criteria**:
  - [ ] `src/cube_animator.h/cpp` 存在
  - [ ] 动画播放正常，速度正确
  - [ ] 移动队列正常工作
  - [ ] `cmake --build build` 构建成功

  **QA Scenarios**:
  ```
  Scenario: Animation timing preserved
    Tool: Bash
    Steps:
      1. Start app
      2. Execute a move (R)
      3. Verify animation completes in ~200ms (manual observation or logging)
    Expected Result: Animation smooth, timing unchanged
    Evidence: .sisyphus/evidence/task-05-animation.log

  Scenario: Move queue works
    Tool: Bash
    Steps:
      1. Rapidly click R, L, U buttons
      2. Verify all moves execute in order
    Expected Result: No moves dropped, correct order
    Evidence: .sisyphus/evidence/task-05-queue.log
  ```

  **Commit**: YES
  - Message: `refactor(renderer): extract CubeAnimator class`
  - Files: `src/cube_animator.h, src/cube_animator.cpp, src/renderer.h, src/renderer.cpp, CMakeLists.txt`

- [x] 6. **提取 Renderer2D 类** ✅

  **What to do**:
  - 创建 `src/renderer_2d.h` 和 `src/renderer_2d.cpp`
  - 提取 2D 渲染逻辑：
    ```cpp
    class Renderer2D {
    public:
        Renderer2D();
        
        // 渲染 2D 展开视图
        void draw(ImDrawList* drawList, ImVec2 center, float scale,
                  const RubiksCube& cube, const ColorProvider& colors);
        
    private:
        void drawFace(ImDrawList* drawList, const std::array<Color, 9>& face,
                     ImVec2 offset, float size, float gap, 
                     bool flipVertical, Color faceType,
                     const ColorProvider& colors);
    };
    ```
  - **注意**：2D 渲染器不持有状态，通过参数接收：
    - `RubiksCube&` - 读取状态
    - `ColorProvider&` - 读取颜色配置

  **Must NOT do**:
  - 不要改变 2D 展开布局
  - 不要添加动画（2D 无动画）

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: 渲染逻辑提取，需要仔细处理参数传递
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Tasks 5, 7)
  - **Parallel Group**: Wave 2
  - **Blocks**: Task 8
  - **Blocked By**: Task 4 (ColorProvider)

  **References**:
  - `src/renderer.cpp:145-177` - draw2D 实现
  - `src/renderer.cpp:375-423` - drawFace 实现

  **Acceptance Criteria**:
  - [ ] `src/renderer_2d.h/cpp` 存在
  - [ ] 2D 展开视图显示正确
  - [ ] 6 个面位置和颜色正确
  - [ ] `cmake --build build` 构建成功

  **QA Scenarios**:
  ```
  Scenario: 2D view renders correctly
    Tool: Bash
    Steps:
      1. Start app
      2. Verify 2D net view shows 6 faces in cross pattern
    Expected Result: Correct layout, correct colors
    Evidence: .sisyphus/evidence/task-06-2d-view.log

  Scenario: 2D view reflects cube state
    Tool: Bash
    Steps:
      1. Execute move R
      2. Verify 2D view updates to show new state
    Expected Result: 2D view matches cube state
    Evidence: .sisyphus/evidence/task-06-2d-state.log
  ```

  **Commit**: YES
  - Message: `refactor(renderer): extract Renderer2D class`
  - Files: `src/renderer_2d.h, src/renderer_2d.cpp, src/renderer.h, src/renderer.cpp, CMakeLists.txt`

- [x] 7. **提取 Renderer3DOpenGL 类** ✅

  **What to do**:
  - 创建 `src/renderer_3d_opengl.h` 和 `src/renderer_3d_opengl.cpp`
  - 实现 `IRenderer3D` 接口：
    ```cpp
    class Renderer3DOpenGL : public IRenderer3D {
    public:
        Renderer3DOpenGL();
        ~Renderer3DOpenGL() override;
        
        // IRenderer3D 接口实现
        void render(int windowWidth, int windowHeight) override;
        void setViewState(const ViewState* state) override;
        void setColorProvider(const ColorProvider* provider) override;
        void setAnimator(const CubeAnimator* animator) override;
        
        // 设置 Cube 数据源
        void setCube(const RubiksCube* cube);
        
    private:
        Model* cubeModel_;
        const RubiksCube* cube_ = nullptr;
        const ViewState* viewState_ = nullptr;
        const ColorProvider* colorProvider_ = nullptr;
        const CubeAnimator* animator_ = nullptr;
        
        bool initGL();
        void drawCube(int cubeIndex, bool usePreAnimationState);
        void drawCircleCanvas();
        void applyRotationTransform(float angle, Move move);
    };
    ```
  - 从 `renderer.cpp` 迁移：
    - `initGL3D()` → `initGL()`
    - `render3DOverlay()` → `render()`
    - `drawCube()` → 保持
    - `drawCircleCanvas()` → 保持
    - `applyRotationTransform()` → 保持
    - `isCubeAnimating()` → 使用 `animator_->isCubeInAnimatingSlice()`

  **Must NOT do**:
  - 不要改变 OpenGL 渲染输出
  - 不要添加新的 OpenGL 特性

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: 复杂的 3D 渲染代码迁移
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES (with Tasks 5, 6)
  - **Parallel Group**: Wave 2
  - **Blocks**: Task 8
  - **Blocked By**: Task 3 (IRenderer3D), Task 4 (ColorProvider), Task 5 (Animator)

  **References**:
  - `src/renderer.cpp:120-143` - initGL3D 实现
  - `src/renderer.cpp:238-349` - render3DOverlay 实现
  - `src/renderer.cpp:1119-1282` - drawCube 实现
  - `src/renderer.cpp:793-835` - drawCircleCanvas 实现
  - `src/renderer.cpp:1313-1399` - isCubeAnimating 实现
  - `src/renderer.cpp:1403-1540` - applyRotationTransform 实现

  **Acceptance Criteria**:
  - [ ] `src/renderer_3d_opengl.h/cpp` 存在
  - [ ] 实现 `IRenderer3D` 接口
  - [ ] 3D 视图显示正确
  - [ ] 动画在 3D 视图中正常工作
  - [ ] `cmake --build build` 构建成功

  **QA Scenarios**:
  ```
  Scenario: 3D view renders correctly
    Tool: Bash
    Steps:
      1. Start app
      2. Verify 3D view shows cube with correct colors
    Expected Result: 3D cube visible, correct orientation
    Evidence: .sisyphus/evidence/task-07-3d-view.log

  Scenario: 3D animation works
    Tool: Bash
    Steps:
      1. Execute move R
      2. Verify right slice rotates smoothly
    Expected Result: Smooth rotation animation
    Evidence: .sisyphus/evidence/task-07-3d-animation.log

  Scenario: View rotation works
    Tool: Bash
    Steps:
      1. Drag mouse in 3D view
      2. Verify cube rotates accordingly
    Expected Result: Smooth view rotation
    Evidence: .sisyphus/evidence/task-07-view-rotation.log
  ```

  **Commit**: YES
  - Message: `refactor(renderer): extract Renderer3DOpenGL class`
  - Files: `src/renderer_3d_opengl.h, src/renderer_3d_opengl.cpp, src/renderer.h, src/renderer.cpp, CMakeLists.txt`

- [x] 8. **重构 CubeRenderer 为门面类** ✅

  **What to do**:
  - 重写 `src/cube_renderer.h` 和 `src/cube_renderer.cpp`
  - 新架构：
    ```cpp
    class CubeRenderer {
    public:
        // 构造函数：依赖注入 RubiksCube
        explicit CubeRenderer(const RubiksCube& cube);
        ~CubeRenderer();
        
        // 渲染接口
        void draw2D(ImDrawList* drawList, ImVec2 center, float scale);
        void render3DOverlay(int windowWidth, int windowHeight);
        
        // 移动操作（委托给 animator）
        void executeMove(Move move);
        void executeMove(Move move, bool recordHistory);
        void updateAnimation(float deltaTime);
        
        // Cube 操作（委托给 cube）
        void reset();
        void undo();
        void redo();
        std::vector<Move> scramble(int numMoves = 20);
        
        // 状态查询
        bool isAnimating() const;
        float animationProgress() const;
        bool isSolved() const;
        void dump() const;
        
        // 历史访问
        const std::vector<Move>& getMoveHistory() const;
        const std::vector<Move>& getRedoHistory() const;
        bool canRedo() const;
        
        // 视角控制
        void resetView();
        ViewState& viewState() { return viewState_; }
        
        // 颜色配置
        void setCustomColors(const ColorConfig& config);
        ColorProvider& colorProvider() { return colorProvider_; }
        
        // 3D 渲染器切换
        void setRenderer3D(std::unique_ptr<IRenderer3D> renderer);
        IRenderer3D* getRenderer3D() const { return renderer3D_.get(); }
        
        // 公开配置（供 UI 访问）
        float& animationSpeed() { return animator_.animationSpeed; }
        bool& enableAnimation() { return animator_.enableAnimation; }
        
    private:
        // 依赖注入的 Cube（不拥有）
        const RubiksCube& cube_;
        
        // 组件
        ViewState viewState_;
        ColorProvider colorProvider_;
        CubeAnimator animator_;
        std::unique_ptr<Renderer2D> renderer2D_;
        std::unique_ptr<IRenderer3D> renderer3D_;
        
        // 动画完成回调
        void onMoveComplete(Move move, bool recordHistory);
    };
    ```
  - **关键变更**：
    - 构造函数接收 `const RubiksCube&`，不拥有 cube
    - 组合各组件而非继承
    - 公开接口保持兼容（部分通过委托）

  **Must NOT do**:
  - 不要让 CubeRenderer 拥有 RubiksCube
  - 不要破坏现有的公共 API（尽量保持兼容）

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 架构整合，需要仔细处理组件组装
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 3 (sequential after Task 7)
  - **Blocks**: Task 9
  - **Blocked By**: Task 3, 5, 6, 7

  **References**:
  - `src/renderer.h` - 当前 CubeRenderer 接口
  - `src/main.cpp:256-265` - 当前初始化方式

  **Acceptance Criteria**:
  - [ ] `CubeRenderer` 构造函数接收 `const RubiksCube&`
  - [ ] 所有组件正确组装
  - [ ] 现有功能正常工作
  - [ ] `cmake --build build` 构建成功

  **QA Scenarios**:
  ```
  Scenario: All components integrated
    Tool: Bash
    Steps:
      1. cmake -S . -B build && make -C build
      2. ./build/rubiks-cube
    Expected Result: No crash, app starts normally
    Evidence: .sisyphus/evidence/task-08-integration.log

  Scenario: All features work through facade
    Tool: Bash
    Steps:
      1. Test: execute move, undo, redo, scramble, reset
      2. Verify each operation works
    Expected Result: All operations successful
    Evidence: .sisyphus/evidence/task-08-features.log
  ```

  **Commit**: YES
  - Message: `refactor(renderer): refactor CubeRenderer as facade`
  - Files: `src/cube_renderer.h, src/cube_renderer.cpp`

- [x] 9. **更新 main.cpp 适配新架构** ✅

  **What to do**:
  - 更新 `src/main.cpp` 以适配新的依赖注入模式：
    ```cpp
    int main(int argc, char* argv[]) {
        // ... GLFW/ImGui 初始化 ...
        
        // 1. 创建 RubiksCube（数据层）
        RubiksCube cube;
        
        // 2. 创建 CubeRenderer（视图层），注入 cube
        CubeRenderer renderer(cube);
        
        // 3. 加载配置
        ColorConfig config = loadColorConfig();
        renderer.setCustomColors(config);
        renderer.animationSpeed() = config.getAnimationSpeed();
        renderer.enableAnimation() = config.getEnableAnimation();
        
        // ... 主循环 ...
        
        // 更新所有 renderer.xxx 为新接口
        // 例如：renderer.rotationX → renderer.viewState().rotationX
        //       renderer.customFront → renderer.colorProvider().customFront
    }
    ```
  - 更新所有对 `renderer` 的访问：
    - `renderer.rotationX` → `renderer.viewState().rotationX`
    - `renderer.scale` → `renderer.viewState().scale3D`
    - `renderer.scale2D` → `renderer.viewState().scale2D`
    - `renderer.customFront` → `renderer.colorProvider().customFront`
    - 等等

  **Must NOT do**:
  - 不要改变应用行为
  - 不要添加新功能

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: 大量的调用点更新
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 3 (sequential after Task 8)
  - **Blocks**: Final Verification
  - **Blocked By**: Task 8

  **References**:
  - `src/main.cpp:256-265` - 当前初始化
  - `src/main.cpp` - 所有 `renderer.` 访问点

  **Acceptance Criteria**:
  - [ ] `RubiksCube` 在 `main.cpp` 中创建
  - [ ] `CubeRenderer` 通过依赖注入接收 cube
  - [ ] 所有 `renderer.xxx` 访问更新为新的接口
  - [ ] 应用功能正常
  - [ ] `cmake --build build` 构建成功
  - [ ] `ctest --test-dir build` 所有测试通过

  **QA Scenarios**:
  ```
  Scenario: main.cpp compiles with new architecture
    Tool: Bash
    Steps:
      1. cmake -S . -B build && make -C build
    Expected Result: Exit code 0, no errors
    Evidence: .sisyphus/evidence/task-09-build.log

  Scenario: All existing tests pass
    Tool: Bash
    Steps:
      1. ctest --test-dir build --output-on-failure
    Expected Result: All tests PASS
    Evidence: .sisyphus/evidence/task-09-tests.log

  Scenario: Full application smoke test
    Tool: Bash
    Steps:
      1. timeout 10 ./build/rubiks-cube
      2. Verify no crash or error output
    Expected Result: App runs without issues
    Evidence: .sisyphus/evidence/task-09-smoke.log

  Scenario: All UI controls work
    Tool: Bash
    Steps:
      1. Manual test: All move buttons (R, L, U, D, F, B, M, E, S, X, Y, Z)
      2. Manual test: Scramble, Reset, Undo, Redo
      3. Manual test: Settings tab (colors, animation, scales)
    Expected Result: All controls functional
    Evidence: .sisyphus/evidence/task-09-controls.log
  ```

  **Commit**: YES
  - Message: `refactor(main): adapt to new CubeRenderer architecture`
  - Files: `src/main.cpp, CMakeLists.txt`

## Final Verification Wave (MANDATORY — after ALL implementation tasks)

> 3 review agents run in PARALLEL. ALL must APPROVE.

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Verify each task: "What to do" matches actual diff. Check Must Have present, Must NOT Have absent.
  Output: `Tasks [N/N compliant] | Must Have [N/N] | Must NOT Have [N/N] | VERDICT`

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Run `cmake --build build` + `ctest --test-dir build`. Check for: compilation warnings, test failures.
  Output: `Build [PASS/FAIL] | Tests [N/N pass] | Warnings [N] | VERDICT`

- [ ] F3. **Full Functionality Smoke Test** — `unspecified-high` (+ `playwright` skill if GUI)
  Start app, execute: scramble → undo → redo → reset. Verify 2D/3D view renders correctly.
  Output: `Scenarios [N/N pass] | GUI [PASS/FAIL] | VERDICT`

---

## Commit Strategy

- **Atomic commits** after each task completion
- **Commit message format**: `refactor(renderer): {description}`
- Example: `refactor(renderer): extract CubeAnimator class from CubeRenderer`

- **Atomic commits** after each task completion
- **Commit message format**: `refactor(renderer): {description}`
- Example: `refactor(renderer): extract CubeAnimator class from CubeRenderer`

---

## Success Criteria

### Verification Commands
```bash
# Build verification
cmake -S . -B build && make -C build
# Expected: Exit code 0, no compilation errors

# Existing tests
ctest --test-dir build --output-on-failure
# Expected: All tests PASS

# Application startup (smoke test)
timeout 3 ./build/rubiks-cube || [ $? -eq 124 ]
# Expected: No crash, exits normally or via timeout
```

### Final Checklist
- [ ] All "Must Have" present
- [ ] All "Must NOT Have" absent
- [ ] All existing tests pass
- [ ] Build succeeds without warnings
- [ ] Application runs without crash
- [ ] 2D/3D views render correctly
- [ ] Animation timing unchanged
- [ ] Config file format unchanged
