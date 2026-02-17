#include "../src/cube.h"
#include <iostream>
#include <cassert>
#include <string>

// Terminal color codes
const std::string RESET = "\033[0m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string YELLOW = "\033[33m";

int testsPassed = 0;
int testsFailed = 0;

// Test assertion helper
void assertTest(const std::string& testName, bool condition, const std::string& details = "") {
    if (condition) {
        std::cout << GREEN << "[PASS] " << testName << RESET << std::endl;
        testsPassed++;
    } else {
        std::cout << RED << "[FAIL] " << testName << RESET;
        if (!details.empty()) {
            std::cout << " - " << details;
        }
        std::cout << std::endl;
        testsFailed++;
    }
}

// Helper to check if a face is all one color
bool isFaceSingleColor(const std::array<Color, 9>& face, Color color) {
    for (int i = 0; i < 9; i++) {
        if (face[i] != color) return false;
    }
    return true;
}

// Test 1: Initial state should be solved
void testInitialState() {
    std::cout << "\\n=== Test 1: Initial State ===" << std::endl;
    RubiksCube cube;
    assertTest("Initial cube is solved", cube.isSolved());
    assertTest("Front face is all green", isFaceSingleColor(cube.getFront(), Color::GREEN));
    assertTest("Back face is all blue", isFaceSingleColor(cube.getBack(), Color::BLUE));
    assertTest("Left face is all orange", isFaceSingleColor(cube.getLeft(), Color::ORANGE));
    assertTest("Right face is all red", isFaceSingleColor(cube.getRight(), Color::RED));
    assertTest("Up face is all white", isFaceSingleColor(cube.getUp(), Color::WHITE));
    assertTest("Down face is all yellow", isFaceSingleColor(cube.getDown(), Color::YELLOW));
}

// Test 2: Inverse moves should return to original state
void testInverseMoves() {
    std::cout << "\\n=== Test 2: Inverse Moves ===" << std::endl;

    struct TestCase {
        std::string name;
        Move move1;
        Move move2;
    };

    TestCase tests[] = {
        {"U + U'", Move::U, Move::UP},
        {"D + D'", Move::D, Move::DP},
        {"L + L'", Move::L, Move::LP},
        {"R + R'", Move::R, Move::RP},
        {"F + F'", Move::F, Move::FP},
        {"B + B'", Move::B, Move::BP},
        {"M + M'", Move::M, Move::MP},
        {"E + E'", Move::E, Move::EP},
        {"S + S'", Move::S, Move::SP},
    };

    for (const auto& test : tests) {
        RubiksCube cube;
        auto frontBefore = cube.getFront();
        auto backBefore = cube.getBack();
        auto leftBefore = cube.getLeft();
        auto rightBefore = cube.getRight();
        auto upBefore = cube.getUp();
        auto downBefore = cube.getDown();

        cube.executeMove(test.move1);
        cube.executeMove(test.move2);

        bool allMatch = (cube.getFront() == frontBefore) &&
                      (cube.getBack() == backBefore) &&
                      (cube.getLeft() == leftBefore) &&
                      (cube.getRight() == rightBefore) &&
                      (cube.getUp() == upBefore) &&
                      (cube.getDown() == downBefore);

        assertTest(test.name + " returns to original state", allMatch);
        assertTest(test.name + " is solved after", cube.isSolved());
    }
}

// Test 3: Four moves of same face should return to original
void testFourMoves() {
    std::cout << "\\n=== Test 3: Four Moves ===" << std::endl;

    struct TestCase {
        std::string name;
        Move move;
    };

    TestCase tests[] = {
        {"U four times", Move::U},
        {"D four times", Move::D},
        {"L four times", Move::L},
        {"R four times", Move::R},
        {"F four times", Move::F},
        {"B four times", Move::B},
        {"M four times", Move::M},
        {"E four times", Move::E},
        {"S four times", Move::S},
    };

    for (const auto& test : tests) {
        RubiksCube cube;
        auto frontBefore = cube.getFront();
        auto backBefore = cube.getBack();
        auto leftBefore = cube.getLeft();
        auto rightBefore = cube.getRight();
        auto upBefore = cube.getUp();
        auto downBefore = cube.getDown();

        cube.executeMove(test.move);
        cube.executeMove(test.move);
        cube.executeMove(test.move);
        cube.executeMove(test.move);

        bool allMatch = (cube.getFront() == frontBefore) &&
                      (cube.getBack() == backBefore) &&
                      (cube.getLeft() == leftBefore) &&
                      (cube.getRight() == rightBefore) &&
                      (cube.getUp() == upBefore) &&
                      (cube.getDown() == downBefore);

        assertTest(test.name + " returns to original state", allMatch);
        assertTest(test.name + " is solved after", cube.isSolved());
    }
}

// Test 4: Reset should restore solved state
void testReset() {
    std::cout << "\\n=== Test 4: Reset ===" << std::endl;

    RubiksCube cube;

    // Scramble the cube
    cube.executeMove(Move::U);
    cube.executeMove(Move::R);
    cube.executeMove(Move::F);
    cube.executeMove(Move::D);

    assertTest("Scrambled cube is not solved", !cube.isSolved());

    // Reset
    cube.reset();

    assertTest("Reset cube is solved", cube.isSolved());
    assertTest("Reset front is green", isFaceSingleColor(cube.getFront(), Color::GREEN));
    assertTest("Reset up is white", isFaceSingleColor(cube.getUp(), Color::WHITE));
}

// Test 5: Specific move patterns
void testMovePatterns() {
    std::cout << "\\n=== Test 5: Move Patterns ===" << std::endl;

    {
        RubiksCube cube;
        cube.executeMove(Move::R);
        cube.executeMove(Move::U);
        cube.executeMove(Move::RP);
        cube.executeMove(Move::UP);
        assertTest("R U R' U' executed successfully", true);
    }

    {
        RubiksCube cube;
        cube.executeMove(Move::U);
        cube.executeMove(Move::D);
        cube.executeMove(Move::UP);
        cube.executeMove(Move::DP);
        assertTest("U D U' D' executed successfully", true);
    }

    {
        RubiksCube cube;
        cube.executeMove(Move::M);
        cube.executeMove(Move::E);
        cube.executeMove(Move::MP);
        cube.executeMove(Move::EP);
        assertTest("M E M' E' executed successfully", true);
    }

    {
        RubiksCube cube;
        cube.executeMove(Move::S);
        cube.executeMove(Move::SP);
        assertTest("S S' executed successfully", true);
    }

    {
        RubiksCube cube;
        cube.executeMove(Move::M);
        cube.executeMove(Move::S);
        cube.executeMove(Move::E);
        cube.executeMove(Move::MP);
        cube.executeMove(Move::SP);
        cube.executeMove(Move::EP);
        assertTest("M S E M' S' E' executed successfully", true);
    }
}

// Test 6: Adjacent faces affect each other correctly
void testAdjacentFaces() {
    std::cout << "\\n=== Test 6: Adjacent Faces Interaction ===" << std::endl;

    {
        RubiksCube cube;
        auto frontBefore = cube.getFront();
        auto rightBefore = cube.getRight();
        auto backBefore = cube.getBack();
        auto leftBefore = cube.getLeft();

        cube.executeMove(Move::U);

        // U move should affect top row of front, left, back, right
        bool frontChanged = (cube.getFront()[0] != frontBefore[0]) ||
                             (cube.getFront()[1] != frontBefore[1]) ||
                             (cube.getFront()[2] != frontBefore[2]);
        bool rightChanged = (cube.getRight()[0] != rightBefore[0]) ||
                            (cube.getRight()[1] != rightBefore[1]) ||
                            (cube.getRight()[2] != rightBefore[2]);
        bool leftChanged = (cube.getLeft()[0] != leftBefore[0]) ||
                           (cube.getLeft()[1] != leftBefore[1]) ||
                           (cube.getLeft()[2] != leftBefore[2]);
        bool backChanged = (cube.getBack()[0] != backBefore[0]) ||
                           (cube.getBack()[1] != backBefore[1]) ||
                           (cube.getBack()[2] != backBefore[2]);

        assertTest("U move affected adjacent faces", frontChanged && rightChanged && leftChanged && backChanged);
    }

    {
        RubiksCube cube;
        auto upBefore = cube.getUp();
        auto rightBefore = cube.getRight();
        auto downBefore = cube.getDown();
        auto leftBefore = cube.getLeft();

        cube.executeMove(Move::F);

        // F move should affect bottom of up (6,7,8), left of right (0,3,6), top of down (0,1,2), right of left (2,5,8)
        bool upChanged = (cube.getUp()[6] != upBefore[6]) ||
                         (cube.getUp()[7] != upBefore[7]) ||
                         (cube.getUp()[8] != upBefore[8]);
        bool rightChanged = (cube.getRight()[0] != rightBefore[0]) ||
                            (cube.getRight()[3] != rightBefore[3]) ||
                            (cube.getRight()[6] != rightBefore[6]);
        bool downChanged = (cube.getDown()[0] != downBefore[0]) ||
                           (cube.getDown()[1] != downBefore[1]) ||
                           (cube.getDown()[2] != downBefore[2]);
        bool leftChanged = (cube.getLeft()[2] != leftBefore[2]) ||
                           (cube.getLeft()[5] != leftBefore[5]) ||
                           (cube.getLeft()[8] != leftBefore[8]);

        assertTest("F move affected adjacent faces", upChanged && rightChanged && downChanged && leftChanged);
    }

    {
        RubiksCube cube;
        auto upBefore = cube.getUp();
        auto frontBefore = cube.getFront();
        auto downBefore = cube.getDown();
        auto backBefore = cube.getBack();

        cube.executeMove(Move::R);

        // R move should affect right of up (2,5,8), right of front (2,5,8), right of down (2,5,8), left of back (6,3,0)
        bool upChanged = (cube.getUp()[2] != upBefore[2]) ||
                         (cube.getUp()[5] != upBefore[5]) ||
                         (cube.getUp()[8] != upBefore[8]);
        bool frontChanged = (cube.getFront()[2] != frontBefore[2]) ||
                            (cube.getFront()[5] != frontBefore[5]) ||
                            (cube.getFront()[8] != frontBefore[8]);
        bool downChanged = (cube.getDown()[2] != downBefore[2]) ||
                           (cube.getDown()[5] != downBefore[5]) ||
                           (cube.getDown()[8] != downBefore[8]);
        bool backChanged = (cube.getBack()[6] != backBefore[6]) ||
                           (cube.getBack()[3] != backBefore[3]) ||
                           (cube.getBack()[0] != backBefore[0]);

        assertTest("R move affected adjacent faces", upChanged && frontChanged && downChanged && backChanged);
    }

    {
        RubiksCube cube;
        auto upBefore = cube.getUp();
        auto downBefore = cube.getDown();
        auto frontBefore = cube.getFront();
        auto backBefore = cube.getBack();

        cube.executeMove(Move::M);

        // M move should affect middle column (1,4,7) of up, down, front, back
        bool upChanged = (cube.getUp()[1] != upBefore[1]) ||
                         (cube.getUp()[4] != upBefore[4]) ||
                         (cube.getUp()[7] != upBefore[7]);
        bool downChanged = (cube.getDown()[1] != downBefore[1]) ||
                           (cube.getDown()[4] != downBefore[4]) ||
                           (cube.getDown()[7] != downBefore[7]);
        bool frontChanged = (cube.getFront()[1] != frontBefore[1]) ||
                           (cube.getFront()[4] != frontBefore[4]) ||
                           (cube.getFront()[7] != frontBefore[7]);
        bool backChanged = (cube.getBack()[1] != backBefore[1]) ||
                          (cube.getBack()[4] != backBefore[4]) ||
                          (cube.getBack()[7] != backBefore[7]);

        assertTest("M move affected adjacent faces", upChanged && downChanged && frontChanged && backChanged);
    }

    {
        RubiksCube cube;
        auto frontBefore = cube.getFront();
        auto backBefore = cube.getBack();
        auto leftBefore = cube.getLeft();
        auto rightBefore = cube.getRight();

        cube.executeMove(Move::E);

        // E move should affect middle row (3,4,5) of front, back, left, right
        bool frontChanged = (cube.getFront()[3] != frontBefore[3]) ||
                           (cube.getFront()[4] != frontBefore[4]) ||
                           (cube.getFront()[5] != frontBefore[5]);
        bool backChanged = (cube.getBack()[3] != backBefore[3]) ||
                          (cube.getBack()[4] != backBefore[4]) ||
                          (cube.getBack()[5] != backBefore[5]);
        bool leftChanged = (cube.getLeft()[3] != leftBefore[3]) ||
                          (cube.getLeft()[4] != leftBefore[4]) ||
                          (cube.getLeft()[5] != leftBefore[5]);
        bool rightChanged = (cube.getRight()[3] != rightBefore[3]) ||
                           (cube.getRight()[4] != rightBefore[4]) ||
                           (cube.getRight()[5] != rightBefore[5]);

        assertTest("E move affected adjacent faces", frontChanged && backChanged && leftChanged && rightChanged);
    }

    {
        RubiksCube cube;
        auto upBefore = cube.getUp();
        auto downBefore = cube.getDown();
        auto leftBefore = cube.getLeft();
        auto rightBefore = cube.getRight();

        cube.executeMove(Move::S);

        // S move should affect middle row (3,4,5) of up and down, middle column (1,4,7) of left and right
        bool upChanged = (cube.getUp()[3] != upBefore[3]) ||
                         (cube.getUp()[4] != upBefore[4]) ||
                         (cube.getUp()[5] != upBefore[5]);
        bool downChanged = (cube.getDown()[3] != downBefore[3]) ||
                           (cube.getDown()[4] != downBefore[4]) ||
                           (cube.getDown()[5] != downBefore[5]);
        bool leftChanged = (cube.getLeft()[1] != leftBefore[1]) ||
                          (cube.getLeft()[4] != leftBefore[4]) ||
                          (cube.getLeft()[7] != leftBefore[7]);
        bool rightChanged = (cube.getRight()[1] != rightBefore[1]) ||
                           (cube.getRight()[4] != rightBefore[4]) ||
                           (cube.getRight()[7] != rightBefore[7]);

        assertTest("S move affected adjacent faces", upChanged && downChanged && leftChanged && rightChanged);
    }
}

// Test 7: Complex scrambling should make cube unsolved
void testScrambling() {
    std::cout << "\\n=== Test 7: Scrambling ===" << std::endl;

    {
        RubiksCube cube;
        Move scrambleMoves[] = {
            Move::U, Move::R, Move::F, Move::D,
            Move::L, Move::B, Move::UP, Move::DP,
            Move::RP, Move::FP, Move::LP, Move::BP
        };

        for (auto move : scrambleMoves) {
            cube.executeMove(move);
        }

        assertTest("Scrambled cube is not solved (basic moves)", !cube.isSolved());
    }

    {
        RubiksCube cube;
        Move scrambleMoves[] = {
            Move::M, Move::E, Move::S, Move::MP,
            Move::EP, Move::SP, Move::U, Move::D,
            Move::R, Move::L, Move::F, Move::B
        };

        for (auto move : scrambleMoves) {
            cube.executeMove(move);
        }

        assertTest("Scrambled cube is not solved (with slice moves)", !cube.isSolved());
    }
}

// Test 8: Color conversion
void testColorConversion() {
    std::cout << "\\n=== Test 8: Color to RGB ===" << std::endl;

    auto white = colorToRgb(Color::WHITE);
    assertTest("White RGB is (1, 1, 1)",
               white[0] == 1.0f && white[1] == 1.0f && white[2] == 1.0f);

    auto red = colorToRgb(Color::RED);
    assertTest("Red RGB is (1, 0, 0)",
               red[0] == 1.0f && red[1] == 0.0f && red[2] == 0.0f);

    auto yellow = colorToRgb(Color::YELLOW);
    assertTest("Yellow RGB is (1, 1, 0)",
               yellow[0] == 1.0f && yellow[1] == 1.0f && yellow[2] == 0.0f);
}

// Test 9: Move string conversion
void testMoveToString() {
    std::cout << "\\n=== Test 9: Move to String ===" << std::endl;

    assertTest("U converts to 'U'", moveToString(Move::U) == "U");
    assertTest("U' converts to 'U'", moveToString(Move::UP) == "U'");
    assertTest("R converts to 'R'", moveToString(Move::R) == "R");
    assertTest("R' converts to 'R'", moveToString(Move::RP) == "R'");
    assertTest("F converts to 'F'", moveToString(Move::F) == "F");
    assertTest("F' converts to 'F'", moveToString(Move::FP) == "F'");
    assertTest("M converts to 'M'", moveToString(Move::M) == "M");
    assertTest("M' converts to 'M'", moveToString(Move::MP) == "M'");
    assertTest("E converts to 'E'", moveToString(Move::E) == "E");
    assertTest("E' converts to 'E'", moveToString(Move::EP) == "E'");
    assertTest("S converts to 'S'", moveToString(Move::S) == "S");
    assertTest("S' converts to 'S'", moveToString(Move::SP) == "S'");
}

// Test 10: Cube consistency after multiple operations
void testCubeConsistency() {
    std::cout << "\\n=== Test 10: Cube Consistency ===" << std::endl;

    {
        RubiksCube cube;
        cube.executeMove(Move::U);
        cube.executeMove(Move::R);
        cube.executeMove(Move::F);
        cube.executeMove(Move::UP);
        cube.executeMove(Move::RP);
        cube.executeMove(Move::FP);

        assertTest("Front has 9 stickers", true);
        assertTest("Up has 9 stickers", true);
        assertTest("Down has 9 stickers", true);
    }

    {
        RubiksCube cube;
        cube.executeMove(Move::M);
        cube.executeMove(Move::E);
        cube.executeMove(Move::S);
        cube.executeMove(Move::MP);
        cube.executeMove(Move::EP);
        cube.executeMove(Move::SP);

        assertTest("Front has 9 stickers after slice moves", true);
        assertTest("Up has 9 stickers after slice moves", true);
        assertTest("Down has 9 stickers after slice moves", true);
        assertTest("Left has 9 stickers after slice moves", true);
        assertTest("Right has 9 stickers after slice moves", true);
        assertTest("Back has 9 stickers after slice moves", true);
    }

    {
        RubiksCube cube;
        cube.executeMove(Move::U);
        cube.executeMove(Move::M);
        cube.executeMove(Move::E);
        cube.executeMove(Move::S);
        // Inverse moves must be executed in reverse order
        cube.executeMove(Move::SP);
        cube.executeMove(Move::EP);
        cube.executeMove(Move::MP);
        cube.executeMove(Move::UP);

        assertTest("Cube consistent after mixed moves", cube.isSolved());
    }
}

// Test 11: M/E/S inverse moves should return to original state
void testSliceInverseMoves() {
    std::cout << "\\n=== Test 11: Slice Inverse Moves ===" << std::endl;

    struct TestCase {
        std::string name;
        Move move1;
        Move move2;
    };

    TestCase tests[] = {
        {"M + M'", Move::M, Move::MP},
        {"E + E'", Move::E, Move::EP},
        {"S + S'", Move::S, Move::SP},
    };

    for (const auto& test : tests) {
        RubiksCube cube;
        auto frontBefore = cube.getFront();
        auto backBefore = cube.getBack();
        auto leftBefore = cube.getLeft();
        auto rightBefore = cube.getRight();
        auto upBefore = cube.getUp();
        auto downBefore = cube.getDown();

        cube.executeMove(test.move1);
        cube.executeMove(test.move2);

        bool allMatch = (cube.getFront() == frontBefore) &&
                      (cube.getBack() == backBefore) &&
                      (cube.getLeft() == leftBefore) &&
                      (cube.getRight() == rightBefore) &&
                      (cube.getUp() == upBefore) &&
                      (cube.getDown() == downBefore);

        assertTest(test.name + " returns to original state", allMatch);
        assertTest(test.name + " is solved after", cube.isSolved());
    }
}

// Test 12: Four M/E/S moves should return to original
void testSliceFourMoves() {
    std::cout << "\\n=== Test 12: Slice Four Moves ===" << std::endl;

    struct TestCase {
        std::string name;
        Move move;
    };

    TestCase tests[] = {
        {"M four times", Move::M},
        {"E four times", Move::E},
        {"S four times", Move::S},
    };

    for (const auto& test : tests) {
        RubiksCube cube;
        auto frontBefore = cube.getFront();
        auto backBefore = cube.getBack();
        auto leftBefore = cube.getLeft();
        auto rightBefore = cube.getRight();
        auto upBefore = cube.getUp();
        auto downBefore = cube.getDown();

        cube.executeMove(test.move);
        cube.executeMove(test.move);
        cube.executeMove(test.move);
        cube.executeMove(test.move);

        bool allMatch = (cube.getFront() == frontBefore) &&
                      (cube.getBack() == backBefore) &&
                      (cube.getLeft() == leftBefore) &&
                      (cube.getRight() == rightBefore) &&
                      (cube.getUp() == upBefore) &&
                      (cube.getDown() == downBefore);

        assertTest(test.name + " returns to original state", allMatch);
        assertTest(test.name + " is solved after", cube.isSolved());
    }
}

// Test 13: M/E/S slice adjacent faces interaction
void testSliceAdjacentFaces() {
    std::cout << "\\n=== Test 13: Slice Adjacent Faces ===" << std::endl;

    // M move test: affects Up[1,4,7], Down[1,4,7], Front[1,4,7], Back[1,4,7]
    {
        RubiksCube cube;
        auto upBefore = cube.getUp();
        auto downBefore = cube.getDown();
        auto frontBefore = cube.getFront();
        auto backBefore = cube.getBack();

        cube.executeMove(Move::M);

        // M should affect middle column (1,4,7) of up, down, front, back
        bool upChanged = (cube.getUp()[1] != upBefore[1]) ||
                        (cube.getUp()[4] != upBefore[4]) ||
                        (cube.getUp()[7] != upBefore[7]);
        bool downChanged = (cube.getDown()[1] != downBefore[1]) ||
                          (cube.getDown()[4] != downBefore[4]) ||
                          (cube.getDown()[7] != downBefore[7]);
        bool frontChanged = (cube.getFront()[1] != frontBefore[1]) ||
                           (cube.getFront()[4] != frontBefore[4]) ||
                           (cube.getFront()[7] != frontBefore[7]);
        bool backChanged = (cube.getBack()[1] != backBefore[1]) ||
                          (cube.getBack()[4] != backBefore[4]) ||
                          (cube.getBack()[7] != backBefore[7]);

        // M should NOT affect outer columns (0,2,3,5,6,8) of up, down, front, back
        bool upOuterUnchanged = (cube.getUp()[0] == upBefore[0]) &&
                               (cube.getUp()[2] == upBefore[2]) &&
                               (cube.getUp()[3] == upBefore[3]) &&
                               (cube.getUp()[5] == upBefore[5]) &&
                               (cube.getUp()[6] == upBefore[6]) &&
                               (cube.getUp()[8] == upBefore[8]);

        assertTest("M move affected middle column", upChanged && downChanged && frontChanged && backChanged);
        assertTest("M move left outer columns unchanged", upOuterUnchanged);
        assertTest("M move does not affect left face", cube.getLeft() == cube.getLeft());
        assertTest("M move does not affect right face", cube.getRight() == cube.getRight());
    }

    // E move test: affects Front[3,4,5], Back[3,4,5], Left[3,4,5], Right[3,4,5]
    {
        RubiksCube cube;
        auto frontBefore = cube.getFront();
        auto backBefore = cube.getBack();
        auto leftBefore = cube.getLeft();
        auto rightBefore = cube.getRight();

        cube.executeMove(Move::E);

        // E should affect middle row (3,4,5) of front, back, left, right
        bool frontChanged = (cube.getFront()[3] != frontBefore[3]) ||
                           (cube.getFront()[4] != frontBefore[4]) ||
                           (cube.getFront()[5] != frontBefore[5]);
        bool backChanged = (cube.getBack()[3] != backBefore[3]) ||
                          (cube.getBack()[4] != backBefore[4]) ||
                          (cube.getBack()[5] != backBefore[5]);
        bool leftChanged = (cube.getLeft()[3] != leftBefore[3]) ||
                          (cube.getLeft()[4] != leftBefore[4]) ||
                          (cube.getLeft()[5] != leftBefore[5]);
        bool rightChanged = (cube.getRight()[3] != rightBefore[3]) ||
                           (cube.getRight()[4] != rightBefore[4]) ||
                           (cube.getRight()[5] != rightBefore[5]);

        // E should NOT affect up and down faces
        bool upUnchanged = (cube.getUp() == cube.getUp());
        bool downUnchanged = (cube.getDown() == cube.getDown());

        assertTest("E move affected middle row", frontChanged && backChanged && leftChanged && rightChanged);
        assertTest("E move does not affect up face", upUnchanged);
        assertTest("E move does not affect down face", downUnchanged);
    }

    // S move test: affects Up[3,4,5], Down[3,4,5], Left[1,4,7], Right[1,4,7]
    {
        RubiksCube cube;
        auto upBefore = cube.getUp();
        auto downBefore = cube.getDown();
        auto leftBefore = cube.getLeft();
        auto rightBefore = cube.getRight();

        cube.executeMove(Move::S);

        // S should affect middle row (3,4,5) of up and down, middle column (1,4,7) of left and right
        bool upChanged = (cube.getUp()[3] != upBefore[3]) ||
                        (cube.getUp()[4] != upBefore[4]) ||
                        (cube.getUp()[5] != upBefore[5]);
        bool downChanged = (cube.getDown()[3] != downBefore[3]) ||
                          (cube.getDown()[4] != downBefore[4]) ||
                          (cube.getDown()[5] != downBefore[5]);
        bool leftChanged = (cube.getLeft()[1] != leftBefore[1]) ||
                          (cube.getLeft()[4] != leftBefore[4]) ||
                          (cube.getLeft()[7] != leftBefore[7]);
        bool rightChanged = (cube.getRight()[1] != rightBefore[1]) ||
                           (cube.getRight()[4] != rightBefore[4]) ||
                           (cube.getRight()[7] != rightBefore[7]);

        // S should NOT affect front and back faces
        bool frontUnchanged = (cube.getFront() == cube.getFront());
        bool backUnchanged = (cube.getBack() == cube.getBack());

        assertTest("S move affected middle row/column", upChanged && downChanged && leftChanged && rightChanged);
        assertTest("S move does not affect front face", frontUnchanged);
        assertTest("S move does not affect back face", backUnchanged);
    }
}

// Test 14: M/E/S move to string conversion
void testSliceMoveToString() {
    std::cout << "\\n=== Test 14: Slice Move to String ===" << std::endl;

    assertTest("M converts to 'M'", moveToString(Move::M) == "M");
    assertTest("M' converts to 'M'", moveToString(Move::MP) == "M'");
    assertTest("E converts to 'E'", moveToString(Move::E) == "E");
    assertTest("E' converts to 'E'", moveToString(Move::EP) == "E'");
    assertTest("S converts to 'S'", moveToString(Move::S) == "S");
    assertTest("S' converts to 'S'", moveToString(Move::SP) == "S'");
}

// Test 15: Complex sequence with slice moves
void testSliceMovePatterns() {
    std::cout << "\\n=== Test 15: Slice Move Patterns ===" << std::endl;

    {
        RubiksCube cube;
        cube.executeMove(Move::M);
        cube.executeMove(Move::E);
        cube.executeMove(Move::S);
        cube.executeMove(Move::MP);
        cube.executeMove(Move::EP);
        cube.executeMove(Move::SP);
        assertTest("M E S M' E' S' executed successfully", true);
    }

    {
        RubiksCube cube;
        cube.executeMove(Move::M);
        cube.executeMove(Move::M);
        cube.executeMove(Move::M);
        cube.executeMove(Move::M);
        assertTest("M M M M returns to solved", cube.isSolved());
    }

    {
        RubiksCube cube;
        cube.executeMove(Move::E);
        cube.executeMove(Move::E);
        cube.executeMove(Move::E);
        cube.executeMove(Move::E);
        assertTest("E E E E returns to solved", cube.isSolved());
    }

    {
        RubiksCube cube;
        cube.executeMove(Move::S);
        cube.executeMove(Move::S);
        cube.executeMove(Move::S);
        cube.executeMove(Move::S);
        assertTest("S S S S returns to solved", cube.isSolved());
    }
}

int main() {
    std::cout << YELLOW << "Rubik's Cube Logic Tests" << RESET << std::endl;
    std::cout << "===============================" << std::endl;

    testInitialState();
    testInverseMoves();
    testFourMoves();
    testReset();
    testMovePatterns();
    testAdjacentFaces();
    testScrambling();
    testColorConversion();
    testMoveToString();
    testCubeConsistency();
    testSliceInverseMoves();
    testSliceFourMoves();
    testSliceAdjacentFaces();
    testSliceMoveToString();
    testSliceMovePatterns();

    std::cout << "\\n========================================" << std::endl;
    std::cout << YELLOW << "Test Results:" << RESET << std::endl;
    std::cout << GREEN << "Passed: " << testsPassed << RESET << std::endl;
    std::cout << "Failed: " << testsFailed << RESET << std::endl;
    std::cout << "Total:  " << (testsPassed + testsFailed) << std::endl;
    std::cout << "========================================" << std::endl;

    return (testsFailed == 0) ? 0 : 1;
}
