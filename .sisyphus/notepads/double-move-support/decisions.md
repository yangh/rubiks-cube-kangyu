# Decisions: Double Move Support

## Timestamp: 2026-03-05T15:26:00Z

### Technical Decisions

#### 1. Execution Strategy
**Decision**: Execute rotation function twice internally  
**Rationale**: 
- Simplest implementation
- Reuses existing rotation logic
- No need to implement 180° rotation algorithms
**Trade-off**: Slightly less efficient but much simpler

#### 2. Animation Approach
**Decision**: Single 180° animation with configurable angle  
**Rationale**:
- Better UX (smoother, faster)
- Matches user expectation ("double angle")
- Requires modifying animation system to support angle parameter
**Implementation**: 
- Add `float rotationAngle_ = 90.0f;` to renderer
- Pass 180.0f for double moves

#### 3. Move Representation
**Decision**: Add 9 new enum values (U2, D2, L2, R2, F2, B2, M2, E2, S2)  
**Rationale**:
- No prime variants needed (self-inverse)
- Cleaner than executing U twice in parser
- Enables proper history tracking
**Total enum values**: 27 (18 existing + 9 new)

#### 4. Parser Notation
**Decision**: Support "X2" notation (uppercase, no prime)  
**Rationale**:
- Standard Rubik's cube notation
- No "X2'" support (double moves don't have prime)
- Case-sensitive (uppercase only)
**Examples**: "U2", "R2", "F2", "M2", "E2", "S2"

#### 5. History Format
**Decision**: Store as single U2 entry  
**Rationale**:
- Cleaner notation
- Matches user input
- Enables proper undo/redo (single action)
**Alternative rejected**: Store as [U, U] (messier, breaks undo)

#### 6. Scramble Inclusion
**Decision**: Include double moves in scramble  
**Rationale**:
- Completeness
- More varied scrambles
- Minimal added complexity
**Implementation**: Add to basicMoves array (12 → 21 moves)

### Design Trade-offs

#### Simplicity vs Efficiency
- **Chosen**: Simplicity (execute twice)
- **Reason**: Code maintainability > micro-optimization

#### UX vs Implementation Effort  
- **Chosen**: Better UX (single 180° animation)
- **Reason**: User experience matters more

#### Notation Completeness vs Complexity
- **Chosen**: Standard notation only (no "X2'")
- **Reason**: Follows Rubik's cube standards
