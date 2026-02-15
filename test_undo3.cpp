#include "src/cube.h"
#include <iostream>

int main() {
    RubiksCube cube;

    std::cout << "=== TEST: Two moves ===" << std::endl;
    cube.executeMove(Move::U);
    cube.executeMove(Move::D);

    std::cout << "After U then D - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    std::cout << "History: ";
    for (const auto& move : cube.getMoveHistory()) {
        std::cout << moveToString(move) << " ";
    }
    std::cout << std::endl;

    cube.dump();

    std::cout << "\nUndoing D'" << std::endl;
    cube.undo();
    std::cout << "After first undo - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    cube.dump();

    std::cout << "\nUndoing U'" << std::endl;
    cube.undo();
    std::cout << "After second undo - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    cube.dump();

    return 0;
}
