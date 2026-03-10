#include "cube.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

std::array<float, 3> colorToRgb(Color color) {
    switch (color) {
        case Color::WHITE:  return {1.0f, 1.0f, 1.0f};
        case Color::YELLOW: return {1.0f, 1.0f, 0.0f};
        case Color::RED:    return {1.0f, 0.0f, 0.0f};
        case Color::ORANGE: return {1.0f, 0.5f, 0.0f};
        case Color::GREEN:  return {0.0f, 1.0f, 0.0f};
        case Color::BLUE:   return {0.0f, 0.0f, 1.0f};
        default:            return {0.0f, 0.0f, 0.0f};
    }
}

std::string moveToString(Move move) {
    switch (move) {
        case Move::U:  return "U";
        case Move::UP: return "U'";
        case Move::D:  return "D";
        case Move::DP: return "D'";
        case Move::L:  return "L";
        case Move::LP: return "L'";
        case Move::R:  return "R";
        case Move::RP: return "R'";
        case Move::F:  return "F";
        case Move::FP: return "F'";
        case Move::B:  return "B";
        case Move::BP: return "B'";
        case Move::M:  return "M";
        case Move::MP: return "M'";
        case Move::E:  return "E";
        case Move::EP: return "E'";
        case Move::S:  return "S";
        case Move::SP: return "S'";
        case Move::X:  return "X";
        case Move::XP: return "X'";
        case Move::Y:  return "Y";
        case Move::YP: return "Y'";
        case Move::Z:  return "Z";
        case Move::ZP: return "Z'";
        // Double moves (180° rotation)
        case Move::U2: return "U2";
        case Move::D2: return "D2";
        case Move::L2: return "L2";
        case Move::R2: return "R2";
        case Move::F2: return "F2";
        case Move::B2: return "B2";
        case Move::M2: return "M2";
        case Move::E2: return "E2";
        case Move::S2: return "S2";
        case Move::X2: return "X2";
        case Move::Y2: return "Y2";
        case Move::Z2: return "Z2";
        default:       return "?";
    }
}

// Helper function to get inverse of a move
Move getInverseMove(Move move) {
    switch (move) {
        case Move::U:  return Move::UP;
        case Move::UP: return Move::U;
        case Move::D:  return Move::DP;
        case Move::DP: return Move::D;
        case Move::L:  return Move::LP;
        case Move::LP: return Move::L;
        case Move::R:  return Move::RP;
        case Move::RP: return Move::R;
        case Move::F:  return Move::FP;
        case Move::FP: return Move::F;
        case Move::B:  return Move::BP;
        case Move::BP: return Move::B;
        case Move::M:  return Move::MP;
        case Move::MP: return Move::M;
        case Move::E:  return Move::EP;
        case Move::EP: return Move::E;
        case Move::S:  return Move::SP;
        case Move::SP: return Move::S;
        case Move::X:  return Move::XP;
        case Move::XP: return Move::X;
        case Move::Y:  return Move::YP;
        case Move::YP: return Move::Y;
        case Move::Z:  return Move::ZP;
        case Move::ZP: return Move::Z;
        case Move::U2: return Move::U2;
        case Move::D2: return Move::D2;
        case Move::L2: return Move::L2;
        case Move::R2: return Move::R2;
        case Move::F2: return Move::F2;
        case Move::B2: return Move::B2;
        case Move::M2: return Move::M2;
        case Move::E2: return Move::E2;
        case Move::S2: return Move::S2;
        case Move::X2: return Move::X2;
        case Move::Y2: return Move::Y2;
        case Move::Z2: return Move::Z2;
        default: return move;
    }
}

std::string colorToString(Color color) {
    switch (color) {
        case Color::WHITE:  return "W";
        case Color::YELLOW: return "Y";
        case Color::RED:    return "R";
        case Color::ORANGE: return "O";
        case Color::GREEN:  return "G";
        case Color::BLUE:   return "B";
        default:            return "?";
    }
}

