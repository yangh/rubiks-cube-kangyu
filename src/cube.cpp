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
        default:       return "?";
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
    , back_(fillColor(Color::BLUE))
    , left_(fillColor(Color::ORANGE))
    , right_(fillColor(Color::RED))
    , up_(fillColor(Color::WHITE))
    , down_(fillColor(Color::YELLOW))
{
}

void RubiksCube::executeMove(Move move) {
    // Clear redo history when a new move is executed
    redoHistory_.clear();

    // Add move to history
    moveHistory_.push_back(move);

    switch (move) {
        case Move::U:  rotateUp(false); break;
        case Move::UP: rotateUp(true); break;
        case Move::D:  rotateDown(false); break;
        case Move::DP: rotateDown(true); break;
        case Move::L: rotateLeft(false); break;
        case Move::LP: rotateLeft(true); break;
        case Move::R: rotateRight(false); break;
        case Move::RP: rotateRight(true); break;
        case Move::F: rotateFront(false); break;
        case Move::FP: rotateFront(true); break;
        case Move::B: rotateBack(false); break;
        case Move::BP: rotateBack(true); break;
        case Move::M:  rotateMiddle(false); break;
        case Move::MP: rotateMiddle(true); break;
        case Move::E:  rotateEquator(false); break;
        case Move::EP: rotateEquator(true); break;
        case Move::S:  rotateStanding(false); break;
        case Move::SP: rotateStanding(true); break;
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
    // Array of all 12 basic moves (U, U', D, D', L, L', R, R', F, F', B, B')
    static const Move basicMoves[] = {
        Move::U, Move::UP,
        Move::D, Move::DP,
        Move::L, Move::LP,
        Move::R, Move::RP,
        Move::F, Move::FP,
        Move::B, Move::BP
    };
    static const int numBasicMoves = 12;

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
    Move inverseMove;
    switch (lastMove) {
        case Move::U:  inverseMove = Move::UP; break;
        case Move::UP: inverseMove = Move::U; break;
        case Move::D:  inverseMove = Move::DP; break;
        case Move::DP: inverseMove = Move::D; break;
        case Move::L:  inverseMove = Move::LP; break;
        case Move::LP: inverseMove = Move::L; break;
        case Move::R:  inverseMove = Move::RP; break;
        case Move::RP: inverseMove = Move::R; break;
        case Move::F:  inverseMove = Move::FP; break;
        case Move::FP: inverseMove = Move::F; break;
        case Move::B:  inverseMove = Move::BP; break;
        case Move::BP: inverseMove = Move::B; break;
        case Move::M:  inverseMove = Move::MP; break;
        case Move::MP: inverseMove = Move::M; break;
        case Move::E:  inverseMove = Move::EP; break;
        case Move::EP: inverseMove = Move::E; break;
        case Move::S:  inverseMove = Move::SP; break;
        case Move::SP: inverseMove = Move::S; break;
        default: return;
    }

    // Execute inverse move without adding to history
    switch (inverseMove) {
        case Move::U:  rotateUp(false); break;
        case Move::UP: rotateUp(true); break;
        case Move::D:  rotateDown(false); break;
        case Move::DP: rotateDown(true); break;
        case Move::L: rotateLeft(false); break;
        case Move::LP: rotateLeft(true); break;
        case Move::R: rotateRight(false); break;
        case Move::RP: rotateRight(true); break;
        case Move::F: rotateFront(false); break;
        case Move::FP: rotateFront(true); break;
        case Move::B: rotateBack(false); break;
        case Move::BP: rotateBack(true); break;
        case Move::M:  rotateMiddle(false); break;
        case Move::MP: rotateMiddle(true); break;
        case Move::E:  rotateEquator(false); break;
        case Move::EP: rotateEquator(true); break;
        case Move::S:  rotateStanding(false); break;
        case Move::SP: rotateStanding(true); break;
        default: break;
    }
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
    switch (moveToRedo) {
        case Move::U:  rotateUp(false); break;
        case Move::UP: rotateUp(true); break;
        case Move::D:  rotateDown(false); break;
        case Move::DP: rotateDown(true); break;
        case Move::L: rotateLeft(false); break;
        case Move::LP: rotateLeft(true); break;
        case Move::R: rotateRight(false); break;
        case Move::RP: rotateRight(true); break;
        case Move::F: rotateFront(false); break;
        case Move::FP: rotateFront(true); break;
        case Move::B: rotateBack(false); break;
        case Move::BP: rotateBack(true); break;
        case Move::M:  rotateMiddle(false); break;
        case Move::MP: rotateMiddle(true); break;
        case Move::E:  rotateEquator(false); break;
        case Move::EP: rotateEquator(true); break;
        case Move::S:  rotateStanding(false); break;
        case Move::SP: rotateStanding(true); break;
        default: break;
    }
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

void RubiksCube::rotateUp(bool prime) {
    rotateFaceClockwise(up_, prime);

    if (prime) {
        // Counter-clockwise: front <- left <- back <- right <- front
        std::array<Color, 3> temp = {front_[0], front_[1], front_[2]};
        front_[0] = left_[0]; front_[1] = left_[1]; front_[2] = left_[2];
        left_[0] = back_[0]; left_[1] = back_[1]; left_[2] = back_[2];
        back_[0] = right_[0]; back_[1] = right_[1]; back_[2] = right_[2];
        right_[0] = temp[0]; right_[1] = temp[1]; right_[2] = temp[2];
    } else {
        // Clockwise: front <- right <- back <- left <- front
        std::array<Color, 3> temp = {front_[0], front_[1], front_[2]};
        front_[0] = right_[0]; front_[1] = right_[1]; front_[2] = right_[2];
        right_[0] = back_[0]; right_[1] = back_[1]; right_[2] = back_[2];
        back_[0] = left_[0]; back_[1] = left_[1]; back_[2] = left_[2];
        left_[0] = temp[0]; left_[1] = temp[1]; left_[2] = temp[2];
    }
}

void RubiksCube::rotateDown(bool prime) {
    rotateFaceClockwise(down_, prime);

    if (prime) {
        // Counter-clockwise: front <- right <- back <- left <- front
        std::array<Color, 3> temp = {front_[6], front_[7], front_[8]};
        front_[6] = right_[6]; front_[7] = right_[7]; front_[8] = right_[8];
        right_[6] = back_[6]; right_[7] = back_[7]; right_[8] = back_[8];
        back_[6] = left_[6]; back_[7] = left_[7]; back_[8] = left_[8];
        left_[6] = temp[0]; left_[7] = temp[1]; left_[8] = temp[2];
    } else {
        // Clockwise: front <- left <- back <- right <- front
        std::array<Color, 3> temp = {front_[6], front_[7], front_[8]};
        front_[6] = left_[6]; front_[7] = left_[7]; front_[8] = left_[8];
        left_[6] = back_[6]; left_[7] = back_[7]; left_[8] = back_[8];
        back_[6] = right_[6]; back_[7] = right_[7]; back_[8] = right_[8];
        right_[6] = temp[0]; right_[7] = temp[1]; right_[8] = temp[2];
    }
}

void RubiksCube::rotateLeft(bool prime) {
    rotateFaceClockwise(left_, prime);

    if (prime) {
        // Counter-clockwise: up <- front <- down <- back <- up
        std::array<Color, 3> temp = {up_[0], up_[3], up_[6]};
        up_[0] = front_[0]; up_[3] = front_[3]; up_[6] = front_[6];
        front_[0] = down_[0]; front_[3] = down_[3]; front_[6] = down_[6];
        down_[0] = back_[8]; down_[3] = back_[5]; down_[6] = back_[2];
        back_[8] = temp[0]; back_[5] = temp[1]; back_[2] = temp[2];
    } else {
        // Clockwise: up <- back <- down <- front <- up
        std::array<Color, 3> temp = {up_[0], up_[3], up_[6]};
        up_[0] = back_[8]; up_[3] = back_[5]; up_[6] = back_[2];
        back_[8] = down_[6]; back_[5] = down_[3]; back_[2] = down_[0];
        down_[0] = front_[0]; down_[3] = front_[3]; down_[6] = front_[6];
        front_[0] = temp[0]; front_[3] = temp[1]; front_[6] = temp[2];
    }
}

void RubiksCube::rotateRight(bool prime) {
    rotateFaceClockwise(right_, prime);

    if (prime) {
        // Counter-clockwise: up <- back <- down <- front <- up
        std::array<Color, 3> temp = {up_[2], up_[5], up_[8]};
        up_[2] = back_[6]; up_[5] = back_[3]; up_[8] = back_[0];
        back_[6] = down_[8]; back_[3] = down_[5]; back_[0] = down_[2];
        down_[2] = front_[2]; down_[5] = front_[5]; down_[8] = front_[8];
        front_[2] = temp[0]; front_[5] = temp[1]; front_[8] = temp[2];
    } else {
        // Clockwise: up <- front <- down <- back <- up
        std::array<Color, 3> temp = {up_[2], up_[5], up_[8]};
        up_[2] = front_[2]; up_[5] = front_[5]; up_[8] = front_[8];
        front_[2] = down_[2]; front_[5] = down_[5]; front_[8] = down_[8];
        down_[2] = back_[6]; down_[5] = back_[3]; down_[8] = back_[0];
        back_[6] = temp[0]; back_[3] = temp[1]; back_[0] = temp[2];
    }
}

void RubiksCube::rotateFront(bool prime) {
    rotateFaceClockwise(front_, prime);

    if (prime) {
        // Counter-clockwise: up <- right <- down <- left <- up
        std::array<Color, 3> temp = {up_[6], up_[7], up_[8]};
        up_[6] = right_[0]; up_[7] = right_[3]; up_[8] = right_[6];
        right_[0] = down_[2]; right_[3] = down_[1]; right_[6] = down_[0];
        down_[0] = left_[8]; down_[1] = left_[5]; down_[2] = left_[2];
        left_[2] = temp[0]; left_[5] = temp[1]; left_[8] = temp[2];
    } else {
        // Clockwise: up <- left <- down <- right <- up
        std::array<Color, 3> temp = {up_[6], up_[7], up_[8]};
        up_[6] = left_[2]; up_[7] = left_[5]; up_[8] = left_[8];
        left_[2] = down_[2]; left_[5] = down_[1]; left_[8] = down_[0];
        down_[0] = right_[6]; down_[1] = right_[3]; down_[2] = right_[0];
        right_[0] = temp[0]; right_[3] = temp[1]; right_[6] = temp[2];
    }
}

void RubiksCube::rotateBack(bool prime) {
    rotateFaceClockwise(back_, prime);

    if (prime) {
        // Clockwise: up <- left <- down <- right <- up
        std::array<Color, 3> temp = {up_[0], up_[1], up_[2]};
        up_[0] = left_[0]; up_[1] = left_[3]; up_[2] = left_[6];
        left_[0] = down_[6]; left_[3] = down_[7]; left_[6] = down_[8];
        down_[6] = right_[2]; down_[7] = right_[5]; down_[8] = right_[8];
        right_[2] = temp[0]; right_[5] = temp[1]; right_[8] = temp[2];
    } else {
        // Counter-clockwise: up <- right <- down <- left <- up
        std::array<Color, 3> temp = {up_[0], up_[1], up_[2]};
        up_[0] = right_[2]; up_[1] = right_[5]; up_[2] = right_[8];
        right_[2] = down_[6]; right_[5] = down_[7]; right_[8] = down_[8];
        down_[6] = left_[0]; down_[7] = left_[3]; down_[8] = left_[6];
        left_[0] = temp[0]; left_[3] = temp[1]; left_[6] = temp[2];
    }
}

// M (Middle): Rotate the middle slice between L and R
void RubiksCube::rotateMiddle(bool prime) {
    // Middle slice affects: Up[1,4,7], Down[1,4,7], Front[1,4,7], Back[1,4,7]
    // Looking from left: M rotates up->down (same as L from left perspective)
    if (prime) {
        // M' (clockwise from left): up -> back -> down -> front -> up
        std::array<Color, 3> temp = {up_[1], up_[4], up_[7]};
        up_[1] = front_[1]; up_[4] = front_[4]; up_[7] = front_[7];
        front_[1] = down_[1]; front_[4] = down_[4]; front_[7] = down_[7];
        down_[1] = back_[1]; down_[4] = back_[4]; down_[7] = back_[7];
        back_[1] = temp[0]; back_[4] = temp[1]; back_[7] = temp[2];
    } else {
        // M (counter-clockwise from left): up -> front -> down -> back -> up
        std::array<Color, 3> temp = {up_[1], up_[4], up_[7]};
        up_[1] = back_[1]; up_[4] = back_[4]; up_[7] = back_[7];
        back_[1] = down_[1]; back_[4] = down_[4]; back_[7] = down_[7];
        down_[1] = front_[1]; down_[4] = front_[4]; down_[7] = front_[7];
        front_[1] = temp[0]; front_[4] = temp[1]; front_[7] = temp[2];
    }
}

// E (Equator): Rotate the middle slice between U and D
void RubiksCube::rotateEquator(bool prime) {
    // Equator slice affects: Front[3,4,5], Back[3,4,5], Left[3,4,5], Right[3,4,5]
    // Looking from top: E rotates front->left
    if (prime) {
        // E (clockwise from top): front -> left -> back -> right -> front
        std::array<Color, 3> temp = {front_[3], front_[4], front_[5]};
        front_[3] = right_[3]; front_[4] = right_[4]; front_[5] = right_[5];
        right_[3] = back_[3]; right_[4] = back_[4]; right_[5] = back_[5];
        back_[3] = left_[3]; back_[4] = left_[4]; back_[5] = left_[5];
        left_[3] = temp[0]; left_[4] = temp[1]; left_[5] = temp[2];
    } else {
        // E' (counter-clockwise from top): front -> right -> back -> left -> front
        std::array<Color, 3> temp = {front_[3], front_[4], front_[5]};
        front_[3] = left_[3]; front_[4] = left_[4]; front_[5] = left_[5];
        left_[3] = back_[3]; left_[4] = back_[4]; left_[5] = back_[5];
        back_[3] = right_[3]; back_[4] = right_[4]; back_[5] = right_[5];
        right_[3] = temp[0]; right_[4] = temp[1]; right_[5] = temp[2];
    }
}

// S (Standing): Rotate the middle slice between F and B
void RubiksCube::rotateStanding(bool prime) {
    // Standing slice affects: Up[3,4,5], Down[3,4,5], Left[1,4,7], Right[1,4,7]
    // Looking from front: S rotates up->right
    if (prime) {
        // S' (counter-clockwise from front): up -> left -> down -> right -> up
        std::array<Color, 3> temp = {up_[3], up_[4], up_[5]};
        up_[3] = right_[1]; up_[4] = right_[4]; up_[5] = right_[7];
        right_[1] = down_[3]; right_[4] = down_[4]; right_[7] = down_[5];
        down_[3] = left_[1]; down_[4] = left_[4]; down_[5] = left_[7];
        left_[1] = temp[0]; left_[4] = temp[1]; left_[7] = temp[2];
    } else {
        // S (clockwise from front): up -> right -> down -> left -> up
        std::array<Color, 3> temp = {up_[3], up_[4], up_[5]};
        up_[3] = left_[1]; up_[4] = left_[4]; up_[5] = left_[7];
        left_[1] = down_[3]; left_[4] = down_[4]; left_[7] = down_[5];
        down_[3] = right_[1]; down_[4] = right_[4]; down_[5] = right_[7];
        right_[1] = temp[0]; right_[4] = temp[1]; right_[7] = temp[2];
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
