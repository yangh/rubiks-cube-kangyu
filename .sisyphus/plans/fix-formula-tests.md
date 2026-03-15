# Fix Formula Tests - Double Move Support

## TL;DR

> **Quick Summary**: Fix test_formula_ref.cpp to handle double moves when comparing with reference implementation.
>
> **Deliverables**:
> - Fixed `tests/test_formula_ref.cpp` to convert double moves to two single moves
> - All 7 test suites passing (currently 5/7 passing)
>
> **Estimated Effort**: Quick (single function modification)
> **Critical Path**: Fix conversion → Rebuild → Verify tests

---

## Context

### Root Cause
The `toRefMove()` function in `tests/test_formula_ref.cpp` doesn't handle double moves (U2, D2, L2, R2, F2, B2, M2, E2, S2). When a double move is encountered:
- Our cube: Executes U2 correctly (180° rotation)
- Reference cube: Falls through to `return ref::Move::U` (wrong!)

This causes formula tests with double moves to fail because the reference cube doesn't execute the same moves.

### Current Test Failures
1. **test_formula**: 23 failures - "solved check" failures (test design issue)
2. **test_formula_ref**: 7 failures - double moves not converted properly

---

## Work Objectives

### Core Objective
Modify the formula test code to properly handle double moves when applying them to the reference cube.

### Must Have
- Update move application loop to convert double moves to two single moves
- Rebuild and verify all tests pass

### Must NOT Have
- DO NOT modify `src/` files (main algorithm is correct)
- DO NOT add double moves to reference implementation (unnecessary complexity)
- DO NOT change the Move enum

---

## TODOs

- [ ] 1. Fix double move handling in test_formula_ref.cpp

  **What to do**:
  - Modify the move application loop (around line 145-148) to handle double moves
  - For double moves, execute the corresponding single move twice on the reference cube
  - Pattern: U2 → U + U, D2 → D + D, etc.

  **Implementation**:
  Create a helper function to convert double moves:
  ```cpp
  ref::Move getRefBaseMove(Move move) {
      switch (move) {
          case Move::U: case Move::U2: return ref::Move::U;
          case Move::UP: return ref::Move::UP;
          case Move::D: case Move::D2: return ref::Move::D;
          case Move::DP: return ref::Move::DP;
          // ... etc
      }
  }
  
  bool isDoubleMove(Move move) {
      switch (move) {
          case Move::U2: case Move::D2: case Move::L2: case Move::R2:
          case Move::F2: case Move::B2: case Move::M2: case Move::E2: case Move::S2:
          case Move::X2: case Move::Y2: case Move::Z2:
              return true;
          default:
              return false;
      }
  }
  ```
  
  Then in the loop:
  ```cpp
  for (Move move : item.moves) {
      ourCube.executeMove(move);
      ref::Move refMove = getRefBaseMove(move);
      refCube.executeMove(refMove);
      if (isDoubleMove(move)) {
          refCube.executeMove(refMove);  // Execute twice for double moves
      }
  }
  ```

- [ ] 2. Rebuild and verify

  **What to do**:
  - Run `cmake -S . -B build && make -C build`
  - Run `./build/test_formula_ref` to verify passes
  - Run `ctest` to verify all 7 tests pass

---

## Success Criteria

- [ ] test_formula_ref: 0 failures
- [ ] ctest: 100% tests passed, 0 tests failed out of 7