bool parseMoveString(const std::string& moveStr, Move& outMove) {
    // Trim whitespace
    std::string trimmed = moveStr;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
    if (trimmed.empty()) return false;

    // Convert to uppercase
    std::string upper = trimmed;
    for (char& c : upper) c = toupper(c);

    // Handle move with prime notation (R', U', etc.)
    if (upper.length() == 2 && upper[1] == '\'') {
        switch (upper[0]) {
            case 'U': outMove = Move::UP; return true;
            case 'D': outMove = Move::DP; return true;
            case 'L': outMove = Move::LP; return true;
            case 'R': outMove = Move::RP; return true;
            case 'F': outMove = Move::FP; return true;
            case 'B': outMove = Move::BP; return true;
            case 'M': outMove = Move::MP; return true;
            case 'E': outMove = Move::EP; return true;
            case 'S': outMove = Move::SP; return true;
            case 'X': outMove = Move::XP; return true;
            case 'Y': outMove = Move::YP; return true;
            case 'Z': outMove = Move::ZP; return true;
        }
    }
    // Handle double moves (X2 notation)
    else if (upper.length() == 2 && upper[1] == '2') {
        switch (upper[0]) {
            case 'U': outMove = Move::U2; return true;
            case 'D': outMove = Move::D2; return true;
            case 'L': outMove = Move::L2; return true;
            case 'R': outMove = Move::R2; return true;
            case 'F': outMove = Move::F2; return true;
            case 'B': outMove = Move::B2; return true;
            case 'M': outMove = Move::M2; return true;
            case 'E': outMove = Move::E2; return true;
            case 'S': outMove = Move::S2; return true;
            case 'X': outMove = Move::X2; return true;
            case 'Y': outMove = Move::Y2; return true;
            case 'Z': outMove = Move::Z2; return true;
        }
    }
    // Handle single letter moves
    else if (upper.length() == 1) {
        switch (upper[0]) {
            case 'U': outMove = Move::U; return true;
            case 'D': outMove = Move::D; return true;
            case 'L': outMove = Move::L; return true;
            case 'R': outMove = Move::R; return true;
            case 'F': outMove = Move::F; return true;
            case 'B': outMove = Move::B; return true;
            case 'M': outMove = Move::M; return true;
            case 'E': outMove = Move::E; return true;
            case 'S': outMove = Move::S; return true;
            case 'X': outMove = Move::X; return true;
            case 'Y': outMove = Move::Y; return true;
            case 'Z': outMove = Move::Z; return true;
        }
    }

    return false;
}

std::vector<Move> parseMoveSequence(const std::string& sequence) {
    std::vector<Move> moves;
    std::istringstream iss(sequence);
    std::string token;

    while (iss >> token) {
        Move move;
        if (parseMoveString(token, move)) {
            moves.push_back(move);
        }
    }

    return moves;
}

std::array<Color, 9> RubiksCube::fillColor(Color color) {
    return {color, color, color, color, color, color, color, color, color};
}

RubiksCube::RubiksCube()
    : front_(fillColor(Color::GREEN))
    , back_( fillColor(Color::BLUE))
    , left_( fillColor(Color::ORANGE))
    , right_(fillColor(Color::RED))
    , up_(   fillColor(Color::WHITE))
    , down_( fillColor(Color::YELLOW))
{
}

void RubiksCube::executeMove(Move move) {
    // Default behavior: record history
    executeMove(move, true);
}

void RubiksCube::executeMove(Move move, bool recordHistory) {
    if (recordHistory) {
        // Clear redo history when a new move is executed
        redoHistory_.clear();
        // Add move to history
        moveHistory_.push_back(move);
    }

    // Execute the move (rotation algorithm only)
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
        // Double moves (execute rotation twice)
        case Move::U2: rotateUp(false); rotateUp(false); break;
        case Move::D2: rotateDown(false); rotateDown(false); break;
        case Move::L2: rotateLeft(false); rotateLeft(false); break;
        case Move::R2: rotateRight(false); rotateRight(false); break;
        case Move::F2: rotateFront(false); rotateFront(false); break;
        case Move::B2: rotateBack(false); rotateBack(false); break;
        case Move::M2: rotateMiddle(false); rotateMiddle(false); break;
        case Move::E2: rotateEquator(false); rotateEquator(false); break;
        case Move::S2: rotateStanding(false); rotateStanding(false); break;
        // Axis rotations
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

void RubiksCube::reset() {
    front_ = fillColor(Color::GREEN);
    back_ = fillColor(Color::BLUE);
    left_ = fillColor(Color::ORANGE);
    right_ = fillColor(Color::RED);
    up_ = fillColor(Color::WHITE);
    down_ = fillColor(Color::YELLOW);
    moveHistory_.clear();
    redoHistory_.clear();
}

std::vector<Move> RubiksCube::scramble(int numMoves) {
    // Array of all 15 basic moves (U, U', D, D', L, L', R, R', F, F', B, B', E, S, M)
    static const Move basicMoves[] = {
        Move::U, Move::UP,
        Move::D, Move::DP,
        Move::L, Move::LP,
        Move::R, Move::RP,
        Move::F, Move::FP,
        Move::B, Move::BP,
        Move::E, Move::S, Move::M,
    };
    static const int numBasicMoves = sizeof(basicMoves)/sizeof(Move);

    std::vector<Move> scrambleMoves;
    scrambleMoves.reserve(numMoves);

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, numBasicMoves - 1);

    // Generate random sequence of moves
    for (int i = 0; i < numMoves; ++i) {
        Move randomMove = basicMoves[dis(gen)];
        scrambleMoves.push_back(randomMove);

        // Execute the move immediately (no animation for scramble)
        executeMove(randomMove);
    }

    return scrambleMoves;
}

