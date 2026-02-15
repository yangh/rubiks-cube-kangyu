#ifndef CUBE_H
#define CUBE_H

#include <array>
#include <string>
#include <vector>

// Color representation for cube faces
enum class Color {
    WHITE,
    YELLOW,
    RED,
    ORANGE,
    GREEN,
    BLUE
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
    BP,  // Back prime
    M,  // Middle slice (between L and R)
    MP, // Middle slice prime
    E,  // Equator slice (between U and D)
    EP, // Equator slice prime
    S,  // Standing slice (between F and B)
    SP  // Standing slice prime
};

// Convert color to RGB
std::array<float, 3> colorToRgb(Color color);

// Convert move to string
std::string moveToString(Move move);

// Parse a move string (e.g., "U", "R'", "F2") to Move enum
// Returns true if parsing succeeded
bool parseMoveString(const std::string& moveStr, Move& outMove);

// Parse a sequence of move strings to a vector of moves
// Moves can be separated by spaces, commas, or newlines
std::vector<Move> parseMoveSequence(const std::string& sequence);

// Convert color to string abbreviation
std::string colorToString(Color color);

// Represents the 3x3x3 Rubik's cube state
class RubiksCube {
public:
    RubiksCube();
    RubiksCube(const RubiksCube&) = default;
    RubiksCube& operator=(const RubiksCube&) = default;

    // Execute a move on the cube
    void executeMove(Move move);

    // Check if the cube is solved
    bool isSolved() const;

    // Reset the cube to solved state
    void reset();

    // Scramble the cube with random moves
    std::vector<Move> scramble(int numMoves = 20);

    // Dump cube state to console in 2D view format
    void dump() const;

    // Get face colors (for rendering)
    std::array<Color, 9> getFront() const { return front_; }
    std::array<Color, 9> getBack() const { return back_; }
    std::array<Color, 9> getLeft() const { return left_; }
    std::array<Color, 9> getRight() const { return right_; }
    std::array<Color, 9> getUp() const { return up_; }
    std::array<Color, 9> getDown() const { return down_; }

    // Get move history
    const std::vector<Move>& getMoveHistory() const { return moveHistory_; }

    // Undo the last move
    void undo();

    // Redo the last undone move
    void redo();

    // Get redo history
    const std::vector<Move>& getRedoHistory() const { return redoHistory_; }

    // Check if redo is available
    bool canRedo() const { return !redoHistory_.empty(); }

private:
    std::array<Color, 9> front_;
    std::array<Color, 9> back_;
    std::array<Color, 9> left_;
    std::array<Color, 9> right_;
    std::array<Color, 9> up_;
    std::array<Color, 9> down_;

    // Move history
    std::vector<Move> moveHistory_;

    // Redo history (moves that were undone)
    std::vector<Move> redoHistory_;

    // Face rotation functions
    void rotateUp(bool prime);
    void rotateDown(bool prime);
    void rotateLeft(bool prime);
    void rotateRight(bool prime);
    void rotateFront(bool prime);
    void rotateBack(bool prime);

    // Slice rotation functions
    void rotateMiddle(bool prime);   // M: between L and R
    void rotateEquator(bool prime); // E: between U and D
    void rotateStanding(bool prime); // S: between F and B

    // Rotate a face 90 degrees clockwise (or counter-clockwise if prime)
    void rotateFaceClockwise(std::array<Color, 9>& face, bool prime);

    // Helper function to fill an array with a single color
    static std::array<Color, 9> fillColor(Color color);
};

#endif // CUBE_H
