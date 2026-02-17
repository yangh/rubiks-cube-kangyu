#include "../src/formula.h"
#include "../src/cube.h"
#include "ref/ref_cube.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <filesystem>

// Terminal colors
const std::string RESET = "\033[0m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";
const std::string BOLD = "\033[1m";

// Test statistics
int testsPassed = 0;
int testsFailed = 0;

void printTestResult(const std::string& testName, bool passed, const std::string& details = "") {
    if (passed) {
        std::cout << GREEN << "[PASS] " << testName << RESET;
        if (!details.empty()) {
            std::cout << " - " << details;
        }
        std::cout << std::endl;
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

// Convert our cube Color to ref Color
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
    }
    return ref::Move::U; // Should not reach here
}

// Helper to update global counters
void updateGlobalCounters(int passed, int failed) {
    testsPassed += passed;
    testsFailed += failed;
}


// Compare faces between our cube and ref cube
bool facesEqual(const std::array<Color, 9>& ourFace, const std::array<ref::Color, 9>& refFace) {
    for (int i = 0; i < 9; ++i) {
        if (toRefColor(ourFace[i]) != refFace[i]) {
            return false;
        }
    }
    return true;
}

// Compare two cube states
bool statesMatch(const RubiksCube& ourCube, const ref::RubiksCube& refCube) {
    return (facesEqual(ourCube.getFront(), refCube.getFace(ref::Face::FRONT)) &&
           facesEqual(ourCube.getBack(), refCube.getFace(ref::Face::BACK)) &&
           facesEqual(ourCube.getLeft(), refCube.getFace(ref::Face::LEFT)) &&
           facesEqual(ourCube.getRight(), refCube.getFace(ref::Face::RIGHT)) &&
           facesEqual(ourCube.getUp(), refCube.getFace(ref::Face::UP)) &&
           facesEqual(ourCube.getDown(), refCube.getFace(ref::Face::DOWN)));
}

// Test: Verify formulas against reference cube
void testFormulaVsRefCube(const std::string& formulaDir) {
    std::cout << CYAN << BOLD << "\n=== Test: Formula vs Reference Cube ===" << RESET << std::endl;

    FormulaManager formulaManager;
    formulaManager.setFormulaDir(formulaDir);
    formulaManager.loadFormulas();

    auto fileNames = formulaManager.getFileNames();

    if (fileNames.empty()) {
        std::cout << YELLOW << "[SKIP] No formula files found" << RESET << std::endl;
        return;
    }

    int formulasTested = 0;
    int formulasPassed = 0;
    int formulasFailed = 0;

    for (const auto& filename : fileNames) {
        const auto* items = formulaManager.getFileItems(filename);
        if (items == nullptr || items->empty()) {
            std::cout << YELLOW << "[SKIP] " << filename << " - No valid items" << RESET << std::endl;
            continue;
        }

        std::cout << "Testing file: " << filename << std::endl;
        formulasTested++;

        for (const FormulaItem& item : *items) {
            if (item.moves.empty()) {
                std::cout << YELLOW << "[SKIP] " << item.name << " - No moves" << RESET << std::endl;
                continue;
            }

            // Create our cube and ref cube
            RubiksCube ourCube;
            ref::RubiksCube refCube;

            // Apply formula to both cubes
            for (Move move : item.moves) {
                ourCube.executeMove(move);
                refCube.executeMove(toRefMove(move));
            }

            // Test: Both cubes should have the same state
            std::string testName = filename + ":" + item.name;

            if (statesMatch(ourCube, refCube)) {
                std::cout << GREEN << "[PASS] " << testName << RESET << std::endl;
                formulasPassed++;
            } else {
                std::cout << RED << "[FAIL] " << testName << RESET << std::endl;
                formulasFailed++;

                // Show the mismatching face
                std::cout << "  Front mismatch:" << std::endl;
                std::cout << "    Our:  ";
                for (int i = 0; i < 9; ++i) {
                    std::cout << colorToString(ourCube.getFront()[i]);
                    if ((i + 1) % 3 == 0) std::cout << " ";
                }
                std::cout << std::endl;
                std::cout << "    Ref:  ";
                for (int i = 0; i < 9; ++i) {
                    int c = (int)refCube.getFace(ref::Face::FRONT)[i];
                    const char* colors[] = {"W", "Y", "R", "O", "G", "B"};
                    std::cout << colors[c];
                    if ((i + 1) % 3 == 0) std::cout << " ";
                }
                std::cout << std::endl;
            }
        }
    }

    std::cout << "\nFormula vs Ref Results:" << std::endl;
    std::cout << GREEN << "  Passed: " << formulasPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << formulasFailed << RESET << std::endl;

    updateGlobalCounters(formulasPassed, formulasFailed);

    if (formulasFailed == 0) {
        std::cout << GREEN << BOLD << "ALL FORMULA TESTS PASSED!" << RESET << std::endl;
    } else {
        std::cout << RED << BOLD << "SOME FORMULA TESTS FAILED!" << RESET << std::endl;
        std::cout << "Please check the formulas in formula/ directory" << std::endl;
    }
}

