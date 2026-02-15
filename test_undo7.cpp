#include "src/cube.h"
#include <iostream>

void checkSolved(const RubiksCube& cube, const std::string& step) {
    bool solved = cube.isSolved();
    std::cout << step << " - Solved: " << (solved ? "YES" : "NO");
    if (!solved) {
        std::cout << " *** BUG ***";
    }
    std::cout << std::endl;
}

int main() {
    RubiksCube cube;

    checkSolved(cube, "Initial");

    std::cout << "\n=== EXECUTING F' L' U D' R ===" << std::endl;
    cube.executeMove(Move::FP);
    checkSolved(cube, "After F'");
    cube.executeMove(Move::LP);
    checkSolved(cube, "After L'");
    cube.executeMove(Move::U);
    checkSolved(cube, "After U");
    cube.executeMove(Move::DP);
    checkSolved(cube, "After D'");
    cube.executeMove(Move::R);
    checkSolved(cube, "After R");

    std::cout << "\n=== UNDOING ===" << std::endl;
    cube.undo();
    checkSolved(cube, "Undo R'");
    cube.undo();
    checkSolved(cube, "Undo D");
    cube.undo();
    checkSolved(cube, "Undo U'");
    cube.undo();
    checkSolved(cube, "Undo L");
    cube.undo();
    checkSolved(cube, "Undo F");

    std::cout << "\n=== FINAL STATE ===" << std::endl;
    checkSolved(cube, "Final");

    return 0;
}
