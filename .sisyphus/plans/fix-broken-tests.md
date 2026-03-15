# Fix Broken Tests - S Move Bug

## TL;DR

> **Quick Summary**: Fix out-of-bounds array access in reference implementation's S move (standing slice rotation) that causes 174+ test failures.
>
> **Deliverables**:
> - Fixed `tests/ref/ref_cube.cpp` S move implementation
> - All 7 test suites passing (currently 3/7 failing)
>
> **Estimated Effort**: Quick (2-3 line changes + rebuild)
> **Parallel Execution**: NO - sequential single-file fix
> **Critical Path**: Fix S move → Rebuild → Verify tests

---

## Context

### Original Request
Fix broken tests one by one. The main algorithm in `src/` is verified manually as correct, so do NOT touch it.

### Interview Summary
**Key Discussions**:
- Test failures: 3 out of 7 test suites failing
- Root cause identified: S move in reference implementation has array size bug
- Main implementation confirmed correct by user
- Only fix test files, never touch src/

**Research Findings**:
- S move uses `std::array<Color, 3>` but accesses indices [3,4,5] (out of bounds!)
- Should use `std::array<Color, 9>` with full face copy
- Z move depends on S (Z = F S B'), so Z failures are caused by S bug
- All 174 failures traced back to S move bug

### Metis Review
**Identified Gaps** (addressed):
- **Gap**: Initialization also wrong, not just array size
  - **Resolution**: Change from `{up_[3], up_[4], up_[5]}` to just `up_`
- **Gap**: Need explicit rebuild and verification commands
  - **Resolution**: Added specific QA scenarios with exact commands
- **Gap**: Formula test failures might be separate issue
  - **Resolution**: Clarified formula tests will pass once S move is fixed

---

## Work Objectives

### Core Objective
Fix the out-of-bounds array access bug in the reference implementation's S move, making all 7 test suites pass.

### Concrete Deliverables
- `tests/ref/ref_cube.cpp` lines 227 and 234: Fixed array declarations
- All test executables: `test_ref_verify`, `test_formula_verify`, `test_formula_ref_verify` passing

### Definition of Done
- [ ] `./build/test_ref_verify` outputs 0 failures
- [ ] `./build/test_formula_verify` outputs 0 failures
- [ ] `./build/test_formula_ref_verify` outputs 0 failures
- [ ] `ctest` shows 7/7 tests passed

### Must Have
- Fix both S move temp declarations (lines 227 and 234)
- Change `std::array<Color, 3>` to `std::array<Color, 9>`
- Change `{up_[3], up_[4], up_[5]}` to `up_`
- Rebuild and verify

### Must NOT Have (Guardrails)
- **DO NOT** modify `src/cube.cpp` or `src/cube.h` (user confirmed correct)
- **DO NOT** refactor or "improve" the S move logic
- **DO NOT** add comments explaining the fix
- **DO NOT** fix "similar issues" in other moves (only fix what's failing)
- **DO NOT** change any test logic beyond the array declarations
- **DO NOT** create new test cases

---

## Verification Strategy (MANDATORY)

> **ZERO HUMAN INTERVENTION** — ALL verification is agent-executed. No exceptions.

### Test Decision
- **Infrastructure exists**: YES (CMake + CTest)
- **Automated tests**: Tests-after (fix first, then verify)
- **Framework**: CTest + custom test executables
- **If TDD**: N/A (bug fix, not TDD workflow)

### QA Policy
Every task MUST include agent-executed QA scenarios (see TODO template below).
Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

- **Test executables**: Use Bash — Run test executables, capture output, parse results
- **Build verification**: Use Bash — Run cmake and make, check exit codes
- **Specific move verification**: Use Bash + grep — Extract S/Z move results from test output

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Sequential - single file fix):
└── Task 1: Fix S move array declarations in ref_cube.cpp [quick]

Wave 2 (After fix - rebuild):
└── Task 2: Rebuild project with fixed code [quick]

Wave 3 (After rebuild - verification):
└── Task 3: Run and verify all test suites pass [quick]

Critical Path: Task 1 → Task 2 → Task 3
Sequential: YES (must fix before rebuild, must rebuild before verify)
```

### Dependency Matrix

- **1**: — — 2
- **2**: 1 — 3
- **3**: 2 — —

### Agent Dispatch Summary

- **Wave 1**: Task 1 → `quick`
- **Wave 2**: Task 2 → `quick`
- **Wave 3**: Task 3 → `quick`

---

## TODOs

- [ ] 1. Fix S move array declarations in ref_cube.cpp

  **What to do**:
  - Open `tests/ref/ref_cube.cpp`
  - Locate line 227 in the S move prime branch: `std::array<Color, 3> temp = {up_[3], up_[4], up_[5]};`
  - Change to: `std::array<Color, 9> temp = up_;`
  - Locate line 234 in the S move non-prime branch: `std::array<Color, 3> temp = {up_[3], up_[4], up_[5]};`
  - Change to: `std::array<Color, 9> temp = up_;`
  - Save the file
  - Verify no other changes made to the file

  **Must NOT do**:
  - Do NOT modify any other lines in the file
  - Do NOT change the logic of the S move rotation
  - Do NOT modify src/cube.cpp or src/cube.h
  - Do NOT add comments or whitespace changes
  - Do NOT fix "similar issues" in other move functions

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Single-file, 2-line change, straightforward substitution
  - **Skills**: []
    - No special skills needed for simple text replacement
  - **Skills Evaluated but Omitted**:
    - `git-master`: Not needed - simple edit, commit comes later
    - `frontend-ui-ux`: No UI changes

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 1 (Sequential)
  - **Blocks**: Task 2 (rebuild), Task 3 (verification)
  - **Blocked By**: None (can start immediately)

  **References** (CRITICAL - Be Exhaustive):

  **Pattern References** (existing code to follow):
  - `src/cube.cpp:594` - Correct S' implementation: `std::array<Color, 9> temp = up_;`
  - `src/cube.cpp:601` - Correct S implementation: `std::array<Color, 9> temp = up_;`

  **API/Type References** (contracts to implement against):
  - N/A - No API changes, just bug fix

  **Test References** (testing patterns to follow):
  - N/A - Fixing tests, not adding new tests

  **External References** (libraries and frameworks):
  - N/A - Standard C++ array usage

  **WHY Each Reference Matters**:
  - `src/cube.cpp:594,601`: Shows the CORRECT pattern to copy - full face array with 9 elements
  - The reference implementation must match this pattern for S move to work correctly

  **Acceptance Criteria**:
  - [ ] Line 227 changed from `array<Color, 3>` to `array<Color, 9>` with `up_` initialization
  - [ ] Line 234 changed from `array<Color, 3>` to `array<Color, 9>` with `up_` initialization
  - [ ] No other lines modified in the file
  - [ ] File compiles without errors

  **QA Scenarios (MANDATORY — task is INCOMPLETE without these):**

  ```
  Scenario: Verify correct array size after fix
    Tool: Bash (grep)
    Preconditions: File tests/ref/ref_cube.cpp exists
    Steps:
      1. grep -n "std::array<Color, 3>" tests/ref/ref_cube.cpp
      2. Assert: No output (no 3-element arrays remain in S move)
      3. grep -n "std::array<Color, 9> temp = up_;" tests/ref/ref_cube.cpp
      4. Assert: Two lines found (lines 227 and 234)
    Expected Result: Zero matches for array<Color,3>, two matches for array<Color,9> temp = up_
    Failure Indicators: Any lines with array<Color,3> in S move context
    Evidence: .sisyphus/evidence/task-1-array-fix.txt

  Scenario: Verify only specified lines changed
    Tool: Bash (git diff)
    Preconditions: Git repository clean before fix
    Steps:
      1. git diff tests/ref/ref_cube.cpp
      2. Count lines starting with '-' or '+' (excluding metadata)
      3. Assert: Exactly 2 lines changed (227 and 234)
      4. Verify no changes to src/ directory: git diff src/
      5. Assert: No output (src/ untouched)
    Expected Result: Only 2 lines modified in ref_cube.cpp, no src/ changes
    Failure Indicators: Changes to other lines, changes to src/, logic modifications
    Evidence: .sisyphus/evidence/task-1-scope-check.txt
  ```

  **Evidence to Capture**:
  - [ ] Grep output showing array<Color,3> removed
  - [ ] Grep output showing array<Color,9> temp = up_ added (2 occurrences)
  - [ ] Git diff showing only 2 lines changed

  **Commit**: NO (commits with Task 3 after all tests pass)

---

- [ ] 2. Rebuild project with fixed code

  **What to do**:
  - Run cmake to reconfigure: `cmake -S . -B build`
  - Run make to rebuild: `make -C build -j4`
  - Verify all executables built successfully
  - Check for compilation errors or warnings

  **Must NOT do**:
  - Do NOT clean build directory (incremental build is fine)
  - Do NOT modify CMakeLists.txt
  - Do NOT skip the rebuild step

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Standard build command execution, straightforward
  - **Skills**: []
    - No special skills needed for running build commands
  - **Skills Evaluated but Omitted**:
    - All skills - simple command execution

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 2 (Sequential)
  - **Blocks**: Task 3 (verification)
  - **Blocked By**: Task 1 (fix must be applied first)

  **References** (CRITICAL - Be Exhaustive):

  **Pattern References** (existing code to follow):
  - `CMakeLists.txt` - Build configuration (no changes needed)

  **API/Type References** (contracts to implement against):
  - N/A

  **Test References** (testing patterns to follow):
  - N/A

  **External References** (libraries and frameworks):
  - N/A

  **WHY Each Reference Matters**:
  - No references needed - standard rebuild after code fix

  **Acceptance Criteria**:
  - [ ] cmake command exits with code 0
  - [ ] make command exits with code 0
  - [ ] All test executables present in build/ directory:
    - build/test_cube
    - build/test_cube_2step
    - build/test_ref_verify
    - build/test_formula
    - build/test_formula_ref
    - build/test_axis_rotate
    - build/test_color_validation

  **QA Scenarios (MANDATORY — task is INCOMPLETE without these):**

  ```
  Scenario: Verify successful rebuild
    Tool: Bash (cmake + make)
    Preconditions: Task 1 completed (array fix applied)
    Steps:
      1. cmake -S . -B build 2>&1 | tee /tmp/cmake.log
      2. Assert: Exit code = 0
      3. make -C build -j4 2>&1 | tee /tmp/make.log
      4. Assert: Exit code = 0
      5. Check for errors: grep -i "error" /tmp/make.log
      6. Assert: No output (no errors)
      7. Verify test executables exist: ls -1 build/test_* | wc -l
      8. Assert: 7 executables found
    Expected Result: Build completes with exit code 0, all 7 test executables created
    Failure Indicators: Non-zero exit codes, compilation errors, missing executables
    Evidence: .sisyphus/evidence/task-2-build.log

  Scenario: Verify ref_cube.o recompiled
    Tool: Bash (ls + stat)
    Preconditions: Build completed
    Steps:
      1. stat -c "%Y %n" build/CMakeFiles/test_ref_verify.dir/tests/ref/ref_cube.cpp.o
      2. Compare timestamp with ref_cube.cpp modification time
      3. Assert: Object file is newer than source file (or same time)
    Expected Result: Object file recompiled after source change
    Failure Indicators: Object file older than source (stale build)
    Evidence: .sisyphus/evidence/task-2-recompile-check.txt
  ```

  **Evidence to Capture**:
  - [ ] cmake output log
  - [ ] make output log
  - [ ] ls output showing 7 test executables

  **Commit**: NO (commits with Task 3 after tests pass)

---

- [ ] 3. Run and verify all test suites pass

  **What to do**:
  - Run test_ref_verify executable and capture output
  - Verify S, S', Z, Z' moves pass (4 specific tests)
  - Count total failures - must be 0
  - Run test_formula_verify and verify 0 failures
  - Run test_formula_ref_verify and verify 0 failures
  - Run ctest to verify all 7 tests pass
  - Create git commit if all tests pass

  **Must NOT do**:
  - Do NOT skip any test executable
  - Do NOT ignore test failures
  - Do NOT modify test code to make tests pass
  - Do NOT proceed to commit if any tests fail

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Running tests and parsing output, straightforward verification
  - **Skills**: []
    - No special skills needed for running test commands
  - **Skills Evaluated but Omitted**:
    - `git-master`: Will be invoked separately for commit if needed
    - All other skills - simple test execution

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 3 (Sequential)
  - **Blocks**: Final verification wave
  - **Blocked By**: Task 2 (must rebuild first)

  **References** (CRITICAL - Be Exhaustive):

  **Pattern References** (existing code to follow):
  - N/A

  **API/Type References** (contracts to implement against):
  - N/A

  **Test References** (testing patterns to follow):
  - `tests/test_ref_verify.cpp` - Test that verifies our implementation matches reference
  - `tests/test_formula_verify.cpp` - Test formula parsing
  - `tests/test_formula_ref_verify.cpp` - Test formulas against reference

  **External References** (libraries and frameworks):
  - N/A

  **WHY Each Reference Matters**:
  - Understanding what each test validates helps interpret failure messages

  **Acceptance Criteria**:
  - [ ] test_ref_verify: 0 failures (was 174 before fix)
  - [ ] test_formula_verify: 0 failures (was 3 before fix)
  - [ ] test_formula_ref_verify: 0 failures (was 15 before fix)
  - [ ] ctest output: "100% tests passed, 0 tests failed out of 7"
  - [ ] S move verification: [PASS] S and [PASS] S' in output
  - [ ] Z move verification: [PASS] Z and [PASS] Z' in output

  **QA Scenarios (MANDATORY — task is INCOMPLETE without these):**

  ```
  Scenario: Verify test_ref_verify passes with 0 failures
    Tool: Bash (test execution + grep)
    Preconditions: Build completed successfully
    Steps:
      1. ./build/test_ref_verify 2>&1 | tee /tmp/test_ref_verify.log
      2. Assert: Exit code = 0
      3. grep -c "^\[FAIL\]" /tmp/test_ref_verify.log
      4. Assert: Result = 0 (zero failures)
      5. grep "^\[PASS\] S$\|^\[PASS\] S'$\|^\[PASS\] Z$\|^\[PASS\] Z'$" /tmp/test_ref_verify.log | wc -l
      6. Assert: Result = 4 (all S/Z moves pass)
    Expected Result: 0 failures, S/S'/Z/Z' all show [PASS]
    Failure Indicators: Any [FAIL] lines, missing S/Z PASS lines
    Evidence: .sisyphus/evidence/task-3-ref-verify.log

  Scenario: Verify test_formula_verify passes
    Tool: Bash (test execution + grep)
    Preconditions: Build completed successfully
    Steps:
      1. ./build/test_formula_verify 2>&1 | tee /tmp/test_formula_verify.log
      2. Assert: Exit code = 0
      3. grep -c "^\[FAIL\]" /tmp/test_formula_verify.log
      4. Assert: Result = 0 (zero failures)
      5. grep "FINAL RESULTS:" -A 5 /tmp/test_formula_verify.log
      6. Assert: Shows "Passed: 9", "Failed: 0", "Total: 9"
    Expected Result: 0 failures, all parsing tests pass
    Failure Indicators: Any [FAIL] lines
    Evidence: .sisyphus/evidence/task-3-formula-verify.log

  Scenario: Verify test_formula_ref_verify passes
    Tool: Bash (test execution + grep)
    Preconditions: Build completed successfully
    Steps:
      1. ./build/test_formula_ref_verify 2>&1 | tee /tmp/test_formula_ref_verify.log
      2. Assert: Exit code = 0
      3. grep -c "^\[FAIL\]" /tmp/test_formula_ref_verify.log
      4. Assert: Result = 0 (zero failures)
      5. grep "FINAL RESULTS:" -A 5 /tmp/test_formula_ref_verify.log
      6. Assert: Shows "Passed: 68", "Failed: 0", "Total: 68"
    Expected Result: 0 failures, all formula tests pass
    Failure Indicators: Any [FAIL] lines
    Evidence: .sisyphus/evidence/task-3-formula-ref-verify.log

  Scenario: Verify all 7 tests pass via ctest
    Tool: Bash (ctest)
    Preconditions: All individual tests pass
    Steps:
      1. cd build && ctest --output-on-failure 2>&1 | tee /tmp/ctest.log
      2. Assert: Exit code = 0
      3. grep "tests passed" /tmp/ctest.log
      4. Assert: Shows "100% tests passed, 0 tests failed out of 7"
    Expected Result: 7/7 tests pass, 0% failure rate
    Failure Indicators: Any tests showing "Failed", less than 100% pass rate
    Evidence: .sisyphus/evidence/task-3-ctest-all.log

  Scenario: Create commit with all changes
    Tool: Bash (git)
    Preconditions: All 7 tests pass
    Steps:
      1. git status
      2. Assert: Only tests/ref/ref_cube.cpp modified
      3. git add tests/ref/ref_cube.cpp
      4. git commit -m "fix(tests): correct S move array size in reference implementation"
      5. git log -1 --stat
      6. Assert: Commit shows exactly 1 file changed, 2 insertions, 2 deletions
    Expected Result: Commit created with correct message, only ref_cube.cpp modified
    Failure Indicators: Multiple files staged, wrong commit message format
    Evidence: .sisyphus/evidence/task-3-commit.txt
  ```

  **Evidence to Capture**:
  - [ ] test_ref_verify output log
  - [ ] test_formula_verify output log
  - [ ] test_formula_ref_verify output log
  - [ ] ctest summary log
  - [ ] git commit log showing 1 file changed

  **Commit**: YES
  - Message: `fix(tests): correct S move array size in reference implementation`
  - Files: `tests/ref/ref_cube.cpp`
  - Pre-commit: `cd build && ctest` (all tests must pass)

---

## Final Verification Wave (MANDATORY — after ALL implementation tasks)

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Read the plan end-to-end. For each "Must Have": verify implementation exists (read file, check diff). For each "Must NOT Have": search codebase for forbidden patterns — reject with file:line if found. Check evidence files exist in .sisyphus/evidence/. Compare deliverables against plan.
  Output: `Must Have [N/N] | Must NOT Have [N/N] | Tasks [N/N] | VERDICT: APPROVE/REJECT`

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Run `cmake -S . -B build && make -C build`. Check the fixed file for: logic changes beyond array declarations, added comments, modified initialization patterns other than specified. Verify only lines 227 and 234 were modified.
  Output: `Build [PASS/FAIL] | Lines Changed [2/2] | Logic Preserved [YES/NO] | VERDICT`

- [ ] F3. **Test Verification** — `unspecified-high`
  Run all test executables: `./build/test_ref_verify`, `./build/test_formula_verify`, `./build/test_formula_ref_verify`, plus the 4 passing tests. Capture output. Parse for failure count. Verify S, S', Z, Z' specifically pass.
  Output: `Tests [7/7 PASS] | S/Z Moves [4/4 PASS] | Total Failures [0] | VERDICT`

- [ ] F4. **Scope Fidelity Check** — `deep`
  For each task: read "What to do", read actual diff (git diff). Verify 1:1 — only array declarations changed, nothing else. Check "Must NOT do" compliance. Detect any changes outside tests/ref/ref_cube.cpp.
  Output: `Tasks [N/N compliant] | File Scope [1 file modified] | Changes [2 lines] | VERDICT`

---

## Commit Strategy

- **1**: `fix(tests): correct S move array size in reference implementation`
  - Files: `tests/ref/ref_cube.cpp`
  - Pre-commit: `cd build && ctest`

---

## Success Criteria

### Verification Commands
```bash
# Rebuild
cmake -S . -B build && make -C build
# Expected: Build successful, no errors

# Run specific test
./build/test_ref_verify 2>&1 | grep -E "^\[FAIL\]" | wc -l
# Expected: 0

# Verify S/Z moves pass
./build/test_ref_verify 2>&1 | grep -E "^\[PASS\] (S|S'|Z|Z')"
# Expected: [PASS] S, [PASS] S', [PASS] Z, [PASS] Z'

# Run all tests
cd build && ctest --output-on-failure
# Expected: 100% tests passed, 0 tests failed out of 7
```

### Final Checklist
- [ ] All "Must Have" present (2 array declarations fixed)
- [ ] All "Must NOT Have" absent (no src/ modifications, no logic changes)
- [ ] All tests pass (7/7)
- [ ] S, S', Z, Z' specifically verified passing