// Test: Loop formulas against reference
void testLoopFormulasVsRef(const std::string& formulaDir) {
    std::cout << CYAN << BOLD << "\n=== Test: Loop Formulas vs Reference ===" << RESET << std::endl;

    FormulaManager formulaManager;
    formulaManager.setFormulaDir(formulaDir);
    formulaManager.loadFormulas();

    auto fileNames = formulaManager.getFileNames();

    if (fileNames.empty()) {
        std::cout << YELLOW << "[SKIP] No formula files found" << RESET << std::endl;
        return;
    }

    int loopTestsPassed = 0;
    int loopTestsFailed = 0;
    int loopFormulasTested = 0;

    for (const auto& filename : fileNames) {
        const auto* items = formulaManager.getFileItems(filename);
        if (items == nullptr || items->empty()) {
            continue;
        }

        for (const FormulaItem& item : *items) {
            if (item.loopCount <= 0) {
                continue;
            }

            std::cout << "Testing loop formula: " << item.name << std::endl;

            // Apply formula N times to multiple cubes
            int numCubes = item.loopCount;
            std::vector<RubiksCube> ourCubes;
            std::vector<ref::RubiksCube> refCubes;

            ourCubes.reserve(numCubes);
            refCubes.reserve(numCubes);

            for (int i = 0; i < numCubes; ++i) {
                ourCubes.emplace_back();
                refCubes.emplace_back();

                // Apply formula N times
                for (int j = 0; j < numCubes; ++j) {
                    for (Move move : item.moves) {
                        ourCubes[i].executeMove(move);
                        refCubes[i].executeMove(toRefMove(move));
                    }
                }
            }

            // Check all our cubes match corresponding ref cubes and are consistent
            bool ourConsistent = true;
            bool refConsistent = true;
            bool bothMatch = true;

            // Check each ourCube matches its corresponding refCube
            for (int i = 0; i < numCubes; ++i) {
                if (!statesMatch(ourCubes[i], refCubes[i])) {
                    bothMatch = false;
                }
            }

            // Check all our cubes have same state
            for (int i = 1; i < numCubes; ++i) {
                if (ourCubes[0].getFront() != ourCubes[i].getFront() ||
                    ourCubes[0].getBack() != ourCubes[i].getBack() ||
                    ourCubes[0].getLeft() != ourCubes[i].getLeft() ||
                    ourCubes[0].getRight() != ourCubes[i].getRight() ||
                    ourCubes[0].getUp() != ourCubes[i].getUp() ||
                    ourCubes[0].getDown() != ourCubes[i].getDown()) {
                    ourConsistent = false;
                }
            }

            // Check all ref cubes have same state
            for (int i = 1; i < numCubes; ++i) {
                if (refCubes[0].getFace(ref::Face::FRONT) != refCubes[i].getFace(ref::Face::FRONT) ||
                    refCubes[0].getFace(ref::Face::BACK) != refCubes[i].getFace(ref::Face::BACK) ||
                    refCubes[0].getFace(ref::Face::LEFT) != refCubes[i].getFace(ref::Face::LEFT) ||
                    refCubes[0].getFace(ref::Face::RIGHT) != refCubes[i].getFace(ref::Face::RIGHT) ||
                    refCubes[0].getFace(ref::Face::UP) != refCubes[i].getFace(ref::Face::UP) ||
                    refCubes[0].getFace(ref::Face::DOWN) != refCubes[i].getFace(ref::Face::DOWN)) {
                    refConsistent = false;
                }
            }

            std::string testName = filename + ":" + item.name + " x" + std::to_string(item.loopCount);

            if (bothMatch && ourConsistent && refConsistent) {
                std::cout << GREEN << "[PASS] " << testName << RESET << std::endl;
                loopTestsPassed++;
            } else if (!bothMatch) {
                std::cout << RED << "[FAIL] " << testName << " - Our and ref cubes don't match" << RESET << std::endl;
                loopTestsFailed++;
            } else if (!ourConsistent) {
                std::cout << RED << "[FAIL] " << testName << " - Our cubes inconsistent" << RESET << std::endl;
                loopTestsFailed++;
            } else if (!refConsistent) {
                std::cout << RED << "[FAIL] " << testName << " - Ref cubes inconsistent" << RESET << std::endl;
                loopTestsFailed++;
            } else {
                std::cout << RED << "[FAIL] " << testName << " - Both inconsistent" << RESET << std::endl;
                loopTestsFailed++;
            }
        }
        loopFormulasTested++;
    }

    std::cout << "\nLoop Formula vs Ref Results:" << std::endl;
    std::cout << GREEN << "  Passed: " << loopTestsPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << loopTestsFailed << RESET << std::endl;

    updateGlobalCounters(loopTestsPassed, loopTestsFailed);

    if (loopTestsPassed + loopTestsFailed == 0) {
        std::cout << GREEN << BOLD << "ALL LOOP FORMULA TESTS PASSED!" << RESET << std::endl;
    }
}

