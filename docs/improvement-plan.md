# 代码质量改进实施计划

> **项目**: rubiks-cube-kangyu
> **创建日期**: 2026-03-28
> **基于**: code-quality-report.md v2
> **预计改动文件**: 15+

---

## 阶段 1：Critical Bug + 假测试修复

**目标**: 消除未定义行为、逻辑 Bug 和无效测试
**预计工时**: 2-3 小时

### 1.1 修复 cube.h 中 `.back()` 未定义行为

**文件**: `src/cube.h:32-33`

**改动**: 为 `getLastMove()` 和 `getLastRedo()` 添加空检查，返回 `std::optional<Move>` 或添加 assert。

```cpp
// 方案 A: 使用 assert（推荐，保持现有接口）
#include <cassert>
Move getLastMove() { assert(!moveHistory_.empty()); return moveHistory_.back(); }
Move getLastRedo() { assert(!redoHistory_.empty()); return redoHistory_.back(); }

// 方案 B: 使用 optional（更安全但改动大）
std::optional<Move> getLastMove() const {
    return moveHistory_.empty() ? std::nullopt : std::optional<Move>(moveHistory_.back());
}
```

**验收**: 编译通过，现有测试不受影响。

### 1.2 修复 animator 队列 recordHistory flag 覆盖 Bug

**文件**: `src/animator.h`, `src/animator.cpp`

**改动**: 将 `recordHistory` 存储在每个队列项中而非共享变量。

```cpp
// animator.h — 新增结构体
struct PendingMove {
    Move move;
    bool recordHistory;
};

// 替换 std::queue<Move> moveQueue_ 为 std::queue<PendingMove> moveQueue_
// 删除 bool recordCurrentMoveHistory_ 成员
```

**关联修改**: `animator.cpp` 中 `queueMove()`, `startNextAnimation()`, `update()`, `reset()` 需适配新结构体。

**验收**: 编译通过。手动测试 undo/redo 功能正常。队列多 move 时各自 recordHistory 正确。

### 1.3 修复 test_cube.cpp 中的恒真断言（6处自比较）

**文件**: `tests/test_cube.cpp:617-618, 646-647, 679-680`

**改动**: 将自比较替换为与 move 前快照比较。

```cpp
// Before (恒真):
assertTest("M move does not affect left face", cube.getLeft() == cube.getLeft());

// After (正确):
auto leftBefore = cube.getLeft();   // 在 executeMove(Move::M) 之前保存
assertTest("M move does not affect left face", cube.getLeft() == leftBefore);
```

涉及 6 处修复：M move 的 left/right (2处), E move 的 up/down (2处), S move 的 front/back (2处)。需在对应测试块中添加 `auto xxxBefore = cube.getXxx();` 快照保存。

**验收**: `test_cube` 测试全部通过，无断言失败。

### 1.4 修复 test_formula.cpp 循环公式测试逻辑

**文件**: `tests/test_formula.cpp:269-274`

**改动**: 让第 i 个 cube 应用公式 i+1 次（而非 1 次），验证循环后状态一致。

```cpp
// Before:
for (int i = 0; i < numCubes; ++i) {
    cubes.emplace_back();
    for (Move move : item.moves) {
        cubes[i].executeMove(move);
    }
}

// After:
for (int i = 0; i < numCubes; ++i) {
    cubes.emplace_back();
    for (int j = 0; j <= i; ++j) {  // 第 i 个 cube 应用 i+1 次
        for (Move move : item.moves) {
            cubes[i].executeMove(move);
        }
    }
}
```

**验收**: `test_formula` 通过。如果某些公式循环次数不正确，测试会正确报告失败。

### 1.5 修复 test_ref_verify.cpp 双步 move 映射缺失

**文件**: `tests/test_ref_verify.cpp:29-57`

**改动**: 在 `toRefMove()` switch 中添加所有双步 move 的映射。

```cpp
case Move::U2: return ref::Move::U;   // 映射为 base move
case Move::D2: return ref::Move::D;
case Move::L2: return ref::Move::L;
// ... 所有 12 个双步 move
```

需同步确认调用方对双步 move 做了两次 `executeMove`（参考 `test_formula_ref.cpp` 的做法）。

**验收**: `test_ref_verify` 通过。

### 1.6 添加 `const` 到 `getLastMove()` / `getLastRedo()`

**文件**: `src/cube.h:32-33`

```cpp
Move getLastMove() const { ... }
Move getLastRedo() const { ... }
```

