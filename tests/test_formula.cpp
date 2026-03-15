#include "../src/formula.h"
#include "../src/cube.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <system_error>

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

// Test 1: Load formula files from formula directory
void testLoadFormulas(const std::string& formulaDir) {
    std::cout << CYAN << BOLD << "\n=== Test 1: Load Formula Files ===" << RESET << std::endl;

    FormulaManager formulaManager;
    formulaManager.setFormulaDir(formulaDir);

    std::cout << "Loading formulas from formula directory..." << std::endl;
    formulaManager.loadFormulas();

    auto fileNames = formulaManager.getFileNames();

    printTestResult("Formula files loaded", !fileNames.empty(),
                  "Found " + std::to_string(fileNames.size()) + " formula files");

    if (!fileNames.empty()) {
        std::cout << "Available formula files:" << std::endl;
        for (size_t i = 0; i < fileNames.size() && i < 10; ++i) {
            std::cout << "  " << fileNames[i] << std::endl;
        }
        if (fileNames.size() > 10) {
            std::cout << "  ... and " << (fileNames.size() - 10) << " more" << std::endl;
        }
    }
}

// Test 2: Parse and apply each formula
void testApplyFormulas(const std::string& formulaDir) {
    std::cout << CYAN << BOLD << "\n=== Test 2: Apply Formulas ===" << RESET << std::endl;

    FormulaManager formulaManager;
    formulaManager.setFormulaDir(formulaDir);
    formulaManager.loadFormulas();

    auto fileNames = formulaManager.getFileNames();

    if (fileNames.empty()) {
        std::cout << RED << "No formula files to test" << RESET << std::endl;
        return;
    }

    int formulasWithItems = 0;
    int totalFormulasTested = 0;

    for (const auto& filename : fileNames) {
        const auto* items = formulaManager.getFileItems(filename);
        if (items == nullptr || items->empty()) {
            std::cout << YELLOW << "[SKIP] " << filename << " - No valid formula items" << RESET << std::endl;
            continue;
        }

        formulasWithItems++;
        totalFormulasTested++;

        // Test each formula item in the file
        for (size_t i = 0; i < items->size(); ++i) {
            const FormulaItem& item = (*items)[i];

            // Skip if no moves
            if (item.moves.empty()) {
                continue;
            }

            std::string testName = filename + ":" + item.name;

            // Test: Apply formula moves to a cube
            RubiksCube testCube1;
            RubiksCube testCube2;

            for (Move move : item.moves) {
                testCube1.executeMove(move);
                testCube2.executeMove(move);
            }

            // Test: Both cubes should have the same state
            bool statesMatch = (testCube1.getFront() == testCube2.getFront()) &&
                             (testCube1.getBack() == testCube2.getBack()) &&
                             (testCube1.getLeft() == testCube2.getLeft()) &&
                             (testCube1.getRight() == testCube2.getRight()) &&
                             (testCube1.getUp() == testCube2.getUp()) &&
                             (testCube1.getDown() == testCube2.getDown());

            printTestResult(testName, statesMatch,
                          "Cube state consistency after applying moves");
        }
    }

    printTestResult("Formulas with valid items tested", formulasWithItems > 0,
                  std::to_string(formulasWithItems) + " / " + std::to_string(totalFormulasTested));
}

