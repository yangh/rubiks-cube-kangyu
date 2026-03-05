# Learnings: Double Move Support

## Timestamp: 2026-03-05T15:26:00Z

### Codebase Structure
- **Move enum**: Defined in src/cube.h:19-38
- **Current moves**: 18 values (9 pairs of move/prime)
- **Pattern**: Move::X (clockwise), Move::XP (counter-clockwise)

### Update Impact Analysis
- **Core files**: 3 files (cube.h, cube.cpp, renderer.cpp, main.cpp)
- **Switch locations**: 15 distinct switch statements
- **Test files**: 6 files need updates
- **Per new move pair**: ~30-40 code locations

### Animation System
- **Hardcoded angles**: 2 locations in renderer.cpp (lines 319, 542)
- **Current angle**: 90° for all moves
- **Easing**: Quadratic ease-in-out formula
- **Duration**: 0.2 seconds (adjustable via speed multiplier)

### Test Infrastructure
- **Current tests**: 15 test groups in test_cube.cpp
- **Test pattern**: Uses assertTest() helper
- **Insertion point**: After line 741, call in main() at line 761

### Implementation Strategy
For double moves (U2, D2, etc.):
1. Add 9 new enum values (NO prime variants - double moves are self-inverse)
2. Update 15+ switch statements
3. Make animation angle configurable (90° vs 180°)
4. Add parser support for "X2" notation
5. Add tests following existing pattern

### Key Patterns
- **Inverse logic**: Double moves are self-inverse (U2' = U2)
- **Execution**: Call rotation function twice OR implement direct 180°
- **Animation**: Single 180° animation (not two 90° animations)
- **Parsing**: "X2" notation (uppercase, no prime suffix)

## Timestamp: 2026-03-05T15:30:00Z

### Task Completed: Move Enum Extension
- **File modified**: src/cube.h
- **Location**: After line 37 (SP), now lines 38-47
- **Added**: 9 new enum values (U2, D2, L2, R2, F2, B2, M2, E2, S2)
- **Total enum values**: 27 (18 existing + 9 new)
- **Comment added**: "// Double moves (180° rotation)"
- **Compilation**: Successful - all targets built without errors
- **Pattern followed**: Same inline comment style as existing values

### Enum Structure Now
```cpp
enum class Move {
    // Face moves (18 existing values)
    U, UP, D, DP, L, LP, R, RP, F, FP, B, BP,
    // Slice moves
    M, MP, E, EP, S, SP,
    // Double moves (9 new values)
    U2, D2, L2, R2, F2, B2, M2, E2, S2
};
```

### Next Steps (from plan)
- Wave 1: Continue with switch statement updates
- Need to handle all 15+ switch locations
- Animation angle configuration needed