---

## 阶段 2：编译安全 + 封装改进

**目标**: 激活编译器警告、改善封装、消除全局状态
**预计工时**: 3-4 小时

### 2.1 添加编译器警告标志

**文件**: `CMakeLists.txt`

在 `set(CMAKE_CXX_STANDARD_REQUIRED ON)` 之后添加：

```cmake
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow)
endif()
```

**验收**: `cmake --build build` 无新增警告（需先修复阶段 1 的代码）。

### 2.2 封装 CubeRenderer 公共成员

**文件**: `src/renderer.h`, `src/renderer.cpp`, `src/app.cpp`

**改动**: 将以下成员从 public 移到 private，添加 getter/setter：

| 原 public 成员 | 新接口 |
|----------------|--------|
| `viewState_` | `ViewState& viewState()` / `const ViewState& viewState() const` |
| `animator_` | 逐个暴露需要的方法（见下） |
| `colorProvider_` | `ColorProvider& colorProvider()` |

对 `animator_` 的直接访问（`enableAnimation`, `animationSpeed`, `easingType`）改为通过 `CubeRenderer` 代理方法：

```cpp
// renderer.h 新增:
bool isAnimationEnabled() const;
void setAnimationEnabled(bool enabled);
float animationSpeed() const;
void setAnimationSpeed(float speed);
EasingType easingType() const;
void setEasingType(EasingType type);
```

**关联修改**: `app.cpp` 中所有 `renderer_->animator_.xxx` 改为 `renderer_->setXxx()` / `renderer_->xxx()`。

**验收**: 编译通过，功能无变化。

### 2.3 消除全局状态 g_enableDump

**文件**: `src/main.cpp`, `src/renderer.h`, `src/renderer.cpp`, `src/animator.h`, `src/animator.cpp`

**改动**: 
1. `g_enableDump` 移入 `CubeRenderer`（或 `Application`）作为成员
2. 通过构造函数注入或 setter 传递给 `CubeAnimator`
3. 删除所有 `extern bool g_enableDump` 声明

```cpp
// main.cpp:
Application app;
app.setEnableDump(enableDump);  // 已有此接口

// app.cpp initApp():
this->renderer_ = std::make_unique<CubeRenderer>(this->cube_, enableDump_);

// CubeRenderer 构造函数接收并传递给 animator:
CubeRenderer::CubeRenderer(RubiksCube& cube, bool enableDump = false)
    : cube_(cube), enableDump_(enableDump) {
    animator_.setEnableDump(enableDump_);
}

// animator 存储 enableDump_ 并用于条件输出
```

**验收**: 编译通过，`-d` 命令行参数仍正常工作。

### 2.4 统一 undo 路径

**文件**: `src/app.cpp:450-454`

**改动**: 按钮 Undo 改为走 `CubeRenderer::executeMove()`，与键盘快捷键一致。

```cpp
// Before:
this->renderer_->animator_.queueMove(inverseMove, false);
this->cube_.undo();

// After:
this->renderer_->executeMove(inverseMove, false);
this->cube_.undo();
```

**验收**: 编译通过，按钮 Undo 和键盘 Ctrl+Z 行为一致。

### 2.5 修复 Makefile 规范

**文件**: `Makefile`

```makefile
.PHONY: all test run clean

all:
	cmake -S . -B build
	cmake --build build

run: all
	./build/rubiks-cube

test:
	cmake --build build
	cd build && ctest --output-on-failure

clean:
	rm -rf build
```

**验收**: `make`, `make run`, `make test`, `make clean` 全部正常工作。

---

## 阶段 3：代码质量提升

**目标**: 消除魔数、重复代码、性能瓶颈
**预计工时**: 4-5 小时

### 3.1 提取魔数为命名常量

**文件**: `src/app.h`, `src/app.cpp`, `src/renderer_2d.cpp`, `src/renderer_3d_opengl.cpp`, `src/renderer_3d_shader.cpp`

**改动示例**:

```cpp
// app.h — 窗口相关常量
inline constexpr int kDefaultWindowWidth = 1400;
inline constexpr int kDefaultWindowHeight = 900;
inline constexpr float kSidebarWidth = 480.0f;
inline constexpr float kNetViewHeight = 300.0f;

// renderer_2d.cpp — 2D 渲染常量
inline constexpr float kStickerSize = 30.0f;
inline constexpr float kStickerGap = 1.0f;
inline constexpr float kFaceGap = 3.0f;
inline constexpr float kCornerRadiusRatio = 0.12f;
```

