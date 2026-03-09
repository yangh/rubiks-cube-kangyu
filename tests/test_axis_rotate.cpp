#include "../src/cube.h"
#include <iostream>
#include <string>

// Test helper
int testsPassed = 0;
int testsFailed = 0;

void test(bool condition, const std::string& testName) {
    if (condition) {
        std::cout << "[PASS] " << testName << std::endl;
        testsPassed++;
    } else {
        std::cout << "[FAIL] " << testName << std::endl;
        testsFailed++;
    }
}

int main() {
    std::cout << "=== Testing X/Y/Z Axis Rotations ===" << std::endl << std::endl;

    // Test 1: X then X' should restore the cube
    {
        RubiksCube cube;
        cube.executeMove(Move::X);
        cube.executeMove(Move::XP);
        test(cube.isSolved(), "X then X' restores solved state");
    }

    // Test 2: X2 twice should restore
    {
        RubiksCube cube;
        cube.executeMove(Move::X2);
        cube.executeMove(Move::X2);
        test(cube.isSolved(), "X2 twice restores solved state");
    }

    // Test 3: Y then Y' should restore
    {
        RubiksCube cube;
        cube.executeMove(Move::Y);
        cube.executeMove(Move::YP);
        test(cube.isSolved(), "Y then Y' restores solved state");
    }

    // Test 4: Y2 twice should restore
    {
        RubiksCube cube;
        cube.executeMove(Move::Y2);
        cube.executeMove(Move::Y2);
        test(cube.isSolved(), "Y2 twice restores solved state");
    }

    // Test 5: Z then Z' should restore
    {
        RubiksCube cube;
        cube.executeMove(Move::Z);
        cube.executeMove(Move::ZP);
        test(cube.isSolved(), "Z then Z' restores solved state");
    }

    // Test 6: Z2 twice should restore
    {
        RubiksCube cube;
        cube.executeMove(Move::Z2);
        cube.executeMove(Move::Z2);
        test(cube.isSolved(), "Z2 twice restores solved state");
    }

    // Test 7: Complex sequence should restore
    {
        RubiksCube cube;
        cube.executeMove(Move::X);
        cube.executeMove(Move::Y);
        cube.executeMove(Move::Z);
        cube.executeMove(Move::ZP);
        cube.executeMove(Move::YP);
        cube.executeMove(Move::XP);
        test(cube.isSolved(), "Complex sequence X Y Z Z' Y' X' restores solved state");
    }

    // Test 8: X rotation changes scrambled cube
    {
        RubiksCube cube;
        cube.executeMove(Move::R);  // Scramble first
        std::array<Color, 9> frontBefore = cube.getFront();
        cube.executeMove(Move::X);
        std::array<Color, 9> frontAfter = cube.getFront();
        
        bool changed = false;
        for (int i = 0; i < 9; ++i) {
            if (frontBefore[i] != frontAfter[i]) {
                changed = true;
                break;
            }
        }
        test(changed, "X rotation changes scrambled cube state");
    }

    // Test 9: Y rotation changes scrambled cube
    {
        RubiksCube cube;
        cube.executeMove(Move::R);  // Scramble first
        std::array<Color, 9> frontBefore = cube.getFront();
        cube.executeMove(Move::Y);
        std::array<Color, 9> frontAfter = cube.getFront();
        
        bool changed = false;
        for (int i = 0; i < 9; ++i) {
            if (frontBefore[i] != frontAfter[i]) {
                changed = true;
                break;
            }
        }
        test(changed, "Y rotation changes scrambled cube state");
    }

    // Test 10: Z rotation changes scrambled cube
    {
        RubiksCube cube;
        cube.executeMove(Move::R);  // Scramble first
        std::array<Color, 9> frontBefore = cube.getFront();
        cube.executeMove(Move::Z);
        std::array<Color, 9> frontAfter = cube.getFront();
        
        bool changed = false;
        for (int i = 0; i < 9; ++i) {
            if (frontBefore[i] != frontAfter[i]) {
                changed = true;
                break;
            }
        }
        test(changed, "Z rotation changes scrambled cube state");
    }

    // Test 11: Parse move strings
    {
        Move m;
        test(parseMoveString("x", m) && m == Move::X, "Parse 'x' as Move::X");
        test(parseMoveString("X", m) && m == Move::X, "Parse 'X' as Move::X");
        test(parseMoveString("x'", m) && m == Move::XP, "Parse 'x'' as Move::XP");
        test(parseMoveString("X'", m) && m == Move::XP, "Parse 'X'' as Move::XP");
        test(parseMoveString("x2", m) && m == Move::X2, "Parse 'x2' as Move::X2");
        test(parseMoveString("X2", m) && m == Move::X2, "Parse 'X2' as Move::X2");
        test(parseMoveString("y", m) && m == Move::Y, "Parse 'y' as Move::Y");
        test(parseMoveString("y'", m) && m == Move::YP, "Parse 'y'' as Move::YP");
        test(parseMoveString("y2", m) && m == Move::Y2, "Parse 'y2' as Move::Y2");
        test(parseMoveString("z", m) && m == Move::Z, "Parse 'z' as Move::Z");
        test(parseMoveString("z'", m) && m == Move::ZP, "Parse 'z'' as Move::ZP");
        test(parseMoveString("z2", m) && m == Move::Z2, "Parse 'z2' as Move::Z2");
    }

    // Test 12: moveToString
    {
        test(moveToString(Move::X) == "X", "Move::X to string");
        test(moveToString(Move::XP) == "X'", "Move::XP to string");
        test(moveToString(Move::X2) == "X2", "Move::X2 to string");
        test(moveToString(Move::Y) == "Y", "Move::Y to string");
        test(moveToString(Move::YP) == "Y'", "Move::YP to string");
        test(moveToString(Move::Y2) == "Y2", "Move::Y2 to string");
        test(moveToString(Move::Z) == "Z", "Move::Z to string");
        test(moveToString(Move::ZP) == "Z'", "Move::ZP to string");
        test(moveToString(Move::Z2) == "Z2", "Move::Z2 to string");
    }

    // Test 13: getInverseMove
    {
        test(getInverseMove(Move::X) == Move::XP, "Inverse of X is XP");
        test(getInverseMove(Move::XP) == Move::X, "Inverse of XP is X");
        test(getInverseMove(Move::X2) == Move::X2, "Inverse of X2 is X2");
        test(getInverseMove(Move::Y) == Move::YP, "Inverse of Y is YP");
        test(getInverseMove(Move::YP) == Move::Y, "Inverse of YP is Y");
        test(getInverseMove(Move::Y2) == Move::Y2, "Inverse of Y2 is Y2");
        test(getInverseMove(Move::Z) == Move::ZP, "Inverse of Z is ZP");
        test(getInverseMove(Move::ZP) == Move::Z, "Inverse of ZP is Z");
        test(getInverseMove(Move::Z2) == Move::Z2, "Inverse of Z2 is Z2");
    }

    // Test 14: Long sequence with axis rotations
    {
        RubiksCube cube;
        cube.executeMove(Move::R);
        cube.executeMove(Move::U);
        cube.executeMove(Move::X);
        cube.executeMove(Move::Y);
        cube.executeMove(Move::YP);
        cube.executeMove(Move::XP);
        cube.executeMove(Move::UP);
        cube.executeMove(Move::RP);
        test(cube.isSolved(), "Long sequence R U X Y Y' X' U' R' restores solved state");
    }

    // Summary
    std::cout << std::endl;
    std::cout << "=== Test Summary ===" << std::endl;
    std::cout << "Passed: " << testsPassed << std::endl;
    std::cout << "Failed: " << testsFailed << std::endl;
    
    return testsFailed > 0 ? 1 : 0;
}
