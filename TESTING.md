# Testing Documentation

本文档描述了魔方模拟器项目的测试套件和验证方法。

## 测试概览

项目包含两个测试程序：

| 测试程序 | 文件 | 用途 | 测试数量 |
|---------|------|------|---------|
| `test_cube` | `tests/test_cube.cpp` | 基础功能测试 | 117 |
| `test_cube_2step` | `tests/test_cube_2step.cpp` | 2 步移动组合测试 | 288 |

## 运行测试

### 运行单个测试

```bash
# 编译并运行所有测试
cmake -S . -B build
make -C build

# 运行基础测试
./build/test_cube

# 运行 2 步组合测试
./build/test_cube_2step
```

### 使用 CTest 运行

```bash
# 运行所有测试
cd build
ctest

# 运行特定测试
ctest -R cube_logic    # 基础测试
ctest -R cube_2step    # 2 步组合测试
```

---

## test_cube - 基础功能测试

### Test 1: Initial State（初始状态测试）

验证魔方初始状态是已解决的。

**验证方法：**
- 检查 `cube.isSolved()` 返回 `true`
- 检查每个面是否为单一颜色（Front=绿色, Back=蓝色, Left=橙色, Right=红色, Up=白色, Down=黄色）

### Test 2: Inverse Moves（逆移动测试）

验证移动与其逆移动的组合能回到原始状态。

**测试用例：**
- U + U', D + D', L + L', R + R', F + F', B + B'
- M + M', E + E', S + S'

**验证方法：**
- 执行移动对前记录所有面的状态
- 执行移动对后比较所有面的状态
- 检查 `cube.isSolved()` 返回 `true`

### Test 3: Four Moves（四次移动测试）

验证同一移动执行四次能回到原始状态（360°旋转）。

**测试用例：**
- U ×4, D ×4, L ×4, R ×4, F ×4, B ×4
- M ×4, E ×4, S ×4

**验证方法：**
- 同 Test 2，比较执行前后的状态

### Test 4: Reset（重置测试）

验证 `cube.reset()` 能将魔方恢复到已解决状态。

**验证方法：**
- 打乱魔方后检查 `!cube.isSolved()`
- 调用 `reset()` 后检查 `cube.isSolved()` 和各面颜色

### Test 5: Move Patterns（移动模式测试）

测试常见的移动序列能正确执行。

**测试用例：**
- R U R' U' - 常见的触发器模式
- U D U' D' - 独立面移动
- M E M' E' - 切片移动组合
- S S' - 单个切片逆移动
- M S E M' S' E' - 复杂切片移动序列

**验证方法：**
- 确认序列执行不崩溃

### Test 6: Adjacent Faces Interaction（相邻面交互测试）

验证移动正确影响相邻面。

**测试用例：**

| 移动 | 影响的面和位置 |
|------|---------------|
| U | Front/Left/Back/Right 的第 0 行 (0,1,2) |
| F | Up 的第 2 行 (6,7,8), Right 的第 0 列 (0,3,6), Down 的第 0 行 (0,1,2), Left 的第 2 列 (2,5,8) |
| R | Up 的第 2 列 (2,5,8), Front 的第 2 列 (2,5,8), Down 的第 2 列 (2,5,8), Back 的第 0 列倒序 (6,3,0) |
| M | Up/Down/Front/Back 的中间列 (1,4,7) |
| E | Front/Back/Left/Right 的中间行 (3,4,5) |
| S | Up/Down 的中间行 (3,4,5), Left/Right 的中间列 (1,4,7) |

**验证方法：**
- 记录移动前相关面的状态
- 执行移动后检查受影响的位置确实改变
- 检查不受影响的位置保持不变

### Test 7: Scrambling（打乱测试）

验证复杂移动序列能使魔方进入未解决状态。

**测试用例：**
- 基础移动打乱：U, R, F, D, L, B 及其逆
- 包含切片移动打乱：M, E, S 及其逆，混合基础移动

**验证方法：**
- 执行打乱序列后检查 `!cube.isSolved()`

### Test 8: Color Conversion（颜色转换测试）

验证 `colorToRgb()` 函数正确将颜色枚举转换为 RGB 值。

**测试用例：**
- WHITE → (1.0, 1.0, 1.0)
- RED → (1.0, 0.0, 0.0)
- YELLOW → (1.0, 1.0, 0.0)

