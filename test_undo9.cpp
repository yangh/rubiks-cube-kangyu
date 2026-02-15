#include "src/cube.h"
#include <iostream>

void testMoveAndUndo(Move move, const std::string& name) {
    std::cout << "\n=== TEST: " << name << " then undo ===" << std::endl;
    RubiksCube cube;
    cube.executeMove(move);
    std::cout << "After " << name << " - Solved: " << (cube.isSolved() ? "YES" : "NO") << std::endl;
    cube.undo();
    std::cout << "After undo - Solved: " << (cube.isSolved() ? "YES" : "NO");
    if (!cube.isSolved()) {
        std::cout << " *** BUG ***";
        cube.dump();
    }
    std::cout << std::endl;
}

int main() {
    testMoveAndUndo(Move::U, "U");
    testMoveAndUndo(Move::UP, "U'");
    testMoveAndUndo(Move::D, "D");
    testMoveAndUndo(Move::DP, "D'");
    testMoveAndUndo(Move::L, "L");
    testMoveAndUndo(Move::LP, "L'");
    testMoveAndUndo(Move::R, "R");
    testMoveAndUndo(Move::RP, "R'");
    testMoveAndUndo(Move::F, "F");
    testMoveAndUndo(Move::FP, "F'");
    testMoveAndUndo(Move::B, "B");
    testMoveAndUndo(Move::BP, "B'");
    testMoveAndUndo(Move::M, "M");
    testMoveAndUndo(Move::MP, "M'");
    testMoveAndUndo(Move::E, "E");
    testMoveAndUndo(Move::EP, "E'");
    testMoveAndUndo(Move::S, "S");
    testMoveAndUndo(Move::SP, "S'");

    return 0;
}
