#include "src/cube.h"
#include <iostream>

void checkState(const RubiksCube& cube, const std::string& step) {
    bool solved = cube.isSolved();
    std::cout << step << " - Solved: " << (solved ? "YES" : "NO");
    std::cout << ", History: " << cube.getMoveHistory().size();
    std::cout << std::endl;
}

int main() {
    RubiksCube cube;

    checkState(cube, "Initial");

    std::cout << "\n=== EXECUTING F' L' U D' R ===" << std::endl;
    cube.executeMove(Move::FP);
    checkState(cube, "After F'");
    cube.executeMove(Move::LP);
    checkState(cube, "After L'");
    cube.executeMove(Move::U);
    checkState(cube, "After U");
    cube.executeMove(Move::DP);
    checkState(cube, "After D'");
    cube.executeMove(Move::R);
    checkState(cube, "After R");

    std::cout << "\n=== UNDOING ===" << std::endl;
    cube.undo();
    checkState(cube, "Undo R' (was R)");
    cube.undo();
    checkState(cube, "Undo D (was D')");
    cube.undo();
    checkState(cube, "Undo U' (was U)");
    cube.undo();
    checkState(cube, "Undo L (was L')");
    cube.undo();
    checkState(cube, "Undo F (was F')");

    std::cout << "\n=== FINAL STATE ===" << std::endl;
    checkState(cube, "Final (should be solved)");

    if (!cube.isSolved()) {
        std::cout << "\n*** CUBE IS NOT SOLVED ***\n" << std::endl;
        cube.dump();
    }

    return 0;
}
