#include "../src/cube.h"
#include <iostream>
#include <string>

const std::string RESET = "\033[0m";
const std::string GREEN = "\033[32m";
const std::string RED = "\033[31m";
const std::string CYAN = "\033[36m";
const std::string BOLD = "\033[1m";

int testsPassed = 0;
int testsFailed = 0;

void assertTest(const std::string& name, bool condition, const std::string& details = "") {
    if (condition) {
        std::cout << GREEN << "[PASS] " << name << RESET << std::endl;
        testsPassed++;
    } else {
        std::cout << RED << "[FAIL] " << name << RESET;
        if (!details.empty()) std::cout << " - " << details;
        std::cout << std::endl;
        testsFailed++;
    }
}

void testBasicUndoRedo() {
    std::cout << CYAN << BOLD << "\n=== Test 1: Basic Undo/Redo ===" << RESET << std::endl;

    RubiksCube cube;
    assertTest("New cube cannot undo", !cube.canUndo());
    assertTest("New cube cannot redo", !cube.canRedo());

    cube.executeMove(Move::U);
    assertTest("After U move, can undo", cube.canUndo());
    assertTest("After U move, cannot redo", !cube.canRedo());
    assertTest("Last move is U", cube.getLastMove() == Move::U);

    auto stateAfterU = cube.getFront();

    cube.executeMove(getInverseMove(Move::U), false);
    cube.undo();
    assertTest("After undo (inverse applied), cannot undo", !cube.canUndo());
    assertTest("After undo, can redo", cube.canRedo());
    assertTest("After undo, cube is solved", cube.isSolved());

    cube.executeMove(Move::U, false);
    cube.redo();
    assertTest("After redo, cannot redo", !cube.canRedo());
    assertTest("After redo, front matches original", cube.getFront() == stateAfterU);
}

void testMultipleUndoRedo() {
    std::cout << CYAN << BOLD << "\n=== Test 2: Multiple Undo/Redo ===" << RESET << std::endl;

    RubiksCube cube;
    std::vector<Move> moves = {Move::R, Move::U, Move::F, Move::L};

    for (Move m : moves) cube.executeMove(m);
    assertTest("After 4 moves, history size is 4",
               static_cast<int>(cube.getMoveHistory().size()) == 4);

    auto stateAfterMoves = cube.getFront();

    cube.executeMove(getInverseMove(Move::L), false);
    cube.undo();
    cube.executeMove(getInverseMove(Move::F), false);
    cube.undo();
    assertTest("After 2 undos, history size is 2",
               static_cast<int>(cube.getMoveHistory().size()) == 2);

    cube.executeMove(Move::F, false);
    cube.redo();
    cube.executeMove(Move::L, false);
    cube.redo();
    assertTest("After 2 redos, history size is 4",
               static_cast<int>(cube.getMoveHistory().size()) == 4);
    assertTest("After 2 redos, front matches original", cube.getFront() == stateAfterMoves);

    for (int i = 3; i >= 0; i--) {
        cube.executeMove(getInverseMove(moves[i]), false);
        cube.undo();
    }
    assertTest("After 4 undos, cannot undo", !cube.canUndo());
    assertTest("After 4 undos, cube is solved", cube.isSolved());

    for (int i = 0; i < 4; i++) {
        cube.executeMove(moves[i], false);
        cube.redo();
    }
    assertTest("After 4 redos, cannot redo", !cube.canRedo());
}

void testUndoRedoClearsRedoHistory() {
    std::cout << CYAN << BOLD << "\n=== Test 3: New Move Clears Redo ===" << RESET << std::endl;

    RubiksCube cube;
    cube.executeMove(Move::U);
    cube.executeMove(Move::R);
    assertTest("After 2 moves, can undo", cube.canUndo());
    assertTest("After 2 moves, cannot redo", !cube.canRedo());

    cube.undo();
    assertTest("After 1 undo, can redo", cube.canRedo());

    cube.executeMove(Move::F);
    assertTest("After new move, cannot redo (redo cleared)", !cube.canRedo());
    assertTest("After new move, history size is 2",
               static_cast<int>(cube.getMoveHistory().size()) == 2);
}

