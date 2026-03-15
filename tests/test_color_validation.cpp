#include "../src/cube.h"
#include <iostream>
#include <cassert>
#include <string>
#include <random>
#include <vector>

const std::string RESET = "\033[0m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";

int testsPassed = 0;
int testsFailed = 0;

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

void testOppositeColors() {
    std::cout << "\n=== Test: Opposite Color Detection ===" << std::endl;

    assertTest("WHITE is opposite to YELLOW", isOppositeColor(Color::WHITE, Color::YELLOW));
    assertTest("YELLOW is opposite to WHITE", isOppositeColor(Color::YELLOW, Color::WHITE));
    assertTest("RED is opposite to ORANGE", isOppositeColor(Color::RED, Color::ORANGE));
    assertTest("ORANGE is opposite to RED", isOppositeColor(Color::ORANGE, Color::RED));
    assertTest("GREEN is opposite to BLUE", isOppositeColor(Color::GREEN, Color::BLUE));
    assertTest("BLUE is opposite to GREEN", isOppositeColor(Color::BLUE, Color::GREEN));

    assertTest("WHITE is NOT opposite to RED", !isOppositeColor(Color::WHITE, Color::RED));
    assertTest("WHITE is NOT opposite to GREEN", !isOppositeColor(Color::WHITE, Color::GREEN));
    assertTest("WHITE is NOT opposite to WHITE", !isOppositeColor(Color::WHITE, Color::WHITE));
    assertTest("RED is NOT opposite to BLUE", !isOppositeColor(Color::RED, Color::BLUE));
}

void testInitialCubeIsValid() {
    std::cout << "\n=== Test: Initial Cube Validation ===" << std::endl;

    RubiksCube cube;
    assertTest("Initial cube has valid color configuration", cube.isValidColorConfiguration());
    assertTest("Initial cube validation error is empty", cube.getValidationError().empty());
}

void testValidationAfterSingleMoves() {
    std::cout << "\n=== Test: Validation After Single Moves ===" << std::endl;

    Move moves[] = {
        Move::U, Move::UP, Move::D, Move::DP,
        Move::L, Move::LP, Move::R, Move::RP,
        Move::F, Move::FP, Move::B, Move::BP,
        Move::M, Move::MP, Move::E, Move::EP, Move::S, Move::SP,
        Move::X, Move::XP, Move::Y, Move::YP, Move::Z, Move::ZP
    };

    for (Move move : moves) {
        RubiksCube cube;
        cube.executeMove(move);
        std::string err = cube.getValidationError();
        assertTest("After " + moveToString(move) + " cube is valid", 
                   cube.isValidColorConfiguration(),
                   err.empty() ? "" : err);
    }
}

void testValidationAfterDoubleMoves() {
    std::cout << "\n=== Test: Validation After Double Moves ===" << std::endl;

    Move moves[] = {
        Move::U2, Move::D2, Move::L2, Move::R2,
        Move::F2, Move::B2, Move::M2, Move::E2, Move::S2,
        Move::X2, Move::Y2, Move::Z2
    };

    for (Move move : moves) {
        RubiksCube cube;
        cube.executeMove(move);
        std::string err = cube.getValidationError();
        assertTest("After " + moveToString(move) + " cube is valid",
                   cube.isValidColorConfiguration(),
                   err.empty() ? "" : err);
    }
}

void testScramble20WithValidation() {
    std::cout << "\n=== Test: Scramble 20 Steps with Per-Step Validation ===" << std::endl;

    static const Move basicMoves[] = {
        Move::U, Move::UP, Move::D, Move::DP,
        Move::L, Move::LP, Move::R, Move::RP,
        Move::F, Move::FP, Move::B, Move::BP,
        Move::M, Move::MP, Move::E, Move::EP, Move::S, Move::SP
    };
    constexpr int numBasicMoves = sizeof(basicMoves) / sizeof(Move);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, numBasicMoves - 1);

    RubiksCube cube;
    bool allValid = true;
    std::string lastError;
    std::vector<Move> moves;

    for (int i = 0; i < 20; ++i) {
        Move move = basicMoves[dis(gen)];
        moves.push_back(move);
        cube.executeMove(move);

        if (!cube.isValidColorConfiguration()) {
            allValid = false;
            lastError = "Step " + std::to_string(i + 1) + " (" + moveToString(move) + "): " + cube.getValidationError();
            break;
        }
    }

    std::cout << CYAN << "Scramble sequence: ";
    for (size_t i = 0; i < moves.size(); ++i) {
        std::cout << moveToString(moves[i]);
        if (i < moves.size() - 1) std::cout << " ";
    }
    std::cout << RESET << std::endl;

    assertTest("All 20 scramble steps produced valid cube state", allValid, lastError);
}

