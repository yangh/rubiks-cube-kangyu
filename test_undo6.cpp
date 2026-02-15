#include "src/cube.h"
#include <iostream>

int main() {
    RubiksCube cube;

    std::cout << "=== TEST: Multiple moves before undo ===" << std::endl;

    // Execute F' L'
    cube.executeMove(Move::FP);
    cube.executeMove(Move::LP);

    std::cout << "After F' L' - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    // Undo L'
    std::cout << "\n1. Undoing L'" << std::endl;
    cube.undo();
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    if (!cube.isSolved()) {
        cube.dump();
    }

    // Undo F'
    std::cout << "\n2. Undoing F'" << std::endl;
    cube.undo();
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    if (!cube.isSolved()) {
        cube.dump();
    }

    return 0;
}