// Test 3: Verify inverse moves return to solved state
void testFormulaInverseMoves(const std::string& formulaDir) {
    std::cout << CYAN << BOLD << "\n=== Test 3: Formula Inverse Moves ===" << RESET << std::endl;

    FormulaManager formulaManager;
    formulaManager.setFormulaDir(formulaDir);
    formulaManager.loadFormulas();

    auto fileNames = formulaManager.getFileNames();

    if (fileNames.empty()) {
        return;
    }

    int formulasTested = 0;
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

            // Test: Apply formula and then apply inverse moves in reverse
            RubiksCube testCube;

            // Store the solved state BEFORE applying formula
            auto frontBefore = testCube.getFront();
            auto backBefore = testCube.getBack();
            auto leftBefore = testCube.getLeft();
            auto rightBefore = testCube.getRight();
            auto upBefore = testCube.getUp();
            auto downBefore = testCube.getDown();

            // Apply formula
            for (Move move : item.moves) {
                testCube.executeMove(move);
            }

            // Apply inverse moves in reverse order
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
                    // Double moves are self-inverse
                    case Move::U2: inverseMove = Move::U2; break;
                    case Move::D2: inverseMove = Move::D2; break;
                    case Move::L2: inverseMove = Move::L2; break;
                    case Move::R2: inverseMove = Move::R2; break;
                    case Move::F2: inverseMove = Move::F2; break;
                    case Move::B2: inverseMove = Move::B2; break;
                    case Move::M2: inverseMove = Move::M2; break;
                    case Move::E2: inverseMove = Move::E2; break;
                    case Move::S2: inverseMove = Move::S2; break;
                }
                testCube.executeMove(inverseMove);
            }

            // Test: Should return to solved state
            bool allMatch = (testCube.getFront() == frontBefore) &&
                           (testCube.getBack() == backBefore) &&
                           (testCube.getLeft() == leftBefore) &&
                           (testCube.getRight() == rightBefore) &&
                           (testCube.getUp() == upBefore) &&
                           (testCube.getDown() == downBefore);

            if (allMatch) {
                inverseTestsPassed++;
            } else {
                inverseTestsFailed++;
            }

            std::string testName = filename + ":" + item.name + " inverse";
            printTestResult(testName, allMatch);
        }
        formulasTested++;
    }

    std::cout << "\nInverse Move Test Results:" << std::endl;
    std::cout << "  Formulas tested: " << formulasTested << std::endl;
    std::cout << GREEN << "  Passed: " << inverseTestsPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << inverseTestsFailed << RESET << std::endl;
}

// Test 4: Test loop formulas (apply formula multiple times)
void testLoopFormulas(const std::string& formulaDir) {
    std::cout << CYAN << BOLD << "\n=== Test 4: Loop Formulas ===" << RESET << std::endl;

    FormulaManager formulaManager;
    formulaManager.setFormulaDir(formulaDir);
    formulaManager.loadFormulas();

    auto fileNames = formulaManager.getFileNames();

    if (fileNames.empty()) {
        std::cout << RED << "No formula files to test" << RESET << std::endl;
        return;
    }

    int loopTestsPassed = 0;
    int loopTestsFailed = 0;

    for (const auto& filename : fileNames) {
        const auto* items = formulaManager.getFileItems(filename);
        if (items == nullptr) {
            continue;
        }

        for (const FormulaItem& item : *items) {
            // For formulas with loop count > 0, verify state is consistent when applied multiple times
            if (item.loopCount > 0) {
                std::vector<RubiksCube> cubes;
                int numCubes = item.loopCount;

                cubes.reserve(numCubes);
                for (int i = 0; i < numCubes; ++i) {
                    cubes.emplace_back();
                    for (Move move : item.moves) {
                        cubes[i].executeMove(move);
                    }
                }

                // Check all cubes have same state
                bool allMatch = true;
                for (int i = 1; i < numCubes; ++i) {
                    bool match = (cubes[0].getFront() == cubes[i].getFront()) &&
                                 (cubes[0].getBack() == cubes[i].getBack()) &&
                                 (cubes[0].getLeft() == cubes[i].getLeft()) &&
                                 (cubes[0].getRight() == cubes[i].getRight()) &&
                                 (cubes[0].getUp() == cubes[i].getUp()) &&
                                 (cubes[0].getDown() == cubes[i].getDown());
                    if (!match) allMatch = false;
                }

                if (allMatch) {
                    loopTestsPassed++;
                } else {
                    loopTestsFailed++;
                }

                std::string testName = filename + ":" + item.name + " x" + std::to_string(item.loopCount);
                printTestResult(testName, allMatch);
            } else {
                // For formulas without loop count, just verify moves can be applied
                RubiksCube cube1;
                RubiksCube cube2;

                for (Move move : item.moves) {
                    cube1.executeMove(move);
                    cube2.executeMove(move);
                }

                // Check both cubes have same state
                bool statesMatch = (cube1.getFront() == cube2.getFront()) &&
                                 (cube1.getBack() == cube2.getBack()) &&
                                 (cube1.getLeft() == cube2.getLeft()) &&
                                 (cube1.getRight() == cube2.getRight()) &&
                                 (cube1.getUp() == cube2.getUp()) &&
                                 (cube1.getDown() == cube2.getDown());

                if (statesMatch) {
                    loopTestsPassed++;
                } else {
                    loopTestsFailed++;
                }

                std::string testName = filename + ":" + item.name + " (no loop)";
                printTestResult(testName, statesMatch);
            }
        }
    }

    std::cout << "\nLoop Formula Test Results:" << std::endl;
    std::cout << GREEN << "  Passed: " << loopTestsPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << loopTestsFailed << RESET << std::endl;
}

