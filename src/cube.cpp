#include "cube.h"
#include "move.h"
#include <algorithm>
#include <iostream>
#include <vector>

std::array<Color, 9> RubiksCube::fillColor(Color color) {
    return {color, color, color, color, color, color, color, color, color};
}

void RubiksCube::executeMove(Move move) {
    executeMove(move, true);
}

void RubiksCube::executeMove(Move move, bool recordHistory) {
    if (recordHistory) {
        redoHistory_.clear();
        moveHistory_.push_back(move);
    }

    switch (move) {
        case Move::U:  rotateUp(false); break;
        case Move::UP: rotateUp(true); break;
        case Move::D:  rotateDown(false); break;
        case Move::DP: rotateDown(true); break;
        case Move::L:  rotateLeft(false); break;
        case Move::LP: rotateLeft(true); break;
        case Move::R:  rotateRight(false); break;
        case Move::RP: rotateRight(true); break;
        case Move::F:  rotateFront(false); break;
        case Move::FP: rotateFront(true); break;
        case Move::B:  rotateBack(false); break;
        case Move::BP: rotateBack(true); break;
        case Move::M:  rotateMiddle(false); break;
        case Move::MP: rotateMiddle(true); break;
        case Move::E:  rotateEquator(false); break;
        case Move::EP: rotateEquator(true); break;
        case Move::S:  rotateStanding(false); break;
        case Move::SP: rotateStanding(true); break;
        case Move::U2: rotateUp(false); rotateUp(false); break;
        case Move::D2: rotateDown(false); rotateDown(false); break;
        case Move::L2: rotateLeft(false); rotateLeft(false); break;
        case Move::R2: rotateRight(false); rotateRight(false); break;
        case Move::F2: rotateFront(false); rotateFront(false); break;
        case Move::B2: rotateBack(false); rotateBack(false); break;
        case Move::M2: rotateMiddle(false); rotateMiddle(false); break;
        case Move::E2: rotateEquator(false); rotateEquator(false); break;
        case Move::S2: rotateStanding(false); rotateStanding(false); break;
        case Move::X:  rotateX(false); break;
        case Move::XP: rotateX(true); break;
        case Move::Y:  rotateY(false); break;
        case Move::YP: rotateY(true); break;
        case Move::Z:  rotateZ(false); break;
        case Move::ZP: rotateZ(true); break;
        case Move::X2: rotateX(false); rotateX(false); break;
        case Move::Y2: rotateY(false); rotateY(false); break;
        case Move::Z2: rotateZ(false); rotateZ(false); break;
    }
}

void RubiksCube::popMoveHistory() {
    if (!moveHistory_.empty()) {
        moveHistory_.pop_back();
    }
}

void RubiksCube::pushToMoveHistory(Move move) {
    moveHistory_.push_back(move);
}

void RubiksCube::popRedoHistory() {
    if (!redoHistory_.empty()) {
        redoHistory_.pop_back();
    }
}

void RubiksCube::pushToRedoHistory(Move move) {
    redoHistory_.push_back(move);
}

void RubiksCube::reset() {
    front_ = fillColor(Color::GREEN);
    back_  = fillColor(Color::BLUE);
    left_  = fillColor(Color::ORANGE);
    right_ = fillColor(Color::RED);
    up_    = fillColor(Color::WHITE);
    down_  = fillColor(Color::YELLOW);
    moveHistory_.clear();
    redoHistory_.clear();
}

std::vector<Move> RubiksCube::scramble(int numMoves) {
    std::vector<Move> scrambleMoves = generateRandomMoves(numMoves);
    for (Move m : scrambleMoves) {
        executeMove(m);
    }
    return scrambleMoves;
}

void RubiksCube::undo() {
    if (moveHistory_.empty()) {
        return;
    }

    Move lastMove = moveHistory_.back();
    moveHistory_.pop_back();
    redoHistory_.push_back(lastMove);
    Move inverseMove = getInverseMove(lastMove);
    executeMove(inverseMove, false);
}