### 3.2 消除 getCubeFace() 重复代码

**文件**: 新增 `src/renderer_3d_common.h`，修改 `renderer_3d_opengl.cpp`, `renderer_3d_shader.cpp`

提取共享的 `getCubeFace()` 函数到独立头文件。

### 3.3 Shader uniform location 缓存

**文件**: `src/renderer_3d_shader.cpp`

**改动**: 在 `buildShaders()` 后预查所有 uniform locations 存入 `std::unordered_map<std::string, GLint>` 或成员变量。

```cpp
// Before (每帧):
char name[64];
snprintf(name, sizeof(name), "cubiePositions[%d]", i);
GLint loc = shader_.getUniformLocation(name);

// After (启动时):
struct UniformLocations {
    std::vector<GLint> cubiePositions;
    std::vector<GLint> stickerColors;
    // ...
};
```

### 3.4 修复公式计数器

**文件**: `src/formula.cpp`

**改动**: 将 `static int formulaCounter` 改为 `FormulaManager` 成员变量，在 `loadFormulas()` / `refresh()` 时重置。

### 3.5 配置解析加固

**文件**: `src/config.cpp`

**改动**:
1. `static_cast<RendererType>(stoi(value))` 添加范围校验
2. `catch(...)` 改为 `catch(const std::invalid_argument&)` + `catch(const std::out_of_range&)`
3. RGB 值添加 `[0, 1]` 范围裁剪

### 3.6 Shader 编译错误处理

**文件**: `src/renderer_3d_shader.cpp`, `src/shader.cpp`

**改动**: 
- `buildShaders()` 编译失败时设置 `shaderValid_ = false`
- `render()` 在 `shaderValid_ == false` 时跳过渲染并显示错误提示
- `setInt/setFloat` 等检查 location != -1

### 3.7 修复 test_cube.cpp 转义和其他 P1 问题

**文件**: `tests/test_cube.cpp`

**改动**:
- `\\n` → `\n` (多处)
- 移除 `assertTest("...", true)` 恒真断言，或替换为有意义的检查
- 修复测试描述字符串的 copy-paste 错误
- 移除未使用的 `#include <cassert>`

### 3.8 修复 test_formula_ref.cpp 边缘测试计数器

**文件**: `tests/test_formula_ref.cpp:452-489`

**改动**: 在 `testEdgeCasesVsRef()` 结束时调用 `updateGlobalCounters(edgeTestsPassed, edgeTestsFailed)`。

---

## 阶段 4：测试增强

**目标**: 扩展测试覆盖、添加 undo/redo 和 config 测试
**预计工时**: 3-4 小时

### 4.1 新增 undo/redo 测试

**新建文件**: `tests/test_undo_redo.cpp`

**测试内容**:
- 基本 undo/redo 循环：执行 move → undo → 验证恢复 → redo → 验证重做
- 连续多次 undo
- undo 后执行新 move 清空 redo 历史
- 空 history 时 `canUndo()` 返回 false
- `getLastMove()` / `getLastRedo()` 在有历史时返回正确值
- `executeMove(move, false)` 不记录历史
- 混合 `recordHistory=true/false` 的序列

### 4.2 新增 config 解析测试

**新建文件**: `tests/test_config.cpp`

**测试内容**:
- 默认配置值正确
- 保存/加载配置往返一致
- 枚举值边界校验（RendererType 无效值）
- 缺失配置文件时返回默认值
- RGB 值范围裁剪

### 4.3 更新 CMakeLists.txt

添加新的测试目标。

### 4.4 修复剩余 P1 测试问题

- 统一测试辅助函数（减少重复代码）
- 考虑引入 Catch2 作为测试框架（可选，长期改进）

---

## 验收标准

### 每阶段验收

| 阶段 | 验收命令 | 预期结果 |
|------|----------|----------|
| 1 | `make test` | 全部测试通过，无 false positive |
| 2 | `cmake -S . -B build && cmake --build build` | 零警告，构建成功 |
| 3 | `make test` | 全部测试通过 |
| 4 | `make test` | 新增测试全部通过，覆盖率提升 |

### 最终验收

```bash
make clean
make
make test
./build/rubiks-cube -d  # 功能验证
```

- 零编译警告
- 全部测试通过
- 无功能回退

---

*计划创建日期: 2026-03-28*