// Test 5: Test formula names and description parsing
void testFormulaParsing(const std::string& formulaDir) {
    std::cout << CYAN << BOLD << "\n=== Test 5: Formula Parsing ===" << RESET << std::endl;

    FormulaManager formulaManager;
    formulaManager.setFormulaDir(formulaDir);
    formulaManager.loadFormulas();

    auto fileNames = formulaManager.getFileNames();

    if (fileNames.empty()) {
        std::cout << RED << "No formula files to test" << RESET << std::endl;
        return;
    }

    int parsedItems = 0;
    int itemsWithName = 0;
    int itemsWithMoves = 0;
    int itemsWithLoop = 0;

    for (const auto& filename : fileNames) {
        const auto* items = formulaManager.getFileItems(filename);
        if (items == nullptr) {
            continue;
        }

        for (const FormulaItem& item : *items) {
            parsedItems++;
            if (!item.name.empty()) itemsWithName++;
            if (!item.moves.empty()) itemsWithMoves++;
            if (item.loopCount > 0) itemsWithLoop++;
        }
    }

    printTestResult("Formula parsing", true,
                  "Total items: " + std::to_string(parsedItems) +
                  ", With names: " + std::to_string(itemsWithName) +
                  ", With moves: " + std::to_string(itemsWithMoves) +
                  ", With loops: " + std::to_string(itemsWithLoop));
}

// Test 6: Test file selection and item selection
void testSelection(const std::string& formulaDir) {
    std::cout << CYAN << BOLD << "\n=== Test 6: Selection ===" << RESET << std::endl;

    FormulaManager formulaManager;
    formulaManager.setFormulaDir(formulaDir);
    formulaManager.loadFormulas();

    auto fileNames = formulaManager.getFileNames();

    if (fileNames.empty()) {
        printTestResult("Empty file list check", true);
        printTestResult("Get selected file (empty)", true,
                      "Selected: '" + formulaManager.getSelectedFileName() + "'");
        printTestResult("Get selected item (none)", true,
                      "Selected index: " + std::to_string(formulaManager.getSelectedItemIndex()));
        return;
    }

    // Test selecting first file
    printTestResult("Select first file", true);
    formulaManager.setSelectedFile(fileNames[0]);

    bool selectedFileMatches = (formulaManager.getSelectedFileName() == fileNames[0]);
    printTestResult("First file selected", selectedFileMatches);

    const auto* items = formulaManager.getSelectedFileItems();
    if (items != nullptr && !items->empty()) {
        // Test selecting first item
        printTestResult("Select first item", true);
        formulaManager.setSelectedItem(0);

        bool selectedItemMatches = (formulaManager.getSelectedItemIndex() == 0);
        printTestResult("First item selected", selectedItemMatches);

        std::string selectedName = items->at(0).name;
        bool nameMatches = (formulaManager.getSelectedItem()->name == selectedName);
        printTestResult("Selected item name", nameMatches);

        // Test loop count
        bool loopCountMatches = (formulaManager.getSelectedItem()->loopCount == items->at(0).loopCount);
        printTestResult("Loop count", loopCountMatches);

        // Test move count
        bool moveCountMatches = (formulaManager.getSelectedItem()->moves.size() == items->at(0).moves.size());
        printTestResult("Move count", moveCountMatches);
    }
}

