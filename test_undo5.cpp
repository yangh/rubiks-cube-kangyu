#include "src/cube.h"
#include <iostream>

int main() {
    RubiksCube cube;

    std::cout << "=== STEP BY STEP TEST ===" << std::endl;

    // Step 1: F'
    std::cout << "\n1. Executing F'" << std::endl;
    cube.executeMove(Move::FP);
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    // Step 2: Undo F'
    std::cout << "2. Undoing F'" << std::endl;
    cube.undo();
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    if (!cube.isSolved()) {
        cube.dump();
    }

    // Step 3: L'
    std::cout << "\n3. Executing L'" << std::endl;
    cube.executeMove(Move::LP);
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    // Step 4: Undo L'
    std::cout << "4. Undoing L'" << std::endl;
    cube.undo();
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    if (!cube.isSolved()) {
        cube.dump();
    }

    // Step 5: U
    std::cout << "\n5. Executing U" << std::endl;
    cube.executeMove(Move::U);
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    // Step 6: Undo U
    std::cout << "6. Undoing U" << std::endl;
    cube.undo();
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    if (!cube.isSolved()) {
        cube.dump();
    }

    // Step 7: D'
    std::cout << "\n7. Executing D'" << std::endl;
    cube.executeMove(Move::DP);
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    // Step 8: Undo D'
    std::cout << "8. Undoing D'" << std::endl;
    cube.undo();
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    if (!cube.isSolved()) {
        cube.dump();
    }

    // Step 9: R
    std::cout << "\n9. Executing R" << std::endl;
    cube.executeMove(Move::R);
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;

    // Step 10: Undo R
    std::cout << "10. Undoing R" << std::endl;
    cube.undo();
    std::cout << "Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    if (!cube.isSolved()) {
        cube.dump();
    }

    return 0;
}
