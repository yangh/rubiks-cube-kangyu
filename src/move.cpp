#include "move.h"
#include <cctype>
#include <random>
#include <sstream>

namespace MoveLookup {

const MoveInfo& getMoveInfo(Move move) {
    static const MoveInfo table[] = {
        // name, inverse, family, base, axis, {layer,row,col,all}, isPrime, isDouble
        {"U",   Move::UP,  MoveFamily::U, Move::U,  {0, -1,  0}, {-1, 2, -1, false}, false, false},
        {"U'",  Move::U,   MoveFamily::U, Move::U,  {0,  1,  0}, {-1, 2, -1, false}, true,  false},
        {"D",   Move::DP,  MoveFamily::D, Move::D,  {0,  1,  0}, {-1, 0, -1, false}, false, false},
        {"D'",  Move::D,   MoveFamily::D, Move::D,  {0, -1,  0}, {-1, 0, -1, false}, true,  false},
        {"L",   Move::LP,  MoveFamily::L, Move::L,  {1,  0,  0}, {-1, -1, 0, false}, false, false},
        {"L'",  Move::L,   MoveFamily::L, Move::L,  {-1, 0,  0}, {-1, -1, 0, false}, true,  false},
        {"R",   Move::RP,  MoveFamily::R, Move::R,  {-1, 0,  0}, {-1, -1, 2, false}, false, false},
        {"R'",  Move::R,   MoveFamily::R, Move::R,  {1,  0,  0}, {-1, -1, 2, false}, true,  false},
        {"F",   Move::FP,  MoveFamily::F, Move::F,  {0,  0, -1}, {2, -1, -1, false}, false, false},
        {"F'",  Move::F,   MoveFamily::F, Move::F,  {0,  0,  1}, {2, -1, -1, false}, true,  false},
        {"B",   Move::BP,  MoveFamily::B, Move::B,  {0,  0,  1}, {0, -1, -1, false}, false, false},
        {"B'",  Move::B,   MoveFamily::B, Move::B,  {0,  0, -1}, {0, -1, -1, false}, true,  false},
        {"M",   Move::MP,  MoveFamily::M, Move::M,  {1,  0,  0}, {-1, -1, 1, false}, false, false},
        {"M'",  Move::M,   MoveFamily::M, Move::M,  {-1, 0,  0}, {-1, -1, 1, false}, true,  false},
        {"E",   Move::EP,  MoveFamily::E, Move::E,  {0,  1,  0}, {-1, 1, -1, false}, false, false},
        {"E'",  Move::E,   MoveFamily::E, Move::E,  {0, -1,  0}, {-1, 1, -1, false}, true,  false},
        {"S",   Move::SP,  MoveFamily::S, Move::S,  {0,  0, -1}, {1, -1, -1, false}, false, false},
        {"S'",  Move::S,   MoveFamily::S, Move::S,  {0,  0,  1}, {1, -1, -1, false}, true,  false},
        {"X",   Move::XP,  MoveFamily::X, Move::X,  {-1, 0,  0}, {-1, -1, -1, true}, false, false},
        {"X'",  Move::X,   MoveFamily::X, Move::X,  {1,  0,  0}, {-1, -1, -1, true}, true,  false},
        {"Y",   Move::YP,  MoveFamily::Y, Move::Y,  {0, -1,  0}, {-1, -1, -1, true}, false, false},
        {"Y'",  Move::Y,   MoveFamily::Y, Move::Y,  {0,  1,  0}, {-1, -1, -1, true}, true,  false},
        {"Z",   Move::ZP,  MoveFamily::Z, Move::Z,  {0,  0, -1}, {-1, -1, -1, true}, false, false},
        {"Z'",  Move::Z,   MoveFamily::Z, Move::Z,  {0,  0,  1}, {-1, -1, -1, true}, true,  false},
        {"U2",  Move::U2,  MoveFamily::U, Move::U,  {0, -1,  0}, {-1, 2, -1, false}, false, true},
        {"D2",  Move::D2,  MoveFamily::D, Move::D,  {0,  1,  0}, {-1, 0, -1, false}, false, true},
        {"L2",  Move::L2,  MoveFamily::L, Move::L,  {1,  0,  0}, {-1, -1, 0, false}, false, true},
        {"R2",  Move::R2,  MoveFamily::R, Move::R,  {-1, 0,  0}, {-1, -1, 2, false}, false, true},
        {"F2",  Move::F2,  MoveFamily::F, Move::F,  {0,  0, -1}, {2, -1, -1, false}, false, true},
        {"B2",  Move::B2,  MoveFamily::B, Move::B,  {0,  0,  1}, {0, -1, -1, false}, false, true},
        {"M2",  Move::M2,  MoveFamily::M, Move::M,  {1,  0,  0}, {-1, -1, 1, false}, false, true},
        {"E2",  Move::E2,  MoveFamily::E, Move::E,  {0, -1,  0}, {-1, 1, -1, false}, false, true},
        {"S2",  Move::S2,  MoveFamily::S, Move::S,  {0,  0, -1}, {1, -1, -1, false}, false, true},
        {"X2",  Move::X2,  MoveFamily::X, Move::X,  {-1, 0,  0}, {-1, -1, -1, true}, false, true},
        {"Y2",  Move::Y2,  MoveFamily::Y, Move::Y,  {0, -1,  0}, {-1, -1, -1, true}, false, true},
        {"Z2",  Move::Z2,  MoveFamily::Z, Move::Z,  {0,  0, -1}, {-1, -1, -1, true}, false, true},
    };
    
    int index = static_cast<int>(move);
    if (index >= 0 && index < static_cast<int>(sizeof(table) / sizeof(table[0]))) {
        return table[index];
    }
    static const MoveInfo unknown = {"?", Move::U, MoveFamily::NONE, Move::U, {0, 0, 0}, {-1, -1, -1, false}, false, false};
    return unknown;
}

bool charToBaseMove(char c, Move& outMove) {
    switch (c) {
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
        default: return false;
    }
}

Move applyMoveModifier(Move base, bool prime, bool isDouble) {
    if (isDouble) {
        switch (base) {
            case Move::U: return Move::U2;
            case Move::D: return Move::D2;
            case Move::L: return Move::L2;
            case Move::R: return Move::R2;
            case Move::F: return Move::F2;
            case Move::B: return Move::B2;
            case Move::M: return Move::M2;
            case Move::E: return Move::E2;
            case Move::S: return Move::S2;
            case Move::X: return Move::X2;
            case Move::Y: return Move::Y2;
            case Move::Z: return Move::Z2;
            default: return base;
        }
    }
    if (prime) {
        switch (base) {
            case Move::U: return Move::UP;
            case Move::D: return Move::DP;
            case Move::L: return Move::LP;
            case Move::R: return Move::RP;
            case Move::F: return Move::FP;
            case Move::B: return Move::BP;
            case Move::M: return Move::MP;
            case Move::E: return Move::EP;
            case Move::S: return Move::SP;
            case Move::X: return Move::XP;
            case Move::Y: return Move::YP;
            case Move::Z: return Move::ZP;
            default: return base;
        }
    }
    return base;
}

bool isInSlice(int cubeIndex, const AnimationSlice& slice) {
    if (slice.affectsAll) return true;
    
    int layer = cubeIndex / 9;
    int posInLayer = cubeIndex % 9;
    int row = posInLayer / 3;
    int col = posInLayer % 3;
    
    if (slice.layer >= 0 && layer != slice.layer) return false;
    if (slice.row >= 0 && row != slice.row) return false;
    if (slice.col >= 0 && col != slice.col) return false;
    
    return true;
}

} // namespace MoveLookup

