#include "src/cube.h"
#include <iostream>

int main() {
    RubiksCube cube;

    std::cout << "=== TEST: F' L' then undo L' then undo F' ===" << std::endl;
    std::cout << "Initial - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    cube.executeMove(Move::FP);
    std::cout << "After F' - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    cube.executeMove(Move::LP);
    std::cout << "After L' - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    cube.dump();

    cube.undo();
    std::cout << "\nAfter undo (L) - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    cube.dump();

    cube.undo();
    std::cout << "\nAfter undo (F) - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    if (!cube.isSolved()) {
        std::cout << "*** BUG ***\n" << std::endl;
    }
    cube.dump();

    return 0;
}