void RubiksCube::undo() {
    if (moveHistory_.empty()) {
        return; // No moves to undo
    }

    Move lastMove = moveHistory_.back();
    moveHistory_.pop_back();

    // Add to redo history
    redoHistory_.push_back(lastMove);

    // Execute the inverse move
    Move inverseMove = getInverseMove(lastMove);

    // Execute inverse move without adding to history
    executeMove(inverseMove, false);
}

Move RubiksCube::getInverseMoveForUndo() const {
    if (moveHistory_.empty()) {
        return Move::U; // Return default if no history
    }

    Move lastMove = moveHistory_.back();
    // Return the inverse move

    return getInverseMove(lastMove);
}

Move RubiksCube::getMoveForRedo() const {
    if (redoHistory_.empty()) {
        return Move::U; // Return default if no redo history
    }

    return redoHistory_.back();
}

void RubiksCube::redo() {
    if (redoHistory_.empty()) {
        return; // No moves to redo
    }

    Move moveToRedo = redoHistory_.back();
    redoHistory_.pop_back();

    // Add back to move history
    moveHistory_.push_back(moveToRedo);

    // Execute the move
    executeMove(moveToRedo, false);
}

void RubiksCube::dump() const {
    // Print in 2D net layout:
    //      Up
    // Left Front Right Back
    //      Down

    // Helper to print a face row
    auto printRow = [](const std::array<Color, 9>& face, int row) {
        for (int col = 0; col < 3; ++col) {
            std::cout << colorToString(face[row * 3 + col]) << " ";
        }
    };

    // Print Up face (aligned with Front face)
    // Front starts after Left face: "O O O   " = 8 characters
    for (int row = 0; row < 3; ++row) {
        std::cout << "        ";
        printRow(up_, row);
        std::cout << std::endl;
    }
    std::cout << std::endl;

    // Print Left, Front, Right, Back side by side
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

    // Print Down face (aligned with Front face)
    for (int row = 0; row < 3; ++row) {
        std::cout << "        ";
        printRow(down_, row);
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Face index layout:
// 0 1 2
// 3 4 5
// 6 7 8

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
        // Counter-clockwise: front <- left <- back <- right <- front
        shiftLeftOnY(front_, left_, row);
        shiftLeftOnY(left_,  back_, row);
        shiftLeftOnY(back_, right_, row);
        shiftLeftOnY(right_, temp, row);
    } else {
        // Clockwise: front <- right <- back <- left <- front
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

// E (Equator): Rotate the middle slice between U and D
void RubiksCube::rotateEquator(bool prime) {
    // counter-clockwise from top view
    rotateRowX(!prime, 1);
}

void RubiksCube::rotateDown(bool prime) {
    rotateFaceClockwise(down_, prime);
    // counter-clockwise from top view
    rotateRowX(!prime, 2);
}

void RubiksCube::rotateColY(bool prime, int col = 0) {
    std::array<Color, 9> temp = up_;

    if (prime) {
        // Counter-clockwise: up <- back <- down <- front <- up
        shiftLeftOnXfromBack(up_, back_, col);
        shiftLeftOnXtoBack(back_, down_, col);
        shiftLeftOnX(down_, front_, col);
        shiftLeftOnX(front_, temp, col);
    } else {
        // Clockwise: up <- front <- down <- back <- up
        shiftLeftOnX(up_, front_, col);
        shiftLeftOnX(front_, down_, col);
        shiftLeftOnXfromBack(down_, back_, col);
        shiftLeftOnXtoBack(back_, temp, col);
    }
}

void RubiksCube::rotateLeft(bool prime) {
    rotateFaceClockwise(left_, prime);
    // Counter-clockwise
    rotateColY(!prime, 0);
}

// M (Middle): Rotate the middle slice between L and R
void RubiksCube::rotateMiddle(bool prime) {
    // M (clockwise from right view): up <- front <- down <- back
    // M' (counter-clockwise from right view): up <- back <- down <- front
    rotateColY(prime, 1);
}

void RubiksCube::rotateRight(bool prime) {
    rotateFaceClockwise(right_, prime);
    // Clockwise
    rotateColY(prime, 2);
}

void RubiksCube::rotateFront(bool prime) {
    rotateFaceClockwise(front_, prime);

    if (prime) {
        // Counter-clockwise: up <- right <- down <- left <- up
        std::array<Color, 9> temp = up_;
        up_[6] = right_[0]; up_[7] = right_[3]; up_[8] = right_[6];
        right_[0] = down_[2]; right_[3] = down_[1]; right_[6] = down_[0];
        down_[0] = left_[2]; down_[1] = left_[5]; down_[2] = left_[8];
        left_[2] = temp[8]; left_[5] = temp[7]; left_[8] = temp[6];
    } else {
        // Clockwise: up <- left <- down <- right <- up
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
        // Clockwise: up <- left <- down <- right <- up
        std::array<Color, 9> temp = up_;
        up_[0] = left_[6]; up_[1] = left_[3]; up_[2] = left_[0];
        left_[0] = down_[6]; left_[3] = down_[7]; left_[6] = down_[8];
        down_[6] = right_[8]; down_[7] = right_[5]; down_[8] = right_[2];
        right_[2] = temp[0]; right_[5] = temp[1]; right_[8] = temp[2];
    } else {
        // Counter-clockwise: up <- right <- down <- left <- up
        std::array<Color, 9> temp = up_;
        up_[0] = right_[2]; up_[1] = right_[5]; up_[2] = right_[8];
        right_[2] = down_[8]; right_[5] = down_[7]; right_[8] = down_[6];
        down_[6] = left_[0]; down_[7] = left_[3]; down_[8] = left_[6];
        left_[0] = temp[2]; left_[3] = temp[1]; left_[6] = temp[0];
    }
}

// S (Standing): Rotate the middle slice between F and B
void RubiksCube::rotateStanding(bool prime) {
    // Standing slice affects: Up[3,4,5], Down[3,4,5], Left[1,4,7], Right[1,4,7]
    // Looking from front: S rotates up->right
    if (prime) {
        // S' (counter-clockwise from front): up -> left -> down -> right -> up
        std::array<Color, 9> temp = up_;
        up_[3] = right_[1]; up_[4] = right_[4]; up_[5] = right_[7];
        right_[1] = down_[3]; right_[4] = down_[4]; right_[7] = down_[5];
        down_[3] = left_[1]; down_[4] = left_[4]; down_[5] = left_[7];
        left_[1] = temp[3]; left_[4] = temp[4]; left_[7] = temp[5];
    } else {
        // S (clockwise from front): up -> right -> down -> left -> up
        std::array<Color, 9> temp = up_;
        up_[3] = left_[1]; up_[4] = left_[4]; up_[5] = left_[7];
        left_[1] = down_[3]; left_[4] = down_[4]; left_[7] = down_[5];
        down_[3] = right_[1]; down_[4] = right_[4]; down_[5] = right_[7];
        right_[1] = temp[3]; right_[4] = temp[4]; right_[7] = temp[5];
    }
}

void RubiksCube::rotateFaceClockwise(std::array<Color, 9>& face, bool prime) {
    if (prime) {
        // Counter-clockwise: corners 0->2->8->6, edges 1->5->7->3
        std::array<Color, 9> temp = face;
        face[0] = temp[2]; face[1] = temp[5]; face[2] = temp[8];
        face[3] = temp[1]; face[4] = temp[4]; face[5] = temp[7];
        face[6] = temp[0]; face[7] = temp[3]; face[8] = temp[6];
    } else {
        // Clockwise: corners 0->6->8->2, edges 1->3->7->5
        std::array<Color, 9> temp = face;
        face[0] = temp[6]; face[1] = temp[3]; face[2] = temp[0];
        face[3] = temp[7]; face[4] = temp[4]; face[5] = temp[1];
        face[6] = temp[8]; face[7] = temp[5]; face[8] = temp[2];
    }
}

void RubiksCube::rotateX(bool prime) {
    // X-axis rotation (right-left axis):
    // X = R M' L' (rotate whole cube clockwise around X-axis, from right view)
    // X' = R' M L (counter-clockwise)
    if (prime) {
        rotateRight(true);   // R'
        rotateMiddle(true);  // M (M goes same direction as L)
        rotateLeft(false);   // L
    } else {
        rotateRight(false);  // R
        rotateMiddle(false); // M' (M' goes same direction as R)
        rotateLeft(true);    // L'
    }
}

void RubiksCube::rotateY(bool prime) {
    // Y-axis rotation (up-down axis):
    // Y = U E' D' (rotate whole cube clockwise around Y-axis, from top view)
    // Y' = U' E D (counter-clockwise)
    if (prime) {
        rotateUp(true);      // U'
        rotateEquator(false);// E (E goes same direction as D)
        rotateDown(false);   // D
    } else {
        rotateUp(false);     // U
        rotateEquator(true); // E' (E' goes same direction as U)
        rotateDown(true);    // D'
    }
}

void RubiksCube::rotateZ(bool prime) {
    // Z-axis rotation (front-back axis):
    // Z = F S B' (rotate whole cube clockwise around Z-axis, from front view)
    // Z' = F' S' B (counter-clockwise)
    if (prime) {
        rotateFront(true);    // F'
        rotateStanding(true); // S' (S' goes same direction as F')
        rotateBack(false);    // B
    } else {
        rotateFront(false);   // F
        rotateStanding(false);// S (S goes same direction as F)
        rotateBack(true);     // B'
    }
}
