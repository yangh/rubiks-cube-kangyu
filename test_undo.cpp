#include "src/cube.h"
#include <iostream>

int main() {
    RubiksCube cube;

    std::cout << "Initial state - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    std::cout << "Move history size: " << cube.getMoveHistory().size() << std::endl;

    // Scramble with 5 moves
    std::cout << "\n=== SCRAMBLING ===" << std::endl;
    auto scrambleMoves = cube.scramble(5);
    std::cout << "Scramble moves: ";
    for (const auto& move : scrambleMoves) {
        std::cout << moveToString(move) << " ";
    }
    std::cout << std::endl;

    std::cout << "\nAfter scramble - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    std::cout << "Move history size: " << cube.getMoveHistory().size() << std::endl;
    std::cout << "Move history: ";
    for (const auto& move : cube.getMoveHistory()) {
        std::cout << moveToString(move) << " ";
    }
    std::cout << std::endl;

    // Undo all moves
    std::cout << "\n=== UNDOING ALL MOVES ===" << std::endl;
    int undoCount = 0;
    while (!cube.getMoveHistory().empty()) {
        cube.undo();
        undoCount++;
        std::cout << "Undo #" << undoCount << " - History size: " << cube.getMoveHistory().size()
                  << " - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    }

    std::cout << "\nAfter all undos - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    std::cout << "Move history size: " << cube.getMoveHistory().size() << std::endl;

    return 0;
}
