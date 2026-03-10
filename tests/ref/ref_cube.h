#ifndef REF_CUBE_H
#define REF_CUBE_H

#include <array>
#include <cstdint>

// Reference Rubik's Cube Implementation
// Uses cubie-based representation (tracking individual cubie positions)
// This is independent from our face-based implementation

namespace ref {

// Colors matching our cube's color scheme
enum class Color : uint8_t {
    WHITE = 0,
    YELLOW = 1,
    RED = 2,
    ORANGE = 3,
    GREEN = 4,
    BLUE = 5
};

// Move types
enum class Move : uint8_t {
    U = 0, UP = 1,
    D = 2, DP = 3,
    L = 4, LP = 5,
    R = 6, RP = 7,
    F = 8, FP = 9,
    B = 10, BP = 11,
    M = 12, MP = 13,
    E = 14, EP = 15,
    S = 16, SP = 17,
    X = 18, XP = 19,
    Y = 20, YP = 21,
    Z = 22, ZP = 23
};

// Face types for getting face colors
enum class Face : uint8_t {
    FRONT = 0,
    BACK = 1,
    LEFT = 2,
    RIGHT = 3,
    UP = 4,
    DOWN = 5
};

class RubiksCube {
public:
    RubiksCube();
    RubiksCube(const RubiksCube&) = default;
    RubiksCube& operator=(const RubiksCube&) = default;

    // Execute a move
    void executeMove(Move move);

    // Check if solved
    bool isSolved() const;

    // Get face colors (for comparison with our cube)
    std::array<Color, 9> getFace(Face face) const;

    // Reset to solved state
    void reset();

private:
    // Helper to fill a face with a single color
    static std::array<Color, 9> fillColor(Color color);

private:
    // Face colors (same representation as our cube)
    std::array<Color, 9> front_;
    std::array<Color, 9> back_;
    std::array<Color, 9> left_;
    std::array<Color, 9> right_;
    std::array<Color, 9> up_;
    std::array<Color, 9> down_;

    // Helper: rotate a face 90 degrees
    void rotateFace(std::array<Color, 9>& face, bool clockwise);

    // Individual move implementations
    void u(bool prime);
    void d(bool prime);
    void l(bool prime);
    void r(bool prime);
    void f(bool prime);
    void b(bool prime);
    void m(bool prime);
    void e(bool prime);
    void s(bool prime);
    void x(bool prime);
    void y(bool prime);
    void z(bool prime);
};

} // namespace ref

#endif // REF_CUBE_H