### Test 9: Move to String（移动转字符串测试）

验证 `moveToString()` 函数正确转换移动枚举为字符串表示。

**测试用例：**
- 所有 18 种移动：U, U', D, D', L, L', R, R', F, F', B, B', M, M', E, E', S, S'

### Test 10: Cube Consistency（方块一致性测试）

验证执行多次移动后魔方状态保持一致。

**测试用例：**
- R U F U' R' F' - 基础移动组合
- M E S M' E' S' - 切片移动组合
- U M E S S' E' M' U' - 混合移动（按逆序执行逆移动）

**验证方法：**
- 检查每个面有 9 个贴纸
- 对于可逆序列，检查 `cube.isSolved()`

### Test 11-15: Slice Move Tests（切片移动专项测试）

这些是 M/E/S 切片移动的专门测试，与 Test 2-6 对应：

- **Test 11**: 切片移动的逆移动测试
- **Test 12**: 切片移动的四次测试
- **Test 13**: 切片移动的相邻面交互（包含更详细的验证）
- **Test 14**: 切片移动的字符串转换
- **Test 15**: 切片移动的模式测试

---

## test_cube_2step - 2 步移动组合测试

### 测试目的

全面测试所有可能的 2 步移动组合，确保每种移动序列都能正确改变魔方状态。

### 测试范围

- **总移动数**：18 种（U/D/L/R/F/B/M/E/S 及其逆）
- **总组合数**：324（18 × 18）
- **排除组合**：36
  - 18 个重移动（U U, U' U', D D, ...）
  - 18 个逆向对（U U', D D', M M', ...）
- **实际执行**：288 个测试用例

### 测试方法

对每个有效的 2 步移动组合：

1. 创建新的已解决魔方
2. 记录所有 6 个面的初始状态
3. 执行第一个移动
4. 执行第二个移动
5. 检查至少有一个面发生了改变

### 验证逻辑

```
changed = (front != front_before) ||
          (back != back_before) ||
          (left != left_before) ||
          (right != right_before) ||
          (up != up_before) ||
          (down != down_before)
```

**预期结果**：`changed == true`

如果 `changed == false`，说明移动序列没有改变魔方状态，这可能是移动实现错误或移动抵消（已被排除）。

### 示例测试用例

| 移动序列 | 预期 | 说明 |
|---------|------|------|
| U D | ✓ 改变 | 独立面移动 |
| U U' | ✗ 跳过 | 逆移动对 |
| U U | ✗ 跳过 | 重移动 |
| M E | ✓ 改变 | 切片移动组合 |
| U M | ✓ 改变 | 基础和切片混合 |

---

## 测试验证原则

### 正确性验证

1. **状态一致性**：任何有效的移动序列都不应该导致魔方处于无效状态
2. **可逆性**：对于任何移动序列，按逆序执行其逆移动应回到原状态
3. **周期性**：同一移动执行 4 次应回到原状态

### 排除原则

以下情况被排除在测试之外，因为它们不提供额外验证价值：

1. **逆向对**（U + U'）：直接抵消，已由 Test 2 覆盖
2. **重移动**（U + U）：相当于执行两次，简化版本已由 Test 3 覆盖

---

## 添加新测试

### 添加基础测试

在 `tests/test_cube.cpp` 中：

```cpp
void testNewFeature() {
    std::cout << "\n=== Test New Feature ===" << std::endl;

    RubiksCube cube;
    // 测试逻辑

    assertTest("Test name", condition);
}
```

并在 `main()` 函数中调用。

### 添加 CMake 测试

编辑 `CMakeLists.txt`：

```cmake
add_executable(your_test
    tests/your_test.cpp
    src/cube.cpp
)

target_include_directories(your_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME your_test COMMAND your_test)
```

---

## 测试覆盖率

当前测试覆盖的移动功能：

- ✅ 所有 12 种基础移动（U/D/L/R/F/B 及其逆）
- ✅ 所有 6 种切片移动（M/E/S 及其逆）
- ✅ 移动逆组合
- ✅ 移动四倍组合
- ✅ 相邻面交互
- ✅ 所有 2 步移动组合（排除无效组合）
- ✅ 颜色转换
- ✅ 移动字符串转换
- ✅ 重置功能
- ✅ 状态检查