std::string moveToString(Move move) {
    return moveToStringFast(move);
}

Move getInverseMove(Move move) {
    return getInverseMoveFast(move);
}

bool parseMoveString(const std::string& moveStr, Move& outMove) {
    std::string trimmed = moveStr;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
    if (trimmed.empty()) return false;

    std::string upper = trimmed;
    for (char& c : upper) c = std::toupper(c);

    if (upper.empty() || upper.length() > 2) return false;

    Move baseMove;
    if (!MoveLookup::charToBaseMove(upper[0], baseMove)) return false;

    bool isPrime = false;
    bool isDouble = false;

    if (upper.length() == 2) {
        if (upper[1] == '\'') {
            isPrime = true;
        } else if (upper[1] == '2') {
            isDouble = true;
        } else {
            return false;
        }
    }

    outMove = MoveLookup::applyMoveModifier(baseMove, isPrime, isDouble);
    return true;
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

std::vector<Move> generateRandomMoves(int numMoves) {
    static const Move basicMoves[] = {
        Move::U, Move::UP,
        Move::D, Move::DP,
        Move::L, Move::LP,
        Move::R, Move::RP,
        Move::F, Move::FP,
        Move::B, Move::BP,
        Move::E, Move::S, Move::M,
        Move::U2, Move::D2,
        Move::L2, Move::R2,
        Move::F2, Move::B2,
        Move::M2, Move::E2,
        Move::S2,
    };
    static const int numBasicMoves = sizeof(basicMoves) / sizeof(Move);

    std::vector<Move> moves;
    moves.reserve(numMoves);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, numBasicMoves - 1);

    for (int i = 0; i < numMoves; ++i) {
        moves.push_back(basicMoves[dis(gen)]);
    }

    return moves;
}
