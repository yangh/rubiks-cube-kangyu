#include "../src/cube.h"
#include "ref/ref_cube.h"
#include <iostream>
#include <vector>
#include <iomanip>

// Terminal colors
const std::string RESET = "\033[0m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";
const std::string BOLD = "\033[1m";

// Convert our Color to ref Color
ref::Color toRefColor(Color color) {
    switch (color) {
        case Color::WHITE:  return ref::Color::WHITE;
        case Color::YELLOW: return ref::Color::YELLOW;
        case Color::RED:    return ref::Color::RED;
        case Color::ORANGE: return ref::Color::ORANGE;
        case Color::GREEN:  return ref::Color::GREEN;
        case Color::BLUE:   return ref::Color::BLUE;
    }
    return ref::Color::WHITE; // Should not reach here
}

// Convert our Move to ref Move
ref::Move toRefMove(Move move) {
    switch (move) {
        case Move::U:  return ref::Move::U;
        case Move::UP: return ref::Move::UP;
        case Move::D:  return ref::Move::D;
        case Move::DP: return ref::Move::DP;
        case Move::L:  return ref::Move::L;
        case Move::LP: return ref::Move::LP;
        case Move::R:  return ref::Move::R;
        case Move::RP: return ref::Move::RP;
        case Move::F:  return ref::Move::F;
        case Move::FP: return ref::Move::FP;
        case Move::B:  return ref::Move::B;
        case Move::BP: return ref::Move::BP;
        case Move::M:  return ref::Move::M;
        case Move::MP: return ref::Move::MP;
        case Move::E:  return ref::Move::E;
        case Move::EP: return ref::Move::EP;
        case Move::S:  return ref::Move::S;
        case Move::SP: return ref::Move::SP;
        case Move::X:  return ref::Move::X;
        case Move::XP: return ref::Move::XP;
        case Move::Y:  return ref::Move::Y;
        case Move::YP: return ref::Move::YP;
        case Move::Z:  return ref::Move::Z;
        case Move::ZP: return ref::Move::ZP;
    }
    return ref::Move::U; // Should not reach here
}

// Check if two faces are equal
bool facesEqual(const std::array<Color, 9>& f1, const std::array<ref::Color, 9>& f2) {
    for (int i = 0; i < 9; ++i) {
        if (toRefColor(f1[i]) != f2[i]) {
            return false;
        }
    }
    return true;
}

