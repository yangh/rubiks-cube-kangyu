#include "cube.h"
#include <iostream>

FaceColor fillFaceColor(Color color) {
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

void RubiksCube::undo() {
    if (moveHistory_.empty()) {
        return;
    }
    Move lastMove = moveHistory_.back();
    moveHistory_.pop_back();
    redoHistory_.push_back(lastMove);
}

void RubiksCube::redo() {
    if (redoHistory_.empty()) {
        return;
    }
    Move moveToRedo = redoHistory_.back();
    redoHistory_.pop_back();
    moveHistory_.push_back(moveToRedo);
}

void RubiksCube::reset() {
    front_ = fillFaceColor(Color::GREEN);
    back_  = fillFaceColor(Color::BLUE);
    left_  = fillFaceColor(Color::ORANGE);
    right_ = fillFaceColor(Color::RED);
    up_    = fillFaceColor(Color::WHITE);
    down_  = fillFaceColor(Color::YELLOW);
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

void RubiksCube::dump() const {
    auto printRow = [](const FaceColor& face, int row) {
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

// Inline helpers replacing old macros — type-safe and debuggable

// Copy a horizontal row (3 cells) from src to dst: indices [row*3, row*3+1, row*3+2]
static inline void shiftRow(std::array<Color, 9>& dst, const std::array<Color, 9>& src, int row) {
    int base = row * 3;
    dst[base]     = src[base];
    dst[base + 1] = src[base + 1];
    dst[base + 2] = src[base + 2];
}

// Copy a vertical column (3 cells) from src to dst: indices [0+col, 3+col, 6+col]
static inline void shiftCol(std::array<Color, 9>& dst, const std::array<Color, 9>& src, int col) {
    dst[col]     = src[col];
    dst[3 + col] = src[3 + col];
    dst[6 + col] = src[6 + col];
}

// Copy a vertical column from src to dst, but reverse the order (back face mirroring)
// dst[0+col] = src[8-col], dst[3+col] = src[5-col], dst[6+col] = src[2-col]
static inline void shiftColFromBack(std::array<Color, 9>& dst, const std::array<Color, 9>& src, int col) {
    dst[col]     = src[8 - col];
    dst[3 + col] = src[5 - col];
    dst[6 + col] = src[2 - col];
}

// Copy a vertical column to dst in reversed order (back face mirroring)
// dst[8-col] = src[0+col], dst[5-col] = src[3+col], dst[2-col] = src[6+col]
static inline void shiftColToBack(std::array<Color, 9>& dst, const std::array<Color, 9>& src, int col) {
    dst[8 - col] = src[col];
    dst[5 - col] = src[3 + col];
    dst[2 - col] = src[6 + col];
}

void RubiksCube::rotateRowX(bool prime, int row = 0) {
    FaceColor temp = front_;

    if (prime) {
        shiftRow(front_, left_, row);
        shiftRow(left_,  back_, row);
        shiftRow(back_, right_, row);
        shiftRow(right_, temp, row);
    } else {
        shiftRow(front_, right_, row);
        shiftRow(right_, back_, row);
        shiftRow(back_, left_, row);
        shiftRow(left_,  temp, row);
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
    FaceColor temp = up_;

    if (prime) {
        shiftColFromBack(up_, back_, col);
        shiftColToBack(back_, down_, col);
        shiftCol(down_, front_, col);
        shiftCol(front_, temp, col);
    } else {
        shiftCol(up_, front_, col);
        shiftCol(front_, down_, col);
        shiftColFromBack(down_, back_, col);
        shiftColToBack(back_, temp, col);
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

    FaceColor temp = up_;
    if (prime) {
        up_   [6] = right_[0];  up_   [7] = right_[3];  up_   [8] = right_[6];
        right_[0] = down_ [2];  right_[3] = down_ [1];  right_[6] = down_ [0];
        down_ [0] = left_ [2];  down_ [1] = left_ [5];  down_ [2] = left_ [8];
        left_ [2] = temp  [8];  left_ [5] = temp  [7];  left_ [8] = temp  [6];
    } else {
        up_   [6] = left_ [8];  up_   [7] = left_ [5];  up_   [8] = left_ [2];
        left_ [2] = down_ [0];  left_ [5] = down_ [1];  left_ [8] = down_ [2];
        down_ [0] = right_[6];  down_ [1] = right_[3];  down_ [2] = right_[0];
        right_[0] = temp  [6];  right_[3] = temp  [7];  right_[6] = temp  [8];
    }
}

void RubiksCube::rotateBack(bool prime) {
    rotateFaceClockwise(back_, prime);

    FaceColor temp = up_;
    if (prime) {
        up_   [0] = left_ [6];  up_   [1] = left_ [3];  up_   [2] = left_ [0];
        left_ [0] = down_ [6];  left_ [3] = down_ [7];  left_ [6] = down_ [8];
        down_ [6] = right_[8];  down_ [7] = right_[5];  down_ [8] = right_[2];
        right_[2] = temp  [0];  right_[5] = temp  [1];  right_[8] = temp  [2];
    } else {
        up_   [0] = right_[2];  up_   [1] = right_[5];  up_   [2] = right_[8];
        right_[2] = down_ [8];  right_[5] = down_ [7];  right_[8] = down_ [6];
        down_ [6] = left_ [0];  down_ [7] = left_ [3];  down_ [8] = left_ [6];
        left_ [0] = temp  [2];  left_ [3] = temp  [1];  left_ [6] = temp  [0];
    }
}

void RubiksCube::rotateStanding(bool prime) {
    FaceColor temp = up_;
    if (prime) {
        up_   [5] = right_[7];  up_   [4] = right_[4];  up_   [3] = right_[1];
        right_[7] = down_ [3];  right_[4] = down_ [4];  right_[1] = down_ [5];
        down_ [3] = left_ [1];  down_ [4] = left_ [4];  down_ [5] = left_ [7];
        left_ [1] = temp  [5];  left_ [4] = temp  [4];  left_ [7] = temp  [3];
    } else {
        up_   [3] = left_ [7];  up_   [4] = left_ [4];  up_   [5] = left_ [1];
        left_ [1] = down_ [3];  left_ [4] = down_ [4];  left_ [7] = down_ [5];
        down_ [5] = right_[1];  down_ [4] = right_[4];  down_ [3] = right_[7];
        right_[1] = temp  [3];  right_[4] = temp  [4];  right_[7] = temp  [5];
    }
}

void RubiksCube::rotateFaceClockwise(FaceColor& face, bool prime) {
    FaceColor temp = face;
    if (prime) {
        face[0] = temp[2]; face[1] = temp[5]; face[2] = temp[8];
        face[3] = temp[1]; face[4] = temp[4]; face[5] = temp[7];
        face[6] = temp[0]; face[7] = temp[3]; face[8] = temp[6];
    } else {
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
    auto checkFace = [](const FaceColor& face) -> bool {
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
    struct EdgeDef { const FaceColor* f1; int i1; const FaceColor* f2; int i2; const char* name; };
    EdgeDef edges[12] = {
        {&up_,    7, &front_, 1, "UF"}, {&up_,    3, &left_,  1, "UL"},
        {&up_,    5, &right_, 1, "UR"}, {&up_,    1, &back_,  1, "UB"},
        {&down_,  1, &front_, 7, "DF"}, {&down_,  3, &left_,  7, "DL"},
        {&down_,  5, &right_, 7, "DR"}, {&down_,  7, &back_,  7, "DB"},
        {&front_, 3, &left_,  5, "FL"}, {&front_, 5, &right_, 3, "FR"},
        {&back_,  3, &right_, 5, "BL"}, {&back_,  5, &left_,  3, "BR"}
    };
    
    for (const auto& e : edges) {
        if (isOppositeColor((*e.f1)[e.i1], (*e.f2)[e.i2])) {
            return "Edge " + std::string(e.name) + " has opposite colors: " +
                   colorToString((*e.f1)[e.i1]) + "-" + colorToString((*e.f2)[e.i2]);
        }
    }
    
    struct CornerDef { const FaceColor* f1; int i1; const FaceColor* f2; int i2; const FaceColor* f3; int i3; const char* name; };
    CornerDef corners[8] = {
        {&up_,   6, &front_, 0, &left_,  2, "UFL"},
        {&up_,   8, &front_, 2, &right_, 0, "UFR"},
        {&up_,   0, &back_,  2, &left_,  0, "UBL"},
        {&up_,   2, &back_,  0, &right_, 2, "UBR"},
        {&down_, 0, &front_, 6, &left_,  8, "DFL"},
        {&down_, 2, &front_, 8, &right_, 6, "DFR"},
        {&down_, 6, &back_,  8, &left_,  6, "DBL"},
        {&down_, 8, &back_,  6, &right_, 8, "DBR"}
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
