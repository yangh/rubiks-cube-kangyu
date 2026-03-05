# Issues: Double Move Support

## Timestamp: 2026-03-05T15:26:00Z

### Known Issues

#### 1. Lying Comment in cube.h
**Location**: src/cube.h:46  
**Issue**: Comment claims parseMoveString handles "F2" but it doesn't  
**Impact**: Could mislead developers  
**Fix**: Update comment to remove "F2" claim (Task 1)

#### 2. Hardcoded Animation Angle
**Location**: src/renderer.cpp:319, 542  
**Issue**: 90° angle hardcoded, not configurable  
**Impact**: Can't animate 180° rotations  
**Fix**: Make angle a member variable (Task 8)

#### 3. Duplicate Inverse Logic
**Location**: 4 locations (main.cpp:87, cube.cpp:254, 307, test files)  
**Issue**: Same inverse mapping repeated 4+ times  
**Impact**: Maintenance burden, risk of inconsistency  
**Fix**: Keep duplicative for now (matches existing pattern)

### Potential Gotchas

#### 1. Animation Angle Scope
**Risk**: Forgetting to set angle for double moves  
**Mitigation**: Add angle parameter to animateMove() with default 90°

#### 2. Scramble Distribution
**Risk**: Double moves might over-represent in scrambles  
**Mitigation**: Monitor, adjust if needed (21 total moves is fine)

#### 3. Test Coverage
**Risk**: Missing edge cases in tests  
**Mitigation**: Follow existing test patterns (inverse, four-times, equivalence)

### Technical Debt

#### 1. No Centralized Inverse Mapping
**Current**: 4 locations with duplicate switch statements  
**Better**: Single getInverseMove() function  
**Decision**: Keep as-is to match existing architecture

#### 2. Animation State Not Encapsulated
**Current**: Individual member variables in CubeRenderer  
**Better**: AnimationState struct  
**Decision**: Keep as-is, add rotationAngle_ member

#### 3. No Angle Validation
**Current**: Any float value accepted  
**Better**: Validate angle is 90° or 180°  
**Decision**: Accept any value for flexibility