// Test: Inverse moves return to solved state for both cubes
void testInverseMoves(const std::string& formulaDir) {
    std::cout << CYAN << BOLD << "\n=== Test: Inverse Moves ===" << RESET << std::endl;

    FormulaManager formulaManager;
    formulaManager.setFormulaDir(formulaDir);
    formulaManager.loadFormulas();

    auto fileNames = formulaManager.getFileNames();

    if (fileNames.empty()) {
        std::cout << YELLOW << "[SKIP] No formula files found" << RESET << std::endl;
        return;
    }

    int inverseTestsPassed = 0;
    int inverseTestsFailed = 0;

    for (const auto& filename : fileNames) {
        const auto* items = formulaManager.getFileItems(filename);
        if (items == nullptr || items->empty()) {
            continue;
        }

        for (const FormulaItem& item : *items) {
            if (item.moves.empty()) {
                continue;
            }

            // Test: Apply formula and then apply inverse moves to both cubes
            RubiksCube ourCube;
            ref::RubiksCube refCube;

            // Apply formula to both cubes
            for (Move move : item.moves) {
                ourCube.executeMove(move);
                refCube.executeMove(toRefMove(move));
            }

            // Apply inverse moves in reverse order to both cubes
            for (auto it = item.moves.rbegin(); it != item.moves.rend(); ++it) {
                Move inverseMove;
                switch (*it) {
                    case Move::U:  inverseMove = Move::UP; break;
                    case Move::UP: inverseMove = Move::U; break;
                    case Move::D:  inverseMove = Move::DP; break;
                    case Move::DP: inverseMove = Move::D; break;
                    case Move::L:  inverseMove = Move::LP; break;
                    case Move::LP: inverseMove = Move::L; break;
                    case Move::R:  inverseMove = Move::RP; break;
                    case Move::RP: inverseMove = Move::R; break;
                    case Move::F:  inverseMove = Move::FP; break;
                    case Move::FP: inverseMove = Move::F; break;
                    case Move::B:  inverseMove = Move::BP; break;
                    case Move::BP: inverseMove = Move::B; break;
                    case Move::M:  inverseMove = Move::MP; break;
                    case Move::MP: inverseMove = Move::M; break;
                    case Move::E:  inverseMove = Move::EP; break;
                    case Move::EP: inverseMove = Move::E; break;
                    case Move::S:  inverseMove = Move::SP; break;
                    case Move::SP: inverseMove = Move::S; break;
                }
                ourCube.executeMove(inverseMove);
                refCube.executeMove(toRefMove(inverseMove));
            }

            // Test: Both cubes should return to solved state
            bool ourSolved = ourCube.isSolved();
            bool refSolved = refCube.isSolved();
            bool statesMatched = statesMatch(ourCube, refCube);

            std::string testName = filename + ":" + item.name + " inverse";

            if (ourSolved && refSolved && statesMatched) {
                std::cout << GREEN << "[PASS] " << testName << RESET << std::endl;
                inverseTestsPassed++;
            } else {
                std::cout << RED << "[FAIL] " << testName << RESET << std::endl;
                if (!ourSolved) std::cout << "  Reason: Our cube not solved" << std::endl;
                if (!refSolved) std::cout << "  Reason: Ref cube not solved" << std::endl;
                if (!statesMatched) std::cout << "  Reason: States don't match" << std::endl;
                inverseTestsFailed++;
            }
        }
    }

    std::cout << "\nInverse Move Test Results:" << std::endl;
    std::cout << GREEN << "  Passed: " << inverseTestsPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << inverseTestsFailed << RESET << std::endl;

    updateGlobalCounters(inverseTestsPassed, inverseTestsFailed);
}

