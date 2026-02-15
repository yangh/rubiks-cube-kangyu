#include "src/cube.h"
#include <iostream>

int main() {
    RubiksCube cube;

    std::cout << "=== TEST: Exact sequence from test_undo (F' L' U D' R) ===" << std::endl;
    cube.executeMove(Move::FP);
    cube.executeMove(Move::LP);
    cube.executeMove(Move::U);
    cube.executeMove(Move::DP);
    cube.executeMove(Move::R);

    std::cout << "After F' L' U D' R - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    cube.dump();

    std::cout << "\n=== UNDOING ===" << std::endl;
    for (int i = 1; i <= 5; i++) {
        cube.undo();
        std::cout << "After undo #" << i << " - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    }

    std::cout << "\n=== FINAL STATE ===" << std::endl;
    cube.dump();

    return 0;
}