void testMultipleScrambles() {
    std::cout << "\n=== Test: Multiple Scrambles (5 runs) ===" << std::endl;

    for (int run = 0; run < 5; ++run) {
        RubiksCube cube;
        std::vector<Move> scrambleMoves = cube.scramble(20);
        
        bool valid = cube.isValidColorConfiguration();
        std::string err = cube.getValidationError();
        
        assertTest("Scramble run " + std::to_string(run + 1) + " is valid", valid,
                   err.empty() ? "" : err);
    }
}

void testValidationAfterInverseSequences() {
    std::cout << "\n=== Test: Validation After Inverse Sequences ===" << std::endl;

    auto testSequence = [](const std::string& name, const std::vector<Move>& moves) {
        RubiksCube cube;
        for (Move m : moves) {
            cube.executeMove(m);
            if (!cube.isValidColorConfiguration()) {
                assertTest(name + " - valid at each step", false, 
                           "After " + moveToString(m) + ": " + cube.getValidationError());
                return;
            }
        }
        assertTest(name + " - valid at each step", true);
    };

    testSequence("U U' (inverse pair)", {Move::U, Move::UP});
    testSequence("R R' (inverse pair)", {Move::R, Move::RP});
    testSequence("F F' (inverse pair)", {Move::F, Move::FP});
    testSequence("M M' (inverse pair)", {Move::M, Move::MP});
    testSequence("E E' (inverse pair)", {Move::E, Move::EP});
    testSequence("S S' (inverse pair)", {Move::S, Move::SP});

    testSequence("U U U U (4x same)", {Move::U, Move::U, Move::U, Move::U});
    testSequence("R R R R (4x same)", {Move::R, Move::R, Move::R, Move::R});
    testSequence("F F F F (4x same)", {Move::F, Move::F, Move::F, Move::F});

    testSequence("R U R' U' (sexy move)", {Move::R, Move::U, Move::RP, Move::UP});
    testSequence("R U R' U' x6", {
        Move::R, Move::U, Move::RP, Move::UP,
        Move::R, Move::U, Move::RP, Move::UP,
        Move::R, Move::U, Move::RP, Move::UP,
        Move::R, Move::U, Move::RP, Move::UP,
        Move::R, Move::U, Move::RP, Move::UP,
        Move::R, Move::U, Move::RP, Move::UP
    });
}

void testComplexSequences() {
    std::cout << "\n=== Test: Complex Move Sequences ===" << std::endl;

    auto testSequence = [](const std::string& name, const std::vector<Move>& moves) {
        RubiksCube cube;
        for (Move m : moves) {
            cube.executeMove(m);
            if (!cube.isValidColorConfiguration()) {
                assertTest(name, false, "After " + moveToString(m) + ": " + cube.getValidationError());
                return;
            }
        }
        assertTest(name, true);
    };

    testSequence("Superflip (partial)", {
        Move::U, Move::RP, Move::F2, Move::D2, Move::LP,
        Move::D2, Move::F2, Move::RP, Move::U
    });

    testSequence("T-perm", {
        Move::R, Move::U, Move::RP, Move::UP,
        Move::RP, Move::F, Move::R2, Move::UP,
        Move::RP, Move::UP, Move::R, Move::U, Move::RP, Move::FP
    });

    testSequence("Sune algorithm", {
        Move::R, Move::U, Move::RP, Move::U, Move::R, Move::U2, Move::RP
    });

    testSequence("With slice moves", {
        Move::M, Move::S, Move::E, Move::MP, Move::SP, Move::EP,
        Move::U, Move::R, Move::F, Move::D, Move::L, Move::B
    });
}

void testValidationErrorMessage() {
    std::cout << "\n=== Test: Validation Error Messages ===" << std::endl;

    RubiksCube cube;
    assertTest("Valid cube has empty error message", cube.getValidationError() == "");

    cube.executeMove(Move::U);
    assertTest("After valid move, error still empty", cube.getValidationError() == "");

    cube.executeMove(Move::UP);
    assertTest("After returning to solved, error still empty", cube.getValidationError() == "");
}

int main() {
    std::cout << YELLOW << "Rubik's Cube Color Validation Tests" << RESET << std::endl;
    std::cout << "========================================" << std::endl;

    testOppositeColors();
    testInitialCubeIsValid();
    testValidationAfterSingleMoves();
    testValidationAfterDoubleMoves();
    testScramble20WithValidation();
    testMultipleScrambles();
    testValidationAfterInverseSequences();
    testComplexSequences();
    testValidationErrorMessage();

    std::cout << "\n========================================" << std::endl;
    std::cout << YELLOW << "Test Results:" << RESET << std::endl;
    std::cout << GREEN << "Passed: " << testsPassed << RESET << std::endl;
    std::cout << "Failed: " << testsFailed << RESET << std::endl;
    std::cout << "Total:  " << (testsPassed + testsFailed) << std::endl;
    std::cout << "========================================" << std::endl;

    return (testsFailed == 0) ? 0 : 1;
}