void testRecordHistoryFalse() {
    std::cout << CYAN << BOLD << "\n=== Test 4: recordHistory=false ===" << RESET << std::endl;

    RubiksCube cube;

    cube.executeMove(Move::U, false);
    assertTest("Move with recordHistory=false, cannot undo", !cube.canUndo());
    assertTest("Move with recordHistory=false, cube not solved", !cube.isSolved());

    cube.executeMove(Move::R, false);
    assertTest("Two moves with recordHistory=false, cannot undo", !cube.canUndo());

    cube.executeMove(Move::U);
    assertTest("Move with recordHistory=true, can undo", cube.canUndo());
    assertTest("History size is 1", static_cast<int>(cube.getMoveHistory().size()) == 1);
    assertTest("Last move is U", cube.getLastMove() == Move::U);

    cube.undo();
    assertTest("After undo, cannot undo", !cube.canUndo());
}

void testMixedRecordHistory() {
    std::cout << CYAN << BOLD << "\n=== Test 5: Mixed recordHistory ===" << RESET << std::endl;

    RubiksCube cube;
    cube.executeMove(Move::U);
    cube.executeMove(Move::R, false);
    cube.executeMove(Move::F);

    assertTest("History size is 2 (R not recorded)",
               static_cast<int>(cube.getMoveHistory().size()) == 2);

    cube.undo();
    assertTest("After undo, history size is 1",
               static_cast<int>(cube.getMoveHistory().size()) == 1);
    assertTest("Last move is U", cube.getLastMove() == Move::U);
}

void testRedoHistory() {
    std::cout << CYAN << BOLD << "\n=== Test 6: Redo History ===" << RESET << std::endl;

    RubiksCube cube;
    cube.executeMove(Move::U);
    cube.executeMove(Move::R);
    cube.undo();

    assertTest("Can redo after undo", cube.canRedo());
    assertTest("Last redo is R", cube.getLastRedo() == Move::R);
    assertTest("Redo history size is 1",
               static_cast<int>(cube.getRedoHistory().size()) == 1);

    cube.redo();
    assertTest("After redo, cannot redo", !cube.canRedo());
    assertTest("Redo history is empty",
               static_cast<int>(cube.getRedoHistory().size()) == 0);
}

void testUndoRedoComplexSequence() {
    std::cout << CYAN << BOLD << "\n=== Test 7: Complex Sequence ===" << RESET << std::endl;

    RubiksCube cube;
    std::vector<Move> moves = {Move::U, Move::R, Move::F, Move::D, Move::L};
    for (Move m : moves) {
        cube.executeMove(m);
    }

    auto fullState = cube.getFront();

    for (int i = 2; i >= 0; i--) {
        cube.executeMove(getInverseMove(moves[i]), false);
        cube.undo();
    }
    for (int i = 0; i < 3; i++) {
        cube.executeMove(moves[i], false);
        cube.redo();
    }
    assertTest("After undo 3 + redo 3, front matches", cube.getFront() == fullState);

    cube.undo();
    cube.executeMove(Move::B);
    assertTest("After undo + new move, redo cleared", !cube.canRedo());
    assertTest("History is [U, R, F, D, B] (size 5)",
               static_cast<int>(cube.getMoveHistory().size()) == 5);
}

int main() {
    std::cout << CYAN << BOLD << "Undo/Redo Tests" << RESET << std::endl;
    std::cout << "==================" << std::endl;

    testBasicUndoRedo();
    testMultipleUndoRedo();
    testUndoRedoClearsRedoHistory();
    testRecordHistoryFalse();
    testMixedRecordHistory();
    testRedoHistory();
    testUndoRedoComplexSequence();

    std::cout << "\n" << CYAN << BOLD << "==================" << RESET << std::endl;
    std::cout << GREEN << "  Passed: " << testsPassed << RESET << std::endl;
    std::cout << RED << "  Failed: " << testsFailed << RESET << std::endl;

    if (testsFailed == 0) {
        std::cout << GREEN << BOLD << "ALL TESTS PASSED!" << RESET << std::endl;
        return 0;
    }
    return 1;
}
