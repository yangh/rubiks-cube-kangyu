Note:
* The cube algorithm is perfect now, no need to change.

### TODO

1. Test cases re-check/re-write
2. Resizeable and moveable 2d/3d/control view
3. Auto resize view on windows resizing
4. Auto save customiziable window layout, restore on startup
5. Controls re-org (de-dup color settings, merge history + moves etc)
6. Add more Formula and customization
   - manage formula file
   - manage formula entry
7. Add author info
8. Improve 3d scene
   - Add a circle canvas bigger than the cube on the bottom with distance as tall as cube.
   - Round corner sticker
9. Add better 3d model for cube

### 测试状态

当前测试结果（修复后）：
- test_cube: 113 passed / 4 failed (117 total)
- test_cube_2step: 288 passed / 0 failed
- test_formula: 部分通过
- test_ref_verify: 有失败项
- test_formula_ref: 有失败项

已知问题：
- F 和 B 面的旋转逻辑部分修正，但仍有测试失败
- R + R' 反向测试存在失败

### 待处理问题

1. **测试修复**: 仍有部分测试失败需要调试
   - rotateFront 和 rotateBack 的旋转顺序已修正
   - 需要进一步调试剩余的测试失败

2. **其他**:
   - Support GAN or MOYU smart cubic (未开始)

---

### 修改记录


TODO (剩余):

* Test cases fully passing (当前仍有部分测试失败)
* Support GAN or MOYU smart cubic
