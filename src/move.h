#ifndef MOVE_H
#define MOVE_H

#include <array>
#include <string>
#include <vector>

enum class MoveFamily {
    U, D, L, R, F, B,
    M, E, S,
    X, Y, Z,
    NONE
};

// Standard Rubik's cube moves
enum class Move {
    U,  // Up
    UP, // Up prime
    D,  // Down
    DP, // Down prime
    L,  // Left
    LP, // Left prime
    R,  // Right
    RP, // Right prime
    F,  // Front
    FP, // Front prime
    B,  // Back
    BP, // Back prime
    M,  // Middle slice (between L and R)
    MP, // Middle slice prime
    E,  // Equator slice (between U and D)
    EP, // Equator slice prime
    S,  // Standing slice (between F and B)
    SP, // Standing slice prime
    X,  // X-axis rotation (right-left axis)
    XP, // X-axis rotation prime
    Y,  // Y-axis rotation (up-down axis)
    YP, // Y-axis rotation prime
    Z,  // Z-axis rotation (front-back axis)
    ZP, // Z-axis rotation prime
    // Double moves (180° rotation)
    U2,  // Up face double
    D2,  // Down face double
    L2,  // Left face double
    R2,  // Right face double
    F2,  // Front face double
    B2,  // Back face double
    M2,  // Middle slice double
    E2,  // Equator slice double
    S2,  // Standing slice double
    X2,  // X-axis double rotation
    Y2,  // Y-axis double rotation
    Z2   // Z-axis double rotation
};

struct RotationAxis {
    float x, y, z;
};

struct AnimationSlice {
    int layer;
    int row;
    int col;
    bool affectsAll;
};

struct MoveInfo {
    const char* name;
    Move inverse;
    MoveFamily family;
    Move baseMove;
    RotationAxis axis;
    AnimationSlice slice;
    bool isPrime;
    bool isDouble;
};

// Move lookup utilities
namespace MoveLookup {

const MoveInfo& getMoveInfo(Move move);

bool charToBaseMove(char c, Move& outMove);

Move applyMoveModifier(Move base, bool prime, bool isDouble);

bool isInSlice(int cubeIndex, const AnimationSlice& slice);

} // namespace MoveLookup

// Move string conversion
std::string moveToString(Move move);

// Parse a move string (e.g., "U", "R'") to Move enum
// Returns true if parsing succeeded
bool parseMoveString(const std::string& moveStr, Move& outMove);

// Parse a sequence of move strings to a vector of moves
// Moves can be separated by spaces, commas, or newlines
std::vector<Move> parseMoveSequence(const std::string& sequence);

// Get inverse of a move
Move getInverseMove(Move move);

// Generate random moves for scrambling
std::vector<Move> generateRandomMoves(int numMoves);

// Inline accessors for move properties
inline std::string moveToStringFast(Move move) { return MoveLookup::getMoveInfo(move).name; }
inline Move getInverseMoveFast(Move move) { return MoveLookup::getMoveInfo(move).inverse; }
inline bool isDoubleMove(Move move) { return MoveLookup::getMoveInfo(move).isDouble; }
inline bool isPrimeMove(Move move) { return MoveLookup::getMoveInfo(move).isPrime; }
inline RotationAxis getRotationAxis(Move move) { return MoveLookup::getMoveInfo(move).axis; }
inline MoveFamily getMoveFamily(Move move) { return MoveLookup::getMoveInfo(move).family; }
inline const AnimationSlice& getAnimationSlice(Move move) { return MoveLookup::getMoveInfo(move).slice; }

#endif // MOVE_H
