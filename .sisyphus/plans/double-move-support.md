# Double Move Support (U2, D2, L2, R2, F2, B2, M2, E2, S2)

## TL;DR

> **Quick Summary**: Add support for double moves (180° rotations) using "X2" notation. Parser-based support, single 180° animation.
>
> **Deliverables**: Extended Move enum, parser support, execution logic, animation support, tests, documentation
>
> **Estimated Effort**: Medium
> **Parallel Execution**: YES - 4 waves
> **Critical Path**: Enum → Parser → Execution → Animation → Tests

---

## Context

### Original Request
"添加类似 U2 连续转动两次 U 的动作支持。目前的所有转动方式都要支持。"

### Interview Summary
- **Implementation**: Parser/formula support only (no UI buttons)
- **Animation**: Single 180° animation (not two 90°)
- **Execution**: Call rotation function twice internally
- **Test Strategy**: Tests after
- **Scope**: All 9 move types (6 faces + 3 slices)

---

## Work Objectives

Add double-move support with parser support, single 180° animation, and comprehensive testing.

### Must Have
- Parser support for "X2" notation
- Single 180° animation
- Inverse logic (U2 is self-inverse)
- Tests for all 9 double moves

### Must NOT Have
- NO UI buttons for double moves
- NO "X2'" notation
- NO triple moves

- NO lowercase "x2"

---

## Execution Strategy

Tasks are grouped into 2 parallel waves for maximum efficiency.

**Wave 1** (Foundation):
- Task 1: Extend Move enum (add U2, D2, L2, R2, F2, B2, M2, E2, S2)
- Task 2: Update moveToString() for double moves
- Task 3: Update parseMoveString() for "X2" notation

- Task 4: Fix lying comment in cube.h

**Wave 2** (Logic):
- Task 5: Update executeMove() for double moves
- Task 6: Update inverse logic in 3 locations
- Task 7: Update scramble() to include double moves
- Task 8: Add animation support for 180°

**Wave 3** (Verification):
- Task 9: Add tests for all double moves
- Task 10: Update documentation (README, CLAUDE.md)
- Task 11: Manual testing and all scenarios

- Task 12: Final build and test

---

## TODOs

- [x] 1. Extend Move enum

  **Files**: src/cube.h
  **What**: Add U2, D2, L2, R2, F2, B2, M2, E2, S2 enum values
  **Status**: ✅ Complete - 9 values added, commit 85faaee

- [x] 2. Update moveToString()

  **Files**: src/cube.cpp
  **What**: Add cases for U2, D2, L2, R2, F2, B2, M2, E2, S2 returning "U2", "D2", etc.
  **Status**: ✅ Complete - 9 cases added, commit 85faaee

- [x] 3. Update parseMoveString()

  **Files**: src/cube.cpp
  **What**: Parse "U2", "R2", etc. as double moves
  **Status**: ✅ Complete - X2 notation parsing, commit 85faaee

- [x] 4. Fix cube.h comment

  **Files**: src/cube.h
  **What**: Remove "F2" from comment (lying comment)
  **Status**: ✅ Complete - Comment fixed, commit 85faaee

- [ ] 5. Update executeMove()

  **Files**: src/cube.cpp
  **What**: Add cases for double moves (execute twice)

- [ ] 6. Update inverse logic

  **Files**: src/main.cpp, src/cube.cpp
  **What**: Add U2→U2, etc. (self-inverse)

- [ ] 7. Update scramble()

  **Files**: src/cube.cpp
  **What**: Include double moves in scramble array

- [ ] 8. Add animation support

  **Files**: src/renderer.h, src/renderer.cpp
  **What**: Support 180° animations

- [ ] 9. Add tests

  **Files**: tests/test_cube.cpp
  **What**: Test all 9 double moves

- [ ] 10. Update README.md

  **Files**: README.md
  **What**: Document double moves

- [ ] 11. Update CLAUDE.md

  **Files**: CLAUDE.md
  **What**: Update notation section

- [ ] 12. Manual testing

  **What**: Execute all double moves, verify animation, test undo/redo

- [ ] 13. Final build and run

  **What**: Build, run all tests, verify everything works
