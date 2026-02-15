#include "src/cube.h"
#include <iostream>

int main() {
    RubiksCube cube;

    std::cout << "=== TEST 1: Single move undo ===" << std::endl;
    cube.executeMove(Move::U);
    std::cout << "After U - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    std::cout << "History: ";
    for (const auto& move : cube.getMoveHistory()) {
        std::cout << moveToString(move) << " ";
    }
    std::cout << std::endl;

    cube.dump();

    std::cout << "\nUndoing U'" << std::endl;
    cube.undo();
    std::cout << "After undo - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    std::cout << "History size: " << cube.getMoveHistory().size() << std::endl;

    cube.dump();

    std::cout << "\n=== TEST 2: Scramble of 1 move ===" << std::endl;
    RubiksCube cube2;
    cube2.scramble(1);
    std::cout << "After scramble(1) - Solved: " << (cube2.isSolved() ? "YES" : "NO") << std::endl;
    std::cout << "History: ";
    for (const auto& move : cube2.getMoveHistory()) {
        std::cout << moveToString(move) << " ";
    }
    std::cout << std::endl;

    cube2.dump();

    std::cout << "\nUndoing" << std::endl;
    cube2.undo();
    std::cout << "After undo - Solved: " << (cube2.isSolved() ? "YES" : "NO") << std::endl;

    cube2.dump();

    return 0;
}