Move RubiksCube::getInverseMoveForUndo() const {
    if (moveHistory_.empty()) {
        return Move::U;
    }
    return getInverseMove(moveHistory_.back());
}

Move RubiksCube::getMoveForRedo() const {
    if (redoHistory_.empty()) {
        return Move::U;
    }
    return redoHistory_.back();
}

void RubiksCube::redo() {
    if (redoHistory_.empty()) {
        return;
    }

    Move moveToRedo = redoHistory_.back();
    redoHistory_.pop_back();
    moveHistory_.push_back(moveToRedo);
    executeMove(moveToRedo, false);
}

void RubiksCube::dump() const {
    auto printRow = [](const std::array<Color, 9>& face, int row) {
        for (int col = 0; col < 3; ++col) {
            std::cout << colorToString(face[row * 3 + col]) << " ";
        }
    };

    for (int row = 0; row < 3; ++row) {
        std::cout << "        ";
        printRow(up_, row);
        std::cout << std::endl;
    }
    std::cout << std::endl;

    for (int row = 0; row < 3; ++row) {
        printRow(left_, row);
        std::cout << "  ";
        printRow(front_, row);
        std::cout << "  ";
        printRow(right_, row);
        std::cout << "  ";
        printRow(back_, row);
        std::cout << std::endl;
    }
    std::cout << std::endl;

    for (int row = 0; row < 3; ++row) {
        std::cout << "        ";
        printRow(down_, row);
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

#define shiftLeftOnY(a, b, row) \
    a[0 + row * 3] = b[0 + row * 3]; \
    a[1 + row * 3] = b[1 + row * 3]; \
    a[2 + row * 3] = b[2 + row * 3];

#define shiftLeftOnX(a, b, col) \
    a[0 + col] = b[0 + col]; \
    a[3 + col] = b[3 + col]; \
    a[6 + col] = b[6 + col];

#define shiftLeftOnXfromBack(a, b, col) \
    a[0 + col] = b[8 - col]; \
    a[3 + col] = b[5 - col]; \
    a[6 + col] = b[2 - col];

#define shiftLeftOnXtoBack(a, b, col) \
    a[8 - col] = b[0 + col]; \
    a[5 - col] = b[3 + col]; \
    a[2 - col] = b[6 + col];

void RubiksCube::rotateRowX(bool prime, int row = 0) {
    std::array<Color, 9> temp = front_;

    if (prime) {
        shiftLeftOnY(front_, left_, row);
        shiftLeftOnY(left_,  back_, row);
        shiftLeftOnY(back_, right_, row);
        shiftLeftOnY(right_, temp, row);
    } else {
        shiftLeftOnY(front_, right_, row);
        shiftLeftOnY(right_, back_, row);
        shiftLeftOnY(back_, left_, row);
        shiftLeftOnY(left_,  temp, row);
    }
}

void RubiksCube::rotateUp(bool prime) {
    rotateFaceClockwise(up_, prime);
    rotateRowX(prime, 0);
}

void RubiksCube::rotateEquator(bool prime) {
    rotateRowX(!prime, 1);
}

void RubiksCube::rotateDown(bool prime) {
    rotateFaceClockwise(down_, prime);
    rotateRowX(!prime, 2);
}

void RubiksCube::rotateColY(bool prime, int col = 0) {
    std::array<Color, 9> temp = up_;

    if (prime) {
        shiftLeftOnXfromBack(up_, back_, col);
        shiftLeftOnXtoBack(back_, down_, col);
        shiftLeftOnX(down_, front_, col);
        shiftLeftOnX(front_, temp, col);
    } else {
        shiftLeftOnX(up_, front_, col);
        shiftLeftOnX(front_, down_, col);
        shiftLeftOnXfromBack(down_, back_, col);
        shiftLeftOnXtoBack(back_, temp, col);
    }
}

void RubiksCube::rotateLeft(bool prime) {
    rotateFaceClockwise(left_, prime);
    rotateColY(!prime, 0);
}

void RubiksCube::rotateMiddle(bool prime) {
    rotateColY(!prime, 1);
}

void RubiksCube::rotateRight(bool prime) {
    rotateFaceClockwise(right_, prime);
    rotateColY(prime, 2);
}

void RubiksCube::rotateFront(bool prime) {
    rotateFaceClockwise(front_, prime);

    if (prime) {
        std::array<Color, 9> temp = up_;
        up_[6] = right_[0]; up_[7] = right_[3]; up_[8] = right_[6];
        right_[0] = down_[2]; right_[3] = down_[1]; right_[6] = down_[0];
        down_[0] = left_[2]; down_[1] = left_[5]; down_[2] = left_[8];
        left_[2] = temp[8]; left_[5] = temp[7]; left_[8] = temp[6];
    } else {
        std::array<Color, 9> temp = up_;
        up_[6] = left_[8]; up_[7] = left_[5]; up_[8] = left_[2];
        left_[2] = down_[0]; left_[5] = down_[1]; left_[8] = down_[2];
        down_[0] = right_[6]; down_[1] = right_[3]; down_[2] = right_[0];
        right_[0] = temp[6]; right_[3] = temp[7]; right_[6] = temp[8];
    }
}

void RubiksCube::rotateBack(bool prime) {
    rotateFaceClockwise(back_, prime);

    if (prime) {
        std::array<Color, 9> temp = up_;
        up_[0] = left_[6]; up_[1] = left_[3]; up_[2] = left_[0];
        left_[0] = down_[6]; left_[3] = down_[7]; left_[6] = down_[8];
        down_[6] = right_[8]; down_[7] = right_[5]; down_[8] = right_[2];
        right_[2] = temp[0]; right_[5] = temp[1]; right_[8] = temp[2];
    } else {
        std::array<Color, 9> temp = up_;
        up_[0] = right_[2]; up_[1] = right_[5]; up_[2] = right_[8];
        right_[2] = down_[8]; right_[5] = down_[7]; right_[8] = down_[6];
        down_[6] = left_[0]; down_[7] = left_[3]; down_[8] = left_[6];
        left_[0] = temp[2]; left_[3] = temp[1]; left_[6] = temp[0];
    }
}

void RubiksCube::rotateStanding(bool prime) {
    if (prime) {
        std::array<Color, 9> temp = up_;
        up_[5] = right_[7]; up_[4] = right_[4]; up_[3] = right_[1];
        right_[7] = down_[3]; right_[4] = down_[4]; right_[1] = down_[5];
        down_[3] = left_[1]; down_[4] = left_[4]; down_[5] = left_[7];
        left_[1] = temp[5]; left_[4] = temp[4]; left_[7] = temp[3];
    } else {
        std::array<Color, 9> temp = up_;
        up_[3] = left_[7]; up_[4] = left_[4]; up_[5] = left_[1];
        left_[1] = down_[3]; left_[4] = down_[4]; left_[7] = down_[5];
        down_[5] = right_[1]; down_[4] = right_[4]; down_[3] = right_[7];
        right_[1] = temp[3]; right_[4] = temp[4]; right_[7] = temp[5];
    }
}

void RubiksCube::rotateFaceClockwise(std::array<Color, 9>& face, bool prime) {
    if (prime) {
        std::array<Color, 9> temp = face;
        face[0] = temp[2]; face[1] = temp[5]; face[2] = temp[8];
        face[3] = temp[1]; face[4] = temp[4]; face[5] = temp[7];
        face[6] = temp[0]; face[7] = temp[3]; face[8] = temp[6];
    } else {
        std::array<Color, 9> temp = face;
        face[0] = temp[6]; face[1] = temp[3]; face[2] = temp[0];
        face[3] = temp[7]; face[4] = temp[4]; face[5] = temp[1];
        face[6] = temp[8]; face[7] = temp[5]; face[8] = temp[2];
    }
}

void RubiksCube::rotateX(bool prime) {
    if (prime) {
        rotateRight(true);
        rotateMiddle(false);
        rotateLeft(false);
    } else {
        rotateRight(false);
        rotateMiddle(true);
        rotateLeft(true);
    }
}

void RubiksCube::rotateY(bool prime) {
    if (prime) {
        rotateUp(true);
        rotateEquator(false);
        rotateDown(false);
    } else {
        rotateUp(false);
        rotateEquator(true);
        rotateDown(true);
    }
}

void RubiksCube::rotateZ(bool prime) {
    if (prime) {
        rotateFront(true);
        rotateStanding(true);
        rotateBack(false);
    } else {
        rotateFront(false);
        rotateStanding(false);
        rotateBack(true);
    }
}

bool RubiksCube::isSolved() const {
    auto checkFace = [](const std::array<Color, 9>& face) -> bool {
        const Color first = face[0];
        for (int i = 1; i < 9; ++i) {
            if (face[i] != first) return false;
        }
        return true;
    };

    return checkFace(front_) && checkFace(back_) &&
           checkFace(left_) && checkFace(right_) &&
           checkFace(up_) && checkFace(down_);
}

bool RubiksCube::isValidColorConfiguration() const {
    return getValidationError().empty();
}

std::string RubiksCube::getValidationError() const {
    struct EdgeDef { const std::array<Color, 9>* f1; int i1; const std::array<Color, 9>* f2; int i2; const char* name; };
    EdgeDef edges[12] = {
        {&up_, 7, &front_, 1, "UF"}, {&up_, 3, &left_, 1, "UL"},
        {&up_, 5, &right_, 1, "UR"}, {&up_, 1, &back_, 1, "UB"},
        {&down_, 1, &front_, 7, "DF"}, {&down_, 3, &left_, 7, "DL"},
        {&down_, 5, &right_, 7, "DR"}, {&down_, 7, &back_, 7, "DB"},
        {&front_, 3, &left_, 5, "FL"}, {&front_, 5, &right_, 3, "FR"},
        {&back_, 3, &right_, 5, "BL"}, {&back_, 5, &left_, 3, "BR"}
    };
    
    for (const auto& e : edges) {
        if (isOppositeColor((*e.f1)[e.i1], (*e.f2)[e.i2])) {
            return "Edge " + std::string(e.name) + " has opposite colors: " +
                   colorToString((*e.f1)[e.i1]) + "-" + colorToString((*e.f2)[e.i2]);
        }
    }
    
    struct CornerDef { const std::array<Color, 9>* f1; int i1; const std::array<Color, 9>* f2; int i2; const std::array<Color, 9>* f3; int i3; const char* name; };
    CornerDef corners[8] = {
        {&up_, 6, &front_, 0, &left_, 2, "UFL"}, {&up_, 8, &front_, 2, &right_, 0, "UFR"},
        {&up_, 0, &back_, 2, &left_, 0, "UBL"}, {&up_, 2, &back_, 0, &right_, 2, "UBR"},
        {&down_, 0, &front_, 6, &left_, 8, "DFL"}, {&down_, 2, &front_, 8, &right_, 6, "DFR"},
        {&down_, 6, &back_, 8, &left_, 6, "DBL"}, {&down_, 8, &back_, 6, &right_, 8, "DBR"}
    };
    
    for (const auto& c : corners) {
        Color c1 = (*c.f1)[c.i1], c2 = (*c.f2)[c.i2], c3 = (*c.f3)[c.i3];
        if (isOppositeColor(c1, c2) || isOppositeColor(c1, c3) || isOppositeColor(c2, c3)) {
            return "Corner " + std::string(c.name) + " has opposite colors: " +
                   colorToString(c1) + "-" + colorToString(c2) + "-" + colorToString(c3);
        }
    }
    
    return "";
}