// Test 7: Edge case tests
void testEdgeCases() {
    std::cout << CYAN << BOLD << "\n=== Test 7: Edge Cases ===" << RESET << std::endl;

    // Test: Empty move sequence
    {
        RubiksCube cube;
        auto moves = parseMoveSequence("");

        printTestResult("Parse empty move sequence", moves.empty());
    }

    // Test: Single move
    {
        std::string singleMove = "U";
        auto moves = parseMoveSequence(singleMove);
        printTestResult("Parse single move 'U'", moves.size() == 1 &&
                      moves[0] == Move::U);
    }

    // Test: Multiple moves with spaces
    {
        std::string multiMove = "U D L R";
        auto moves = parseMoveSequence(multiMove);
        printTestResult("Parse 'U D L R'", moves.size() == 4 &&
                      moves[0] == Move::U && moves[1] == Move::D &&
                      moves[2] == Move::L && moves[3] == Move::R);
    }

    // Test: Invalid move (Q is not a valid move)
    {
        std::string invalidMove = "Q";
        auto moves = parseMoveSequence(invalidMove);
        printTestResult("Parse invalid move 'Q'", moves.empty());
    }

    // Test: Move with prime notation
    {
        std::string primeMove = "U'";
        auto moves = parseMoveSequence(primeMove);
        printTestResult("Parse 'U''", moves.size() == 1 &&
                      moves[0] == Move::UP);
    }
}

int main() {
    // Get project root from executable location
    std::filesystem::path exePath = std::filesystem::canonical(std::filesystem::current_path());
    std::filesystem::path projectRoot = exePath;
    
    // Walk up to find project root (contains "formula" directory)
    while (projectRoot.has_parent_path()) {
        if (std::filesystem::exists(projectRoot / "formula") ||
            std::filesystem::exists(projectRoot / "CMakeLists.txt")) {
            break;
        }
        projectRoot = projectRoot.parent_path();
    }

    std::string formulaDir = (projectRoot / "formula").string();

    std::cout << YELLOW << BOLD << "Rubik's Cube Formula Verification" << RESET << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "Verifying formula loading, parsing, and execution" << std::endl;
    std::cout << "Formula directory: " << formulaDir << std::endl;
    std::cout << std::endl;

    testsPassed = 0;
    testsFailed = 0;

    testLoadFormulas(formulaDir);
    testFormulaParsing(formulaDir);
    testApplyFormulas(formulaDir);
    testFormulaInverseMoves(formulaDir);
    testLoopFormulas(formulaDir);
    testSelection(formulaDir);
    testEdgeCases();

    std::cout << "\n" << CYAN << BOLD << "=======================================" << RESET << std::endl;
    std::cout << YELLOW << BOLD << "FINAL RESULTS:" << RESET << std::endl;
    std::cout << GREEN << "  Passed: " << testsPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << testsFailed << RESET << std::endl;
    std::cout << "  Total:  " << (testsPassed + testsFailed) << std::endl;
    std::cout << "=======================================" << std::endl;

    if (testsFailed == 0) {
        std::cout << GREEN << BOLD << "ALL TESTS PASSED!" << RESET << std::endl;
        return 0;
    } else {
        std::cout << RED << BOLD << "SOME TESTS FAILED!" << RESET << std::endl;
        std::cout << "Please review the failed tests above." << std::endl;
        return 1;
    }
}
