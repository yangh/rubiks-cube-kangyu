#include <iostream>
#include "src/cube.cpp"

int main() {
    RubiksCube cube;

    std::cout << "=== Initial State ===\n";
    std::cout << "Front face (green should be all green):\n";
    const auto& front = cube.getFront();
    for (int i = 0; i < 9; i++) {
        std::cout << i << ": " << colorToString(front[i]) << " ";
        if ((i + 1) % 3 == 0) std::cout << "\n";
    }

    std::cout << "\nBack face (blue should be all blue):\n";
    const auto& back = cube.getBack();
    for (int i = 0; i < 9; i++) {
        std::cout << i << ": " << colorToString(back[i]) << " ";
        if ((i + 1) % 3 == 0) std::cout << "\n";
    }

    std::cout << "\nUp face (white should be all white):\n";
    const auto& up = cube.getUp();
    for (int i = 0; i < 9; i++) {
        std::cout << i << ": " << colorToString(up[i]) << " ";
        if ((i + 1) % 3 == 0) std::cout << "\n";
    }

    std::cout << "\nDown face (yellow should be all yellow):\n";
    const auto& down = cube.getDown();
    for (int i = 0; i < 9; i++) {
        std::cout << i << ": " << colorToString(down[i]) << " ";
        if ((i + 1) % 3 == 0) std::cout << "\n";
    }

    std::cout << "\n=== After R Move ===\n";
    cube.executeMove(Move::R);

    std::cout << "Front face (right column should be yellow from Down):\n";
    const auto& front2 = cube.getFront();
    for (int i = 0; i < 9; i++) {
        std::cout << i << ": " << colorToString(front2[i]) << " ";
        if ((i + 1) % 3 == 0) std::cout << "\n";
    }
    // Indices 2, 5, 8 (right column) should now be yellow

    std::cout << "\nBack face (left column should be white from Up):\n";
    const auto& back2 = cube.getBack();
    for (int i = 0; i < 9; i++) {
        std::cout << i << ": " << colorToString(back2[i]) << " ";
        if ((i + 1) % 3 == 0) std::cout << "\n";
    }
    // Indices 0, 3, 6 (left column) should now be white

    std::cout << "\nUp face (right column should be blue from Back):\n";
    const auto& up2 = cube.getUp();
    for (int i = 0; i < 9; i++) {
        std::cout << i << ": " << colorToString(up2[i]) << " ";
        if ((i + 1) % 3 == 0) std::cout << "\n";
    }
    // Indices 6, 7, 8 should now be blue

    std::cout << "\nDown face (right column should be green from Front):\n";
    const auto& down2 = cube.getDown();
    for (int i = 0; i < 9; i++) {
        std::cout << i << ": " << colorToString(down2[i]) << " ";
        if ((i + 1) % 3 == 0) std::cout << "\n";
    }
    // Indices 2, 5, 8 should now be green

    std::cout << "\n=== Verification ===\n";

    bool frontOk = (front2[2] == Color::Yellow && front2[5] == Color::Yellow && front2[8] == Color::Yellow);
    bool backOk = (back2[0] == Color::White && back2[3] == Color::White && back2[6] == Color::White);
    bool upOk = (up2[6] == Color::Blue && up2[7] == Color::Blue && up2[8] == Color::Blue);
    bool downOk = (down2[2] == Color::Green && down2[5] == Color::Green && down2[8] == Color::Green);

    std::cout << "Front right column is yellow: " << (frontOk ? "PASS" : "FAIL") << "\n";
    std::cout << "Back left column is white: " << (backOk ? "PASS" : "FAIL") << "\n";
    std::cout << "Up right column is blue: " << (upOk ? "PASS" : "FAIL") << "\n";
    std::cout << "Down right column is green: " << (downOk ? "PASS" : "FAIL") << "\n";

    return (frontOk && backOk && upOk && downOk) ? 0 : 1;
}
