#include "../src/cube.h"
#include <iostream>
#include <vector>

// Terminal color codes
const std::string RESET = "\033[0m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";

int testsPassed = 0;
int testsFailed = 0;

// All available moves
const std::vector<Move> allMoves = {
    Move::U, Move::UP,
    Move::D, Move::DP,
    Move::L, Move::LP,
    Move::R, Move::RP,
    Move::F, Move::FP,
    Move::B, Move::BP,
    Move::M, Move::MP,
    Move::E, Move::EP,
    Move::S, Move::SP
};

// Check if two moves are inverses of each other
bool areInverses(Move m1, Move m2) {
    if (m1 == Move::U && m2 == Move::UP) return true;
    if (m1 == Move::UP && m2 == Move::U) return true;
    if (m1 == Move::D && m2 == Move::DP) return true;
    if (m1 == Move::DP && m2 == Move::D) return true;
    if (m1 == Move::L && m2 == Move::LP) return true;
    if (m1 == Move::LP && m2 == Move::L) return true;
    if (m1 == Move::R && m2 == Move::RP) return true;
    if (m1 == Move::RP && m2 == Move::R) return true;
    if (m1 == Move::F && m2 == Move::FP) return true;
    if (m1 == Move::FP && m2 == Move::F) return true;
    if (m1 == Move::B && m2 == Move::BP) return true;
    if (m1 == Move::BP && m2 == Move::B) return true;
    if (m1 == Move::M && m2 == Move::MP) return true;
    if (m1 == Move::MP && m2 == Move::M) return true;
    if (m1 == Move::E && m2 == Move::EP) return true;
    if (m1 == Move::EP && m2 == Move::E) return true;
    if (m1 == Move::S && m2 == Move::SP) return true;
    if (m1 == Move::SP && m2 == Move::S) return true;
    return false;
}

void printTestResult(const std::string& testName, bool passed) {
    if (passed) {
        std::cout << GREEN << "[PASS] " << testName << RESET << std::endl;
        testsPassed++;
    } else {
        std::cout << RED << "[FAIL] " << testName << RESET << std::endl;
        testsFailed++;
    }
}

int main() {
    std::cout << CYAN << "Rubik's Cube 2-Step Move Combination Tests" << RESET << std::endl;
    std::cout << "==========================================" << std::endl;

    std::cout << "\nTesting all 2-step move combinations..." << std::endl;
    std::cout << "Total moves: " << allMoves.size() << std::endl;
    std::cout << "Expected combinations (excluding inverses): "
              << (allMoves.size() * allMoves.size()) << std::endl;
    std::cout << "==========================================" << std::endl;

    int totalTests = 0;
    int skippedTests = 0;

    for (Move move1 : allMoves) {
        for (Move move2 : allMoves) {
            totalTests++;

            std::string move1Str = moveToString(move1);
            std::string move2Str = moveToString(move2);
            std::string testName = move1Str + " " + move2Str;

            // Skip inverse pairs
            if (areInverses(move1, move2)) {
                skippedTests++;
                continue;
            }

            RubiksCube cube;

            auto frontBefore = cube.getFront();
            auto backBefore = cube.getBack();
            auto leftBefore = cube.getLeft();
            auto rightBefore = cube.getRight();
            auto upBefore = cube.getUp();
            auto downBefore = cube.getDown();

            cube.executeMove(move1);
            cube.executeMove(move2);

            // Check if at least one face changed
            bool changed = (cube.getFront() != frontBefore) ||
                         (cube.getBack() != backBefore) ||
                         (cube.getLeft() != leftBefore) ||
                         (cube.getRight() != rightBefore) ||
                         (cube.getUp() != upBefore) ||
                         (cube.getDown() != downBefore);

            printTestResult(testName, changed);
        }
    }

    std::cout << "\n==========================================" << std::endl;
    std::cout << YELLOW << "Test Results:" << RESET << std::endl;
    std::cout << "Total combinations tested: " << totalTests << std::endl;
    std::cout << "Skipped (inverse pairs): " << skippedTests << std::endl;
    std::cout << "Executed: " << (totalTests - skippedTests) << std::endl;
    std::cout << GREEN << "Passed: " << testsPassed << RESET << std::endl;
    std::cout << RED << "Failed: " << testsFailed << RESET << std::endl;
    std::cout << "==========================================" << std::endl;

    return (testsFailed == 0) ? 0 : 1;
}
