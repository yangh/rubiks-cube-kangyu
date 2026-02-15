#include "src/cube.h"
#include <iostream>
#include <cassert>

int main() {
    RubiksCube cube;

    // Test 1: Check initial state
    std::cout << "Test 1: Initial state" << std::endl;
    assert(cube.isSolved() == true);
    assert(cube.getMoveHistory().empty() == true);
    std::cout << "  Cube is solved: PASS" << std::endl;
    std::cout << "  History is empty: PASS" << std::endl;

    // Test 2: Execute moves and check history
    std::cout << "\nTest 2: Execute moves" << std::endl;
    cube.executeMove(Move::R);
    cube.executeMove(Move::U);
    cube.executeMove(Move::F);
    std::cout << "  Executed: R, U, F" << std::endl;

    const auto& history = cube.getMoveHistory();
    assert(history.size() == 3);
    assert(history[0] == Move::R);
    assert(history[1] == Move::U);
    assert(history[2] == Move::F);
    std::cout << "  History contains 3 moves: PASS" << std::endl;
    std::cout << "  Moves in correct order: PASS" << std::endl;

    // Test 3: Undo moves
    std::cout << "\nTest 3: Undo moves" << std::endl;
    cube.undo();
    std::cout << "  Undone F'" << std::endl;
    assert(cube.getMoveHistory().size() == 2);

    cube.undo();
    std::cout << "  Undone U'" << std::endl;
    assert(cube.getMoveHistory().size() == 1);

    cube.undo();
    std::cout << "  Undone R'" << std::endl;
    assert(cube.getMoveHistory().size() == 0);
    assert(cube.isSolved() == true);
    std::cout << "  Cube is solved after undoing all moves: PASS" << std::endl;

    // Test 4: Undo when empty
    std::cout << "\nTest 4: Undo when empty" << std::endl;
    cube.undo();
    assert(cube.getMoveHistory().size() == 0);
    std::cout << "  Undo on empty history is safe: PASS" << std::endl;

    // Test 5: Reset clears history
    std::cout << "\nTest 5: Reset clears history" << std::endl;
    cube.executeMove(Move::R);
    cube.executeMove(Move::L);
    assert(cube.getMoveHistory().size() == 2);
    std::cout << "  Added moves to history: PASS" << std::endl;

    cube.reset();
    assert(cube.isSolved() == true);
    assert(cube.getMoveHistory().empty() == true);
    std::cout << "  Reset clears history: PASS" << std::endl;

    // Test 6: Prime moves
    std::cout << "\nTest 6: Prime moves" << std::endl;
    cube.reset();
    cube.executeMove(Move::R);
    cube.executeMove(Move::RP);
    assert(cube.getMoveHistory().size() == 2);
    cube.undo();
    assert(cube.getMoveHistory().size() == 1);
    cube.undo();
    assert(cube.isSolved() == true);
    std::cout << "  Prime moves work correctly: PASS" << std::endl;

    // Test 7: Slice moves
    std::cout << "\nTest 7: Slice moves (M, E, S)" << std::endl;
    cube.reset();
    cube.executeMove(Move::M);
    cube.executeMove(Move::E);
    cube.executeMove(Move::S);
    assert(cube.getMoveHistory().size() == 3);
    cube.undo();
    cube.undo();
    cube.undo();
    assert(cube.isSolved() == true);
    std::cout << "  Slice moves work correctly: PASS" << std::endl;

    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}
