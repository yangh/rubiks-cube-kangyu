
## Rubik's Cube Double Move Implementation Research
**Date**: 2026-03-05

### Notation Standard

**Official Singmaster Notation**:
- **U2, D2, L2, R2, F2, B2** denote 180-degree (double) rotations
- Single letter (e.g., `U`) = 90° clockwise rotation
- Letter with prime (e.g., `U'`) = 90° counter-clockwise rotation
- Letter with 2 (e.g., `U2`) = 180° double turn

**Source**: Ruwix, Speedsolving Wiki, Cubelelo, OnyxCubes - all confirm this is the standard notation

### Implementation Examples from GitHub

#### Example 1: benbotto/rubiks-cube-cracker
**Repository**: https://github.com/benbotto/rubiks-cube-cracker

**Move Enum Definition**:
```cpp
enum class MOVE {
  U, UPRIME, U2,
  D, DPRIME, D2,
  L, LPRIME, L2,
  R, RPRIME, R2,
  F, FPRIME, F2,
  B, BPRIME, B2
};
```

**Implementation Pattern**:
```cpp
// From RubiksCube.cpp
void RubiksCube::rotate(MOVE move) {
  switch (move) {
    case MOVE::U2:
      return this->u2();
    // ... other cases
  }
}

// u2() is a dedicated method
void RubiksCube::u2() {
  // Perform U move twice OR
  // Directly implement 180° rotation
  this->u();
  this->u();
}
```

**Key Insight**: Double moves are treated as **separate move types** in the enum, not modifiers. This allows:
- Better type safety
- Independent optimization of double moves
- Easier scramble generation
- Cleaner move history tracking

### Common Implementation Patterns

#### Pattern 1: Enum-based (Most Common)
```cpp
enum Move {
  U, U_PRIME, U2,
  D, D_PRIME, D2,
  // ...
};

// Either:
void u2() {
  u();
  u(); // Apply U twice
}

// OR (optimized):
void u2() {
  // Direct 180° rotation logic
  // More efficient than calling u() twice
}
```

#### Pattern 2: Modifier-based (Less Common)
```cpp
struct Move {
  Face face;      // U, D, L, R, F, B
  int rotation;   // 90, -90, 180
  // OR
  RotationType type; // CLOCKWISE, COUNTER_CLOCKWISE, DOUBLE
};
```

### Animation Patterns for Double Moves

#### Approach 1: Double Duration
```cpp
// Animate 180° rotation in same time as two 90° rotations
float getAnimationDuration(Move move) {
  if (isDoubleMove(move)) {
    return BASE_ANIMATION_DURATION * 2.0f;
  }
  return BASE_ANIMATION_DURATION;
}
```

**Pros**: Simple, natural timing
**Cons**: Slower overall animation

#### Approach 2: Same Duration
```cpp
// Animate 180° rotation in same time as 90° rotation
float getAnimationDuration(Move move) {
  return BASE_ANIMATION_DURATION; // Same for all moves
}
```

**Pros**: Faster, more dynamic
**Cons**: May look rushed, less natural

#### Approach 3: Intermediate (Recommended)
```cpp
// Animate 180° rotation in 1.5x the time
float getAnimationDuration(Move move) {
  if (isDoubleMove(move)) {
    return BASE_ANIMATION_DURATION * 1.5f;
  }
  return BASE_ANIMATION_DURATION;
}
```

**Pros**: Balanced - faster than double time, slower than same time
**Cons**: Custom timing needs tuning

### Implementation from Real Projects

#### Pattern: Rotate Function with Angle Parameter
```cpp
void rotateFace(Face face, float angle) {
  // angle can be 90.0f, -90.0f, or 180.0f
  // Animate rotation over duration
  float duration = (abs(angle) == 180.0f) ? 
    BASE_DURATION * 1.5f : BASE_DURATION;
  
  animateRotation(face, angle, duration);
}
```

### Scramble Generation

Double moves are commonly included in scramble algorithms:

```cpp
vector<Move> generateScramble(int length) {
  vector<Move> scramble;
  vector<Move> allMoves = {
    U, U_PRIME, U2,
    D, D_PRIME, D2,
    L, L_PRIME, L2,
    R, R_PRIME, R2,
    F, F_PRIME, F2,
    B, B_PRIME, B2
  };
  
  for (int i = 0; i < length; i++) {
    scramble.push_back(randomChoice(allMoves));
  }
  
  return scramble;
}
```

### Performance Considerations

**Optimized u2() Implementation**:
```cpp
void RubiksCube::u2() {
  // Instead of calling u() twice, directly swap pieces
  // This is more efficient than two separate rotations
  
  // Swap opposite edges
  swap(edges[0], edges[2]);
  swap(edges[1], edges[3]);
  
  // Rotate corners 180°
  swap(corners[0], corners[2]);
  swap(corners[1], corners[3]);
  
  // Rotate face 180°
  rotateFace180(UP);
}
```

**Why Optimize?**
- Avoids redundant calculations
- Better cache locality
- Faster execution for solvers
- Cleaner move history

### Summary of Findings

1. **Notation**: U2/D2/L2/R2/F2/B2 is the standard
2. **Implementation**: Separate enum values for double moves
3. **Animation**: 1.5x duration recommended for double moves
4. **Optimization**: Direct 180° rotation more efficient than two 90° rotations
5. **Scrambles**: Double moves included in standard scrambles
6. **Type Safety**: Enum-based approach provides better compile-time checking

