#ifndef CUBE_H
#define CUBE_H

#include <array>
#include <string>

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

    // Dump cube state to console in 2D view format
    void dump() const;

    // Get face colors (for rendering)
    std::array<Color, 9> getFront() const { return front_; }
    std::array<Color, 9> getBack() const { return back_; }
    std::array<Color, 9> getLeft() const { return left_; }
    std::array<Color, 9> getRight() const { return right_; }
    std::array<Color, 9> getUp() const { return up_; }
    std::array<Color, 9> getDown() const { return down_; }

private:
    std::array<Color, 9> front_;
    std::array<Color, 9> back_;
    std::array<Color, 9> left_;
    std::array<Color, 9> right_;
    std::array<Color, 9> up_;
    std::array<Color, 9> down_;

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