// Print a face comparison
void printFaceMismatch(const std::string& faceName,
                    const std::array<Color, 9>& ourFace,
                    const std::array<ref::Color, 9>& refFace) {
    std::cout << "    " << faceName << " face mismatch:" << std::endl;
    std::cout << "      Our:  ";
    for (int i = 0; i < 9; ++i) {
        std::cout << colorToString(ourFace[i]) << " ";
        if ((i + 1) % 3 == 0) std::cout << std::endl;
    }
    std::cout << std::endl << "      Ref:  ";
    for (int i = 0; i < 9; ++i) {
        int c = (int)refFace[i];
        const char* colors[] = {"W", "Y", "R", "O", "G", "B"};
        std::cout << colors[c] << " ";
        if ((i + 1) % 3 == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Test single moves
int testSingleMoves() {
    std::cout << CYAN << BOLD << "\n=== Testing Single Moves ===" << RESET << std::endl;

    const std::vector<Move> allMoves = {
        Move::U, Move::UP, Move::D, Move::DP,
        Move::L, Move::LP, Move::R, Move::RP,
        Move::F, Move::FP, Move::B, Move::BP,
        Move::M, Move::MP, Move::E, Move::EP, Move::S, Move::SP,
        Move::X, Move::XP, Move::Y, Move::YP, Move::Z, Move::ZP
    };

    int passed = 0;
    int failed = 0;
    std::vector<std::string> failedMoves;

    for (Move move : allMoves) {
        RubiksCube ourCube;
        ref::RubiksCube refCube;

        ourCube.executeMove(move);
        refCube.executeMove(toRefMove(move));

        std::string moveStr = moveToString(move);

        bool allMatch = facesEqual(ourCube.getFront(), refCube.getFace(ref::Face::FRONT)) &&
                        facesEqual(ourCube.getBack(), refCube.getFace(ref::Face::BACK)) &&
                        facesEqual(ourCube.getLeft(), refCube.getFace(ref::Face::LEFT)) &&
                        facesEqual(ourCube.getRight(), refCube.getFace(ref::Face::RIGHT)) &&
                        facesEqual(ourCube.getUp(), refCube.getFace(ref::Face::UP)) &&
                        facesEqual(ourCube.getDown(), refCube.getFace(ref::Face::DOWN));

        if (allMatch) {
            std::cout << GREEN << "[PASS] " << moveStr << RESET << std::endl;
            passed++;
        } else {
            std::cout << RED << "[FAIL] " << moveStr << RESET << std::endl;
            failed++;
            failedMoves.push_back(moveStr);

            if (!facesEqual(ourCube.getFront(), refCube.getFace(ref::Face::FRONT))) {
                printFaceMismatch("Front", ourCube.getFront(), refCube.getFace(ref::Face::FRONT));
            }
        }
    }

    std::cout << "\nSingle Move Results:" << std::endl;
    std::cout << "  Total:  " << allMoves.size() << std::endl;
    std::cout << GREEN << "  Passed: " << passed << RESET << std::endl;
    std::cout << RED << "  Failed: " << failed << RESET << std::endl;

    if (!failedMoves.empty()) {
        std::cout << "\nFailed moves:";
        for (const auto& m : failedMoves) {
            std::cout << " " << m;
        }
        std::cout << std::endl;
    }

    return failed;
}

// Test 2-step combinations (excluding duplicates and inverses)
int testTwoStepCombos() {
    std::cout << CYAN << BOLD << "\n=== Testing 2-Step Combinations ===" << RESET << std::endl;

    const std::vector<Move> allMoves = {
        Move::U, Move::UP, Move::D, Move::DP,
        Move::L, Move::LP, Move::R, Move::RP,
        Move::F, Move::FP, Move::B, Move::BP,
        Move::M, Move::MP, Move::E, Move::EP, Move::S, Move::SP,
        Move::X, Move::XP, Move::Y, Move::YP, Move::Z, Move::ZP
    };

    auto areInverses = [](Move m1, Move m2) -> bool {
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
        if (m1 == Move::X && m2 == Move::XP) return true;
        if (m1 == Move::XP && m2 == Move::X) return true;
        if (m1 == Move::Y && m2 == Move::YP) return true;
        if (m1 == Move::YP && m2 == Move::Y) return true;
        if (m1 == Move::Z && m2 == Move::ZP) return true;
        if (m1 == Move::ZP && m2 == Move::Z) return true;
        return false;
    };

    int total = 0;
    int skipped = 0;
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failedCombos;

    for (Move move1 : allMoves) {
        for (Move move2 : allMoves) {
            total++;

            std::string combo = moveToString(move1) + " + " + moveToString(move2);

            // Skip inverse pairs and duplicate moves
            if (areInverses(move1, move2) || move1 == move2) {
                skipped++;
                continue;
            }

            RubiksCube ourCube;
            ref::RubiksCube refCube;

            ourCube.executeMove(move1);
            ourCube.executeMove(move2);
            refCube.executeMove(toRefMove(move1));
            refCube.executeMove(toRefMove(move2));

            bool allMatch = facesEqual(ourCube.getFront(), refCube.getFace(ref::Face::FRONT)) &&
                            facesEqual(ourCube.getBack(), refCube.getFace(ref::Face::BACK)) &&
                            facesEqual(ourCube.getLeft(), refCube.getFace(ref::Face::LEFT)) &&
                            facesEqual(ourCube.getRight(), refCube.getFace(ref::Face::RIGHT)) &&
                            facesEqual(ourCube.getUp(), refCube.getFace(ref::Face::UP)) &&
                            facesEqual(ourCube.getDown(), refCube.getFace(ref::Face::DOWN));

            if (allMatch) {
                std::cout << GREEN << "[PASS] " << combo << RESET << std::endl;
                passed++;
            } else {
                std::cout << RED << "[FAIL] " << combo << RESET << std::endl;
                failed++;
                failedCombos.push_back(combo);
            }
        }
    }

    std::cout << "\n2-Step Combo Results:" << std::endl;
    std::cout << "  Total combinations: " << total << std::endl;
    std::cout << "  Skipped (duplicates/inverses): " << skipped << std::endl;
    std::cout << "  Executed: " << (total - skipped) << std::endl;
    std::cout << GREEN << "  Passed: " << passed << RESET << std::endl;
    std::cout << RED << "  Failed: " << failed << RESET << std::endl;

    if (!failedCombos.empty()) {
        std::cout << "\nFailed combinations (" << failedCombos.size() << "):" << std::endl;
        for (const auto& c : failedCombos) {
            std::cout << "  " << c << std::endl;
        }
    }

    return failed;
}

// Test inverse moves (our cube should return to solved state after move + inverse)
int testInverseMoves() {
    std::cout << CYAN << BOLD << "\n=== Testing Inverse Moves ===" << RESET << std::endl;

    struct TestCase {
        Move move1;
        Move move2;
        std::string name;
    };

    std::vector<TestCase> tests = {
        {Move::U, Move::UP, "U + U'"},
        {Move::D, Move::DP, "D + D'"},
        {Move::L, Move::LP, "L + L'"},
        {Move::R, Move::RP, "R + R'"},
        {Move::F, Move::FP, "F + F'"},
        {Move::B, Move::BP, "B + B'"},
        {Move::M, Move::MP, "M + M'"},
        {Move::E, Move::EP, "E + E'"},
        {Move::S, Move::SP, "S + S'"}
    };

    int passed = 0;
    int failed = 0;

    for (const auto& test : tests) {
        RubiksCube ourCube;
        ref::RubiksCube refCube;

        ourCube.executeMove(test.move1);
        ourCube.executeMove(test.move2);
        refCube.executeMove(toRefMove(test.move1));
        refCube.executeMove(toRefMove(test.move2));

        bool allMatch = facesEqual(ourCube.getFront(), refCube.getFace(ref::Face::FRONT)) &&
                        facesEqual(ourCube.getBack(), refCube.getFace(ref::Face::BACK)) &&
                        facesEqual(ourCube.getLeft(), refCube.getFace(ref::Face::LEFT)) &&
                        facesEqual(ourCube.getRight(), refCube.getFace(ref::Face::RIGHT)) &&
                        facesEqual(ourCube.getUp(), refCube.getFace(ref::Face::UP)) &&
                        facesEqual(ourCube.getDown(), refCube.getFace(ref::Face::DOWN));

        bool ourSolved = ourCube.isSolved();
        bool refSolved = refCube.isSolved();

        if (allMatch && ourSolved && refSolved) {
            std::cout << GREEN << "[PASS] " << test.name << RESET << std::endl;
            passed++;
        } else {
            std::cout << RED << "[FAIL] " << test.name << RESET << std::endl;
            if (!allMatch) std::cout << "  Reason: State mismatch" << std::endl;
            if (!ourSolved) std::cout << "  Reason: Our cube not solved" << std::endl;
            if (!refSolved) std::cout << "  Reason: Ref cube not solved" << std::endl;
            failed++;
        }
    }

    std::cout << "\nInverse Move Results:" << std::endl;
    std::cout << GREEN << "  Passed: " << passed << RESET << std::endl;
    std::cout << RED << "  Failed: " << failed << RESET << std::endl;

    return failed;
}

// Test 4x moves (should return to solved state)
int testFourMoves() {
    std::cout << CYAN << BOLD << "\n=== Testing 4x Moves ===" << RESET << std::endl;

    struct TestCase {
        Move move;
        std::string name;
    };

    std::vector<TestCase> tests = {
        {Move::U, "U x 4"},
        {Move::D, "D x 4"},
        {Move::L, "L x 4"},
        {Move::R, "R x 4"},
        {Move::F, "F x 4"},
        {Move::B, "B x 4"},
        {Move::M, "M x 4"},
        {Move::E, "E x 4"},
        {Move::S, "S x 4"}
    };

    int passed = 0;
    int failed = 0;

    for (const auto& test : tests) {
        RubiksCube ourCube;
        ref::RubiksCube refCube;

        for (int i = 0; i < 4; i++) {
            ourCube.executeMove(test.move);
            refCube.executeMove(toRefMove(test.move));
        }

        bool allMatch = facesEqual(ourCube.getFront(), refCube.getFace(ref::Face::FRONT)) &&
                        facesEqual(ourCube.getBack(), refCube.getFace(ref::Face::BACK)) &&
                        facesEqual(ourCube.getLeft(), refCube.getFace(ref::Face::LEFT)) &&
                        facesEqual(ourCube.getRight(), refCube.getFace(ref::Face::RIGHT)) &&
                        facesEqual(ourCube.getUp(), refCube.getFace(ref::Face::UP)) &&
                        facesEqual(ourCube.getDown(), refCube.getFace(ref::Face::DOWN));

        bool ourSolved = ourCube.isSolved();
        bool refSolved = refCube.isSolved();

        if (allMatch && ourSolved && refSolved) {
            std::cout << GREEN << "[PASS] " << test.name << RESET << std::endl;
            passed++;
        } else {
            std::cout << RED << "[FAIL] " << test.name << RESET << std::endl;
            failed++;
        }
    }

    std::cout << "\n4x Move Results:" << std::endl;
    std::cout << GREEN << "  Passed: " << passed << RESET << std::endl;
    std::cout << RED << "  Failed: " << failed << RESET << std::endl;

    return failed;
}

int main() {
    std::cout << YELLOW << BOLD << "Rubik's Cube Reference Verification" << RESET << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "Comparing our cube implementation against reference implementation" << std::endl;
    std::cout << std::endl;

    int totalFailures = 0;

    totalFailures += testSingleMoves();
    totalFailures += testTwoStepCombos();
    totalFailures += testInverseMoves();
    totalFailures += testFourMoves();

    std::cout << "\n" << CYAN << BOLD << "====================================" << RESET << std::endl;
    std::cout << "OVERALL RESULT:" << std::endl;

    if (totalFailures == 0) {
        std::cout << GREEN << BOLD << "ALL TESTS PASSED!" << RESET << std::endl;
        std::cout << "Our cube implementation matches the reference for all tested scenarios." << std::endl;
    } else {
        std::cout << RED << BOLD << "SOME TESTS FAILED!" << RESET << std::endl;
        std::cout << "Total failures: " << totalFailures << std::endl;
        std::cout << "Please review the failed moves above and check the rotation logic in cube.cpp" << std::endl;
    }
    std::cout << "====================================" << std::endl;

    return totalFailures;
}