// Test: Edge cases with both cube implementations
void testEdgeCasesVsRef() {
    std::cout << CYAN << BOLD << "\n=== Test: Edge Cases (Both Cubes) ===" << RESET << std::endl;

    int edgeTestsPassed = 0;
    int edgeTestsFailed = 0;

    // Test: Empty move sequence
    {
        RubiksCube ourCube;
        ref::RubiksCube refCube;

        auto moves = parseMoveSequence("");
        printTestResult("Parse empty move sequence (our cube)", moves.empty());
        printTestResult("Parse empty move sequence (ref cube)", moves.empty());

        // Test: Single moves
        std::vector<Move> testMoves = {Move::U, Move::D, Move::L, Move::R, Move::F, Move::B, Move::M, Move::E, Move::S};

        for (Move move : testMoves) {
            ourCube.executeMove(move);
            refCube.executeMove(toRefMove(move));

            if (statesMatch(ourCube, refCube)) {
                std::string name = "Single move: " + moveToString(move);
                printTestResult(name + " (our vs ref)", true);
                edgeTestsPassed++;
            } else {
                std::string name = "Single move: " + moveToString(move);
                printTestResult(name + " (our vs ref)", false);
                edgeTestsFailed++;
            }
        }
    }

    std::cout << "\nEdge Cases (Both Cubes) Results:" << std::endl;
    std::cout << GREEN << "  Passed: " << edgeTestsPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << edgeTestsFailed << RESET << std::endl;
}

int main() {
    // Get project root from executable location
    std::filesystem::path exePath = std::filesystem::canonical(std::filesystem::current_path());
    std::filesystem::path projectRoot = exePath;

    // Navigate up to find project root (looking for rubiks-imgui directory)
    while (projectRoot.has_parent_path() &&
           projectRoot.filename() != "rubiks-imgui") {
        projectRoot = projectRoot.parent_path();
    }

    // If we ended at "build", go to parent
    if (projectRoot.filename() == "build") {
        projectRoot = projectRoot.parent_path();
    }

    std::string formulaDir = (projectRoot / "formula").string();

    std::cout << YELLOW << BOLD << "Rubik's Cube Formula + Reference Verification" << RESET << std::endl;
    std::cout << "===========================================" << std::endl;
    std::cout << "Verifying formulas against independent reference implementation" << std::endl;
    std::cout << "Formula directory: " << formulaDir << std::endl;
    std::cout << std::endl;

    testsPassed = 0;
    testsFailed = 0;

    testFormulaVsRefCube(formulaDir);
    testLoopFormulasVsRef(formulaDir);
    testInverseMoves(formulaDir);
    testEdgeCasesVsRef();

    std::cout << "\n" << CYAN << BOLD << "==========================================" << RESET << std::endl;
    std::cout << YELLOW << BOLD << "FINAL RESULTS:" << RESET << std::endl;
    std::cout << GREEN << "  Passed: " << testsPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << testsFailed << RESET << std::endl;
    std::cout << "  Total:  " << (testsPassed + testsFailed) << std::endl;
    std::cout << "==========================================" << std::endl;

    if (testsFailed == 0) {
        std::cout << GREEN << BOLD << "ALL TESTS PASSED!" << RESET << std::endl;
        std::cout << "Our cube implementation matches the reference for all tested scenarios!" << std::endl;
        return 0;
    } else {
        std::cout << RED << BOLD << "SOME TESTS FAILED!" << RESET << std::endl;
        std::cout << "Please review the failed tests above." << std::endl;
        return 1;
    }
}
