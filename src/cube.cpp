#include "cube.h"
#include <algorithm>

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
        default:       return "?";
    }
}

RubiksCube::RubiksCube()
    : front_(RubiksCube::fillColor(Color::RED))
    , back_(RubiksCube::fillColor(Color::ORANGE))
    , left_(RubiksCube::fillColor(Color::GREEN))
    , right_(RubiksCube::fillColor(Color::BLUE))
    , up_(RubiksCube::fillColor(Color::WHITE))
    , down_(RubiksCube::fillColor(Color::YELLOW))
{
}

void RubiksCube::executeMove(Move move) {
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
    front_ = RubiksCube::fillColor(Color::RED);
    back_ = RubiksCube::fillColor(Color::ORANGE);
    left_ = RubiksCube::fillColor(Color::GREEN);
    right_ = RubiksCube::fillColor(Color::BLUE);
    up_ = RubiksCube::fillColor(Color::WHITE);
    down_ = RubiksCube::fillColor(Color::YELLOW);
}

void RubiksCube::rotateUp(bool prime) {
    rotateFaceClockwise(up_, prime);

    if (prime) {
        std::array<Color, 3> temp = {front_[0], front_[1], front_[2]};
        std::copy_n(left_.begin(), 3, front_.begin());
        std::copy_n(back_.begin(), 3, left_.begin());
        std::copy_n(right_.begin(), 3, back_.begin());
        std::copy_n(temp.begin(), 3, right_.begin());
    } else {
        std::array<Color, 3> temp = {front_[0], front_[1], front_[2]};
        std::copy_n(right_.begin(), 3, front_.begin());
        std::copy_n(back_.begin(), 3, right_.begin());
        std::copy_n(left_.begin(), 3, back_.begin());
        std::copy_n(temp.begin(), 3, left_.begin());
    }
}

void RubiksCube::rotateDown(bool prime) {
    rotateFaceClockwise(down_, prime);

    if (prime) {
        std::array<Color, 3> temp = {front_[6], front_[7], front_[8]};
        std::copy_n(left_.begin() + 6, 3, front_.begin() + 6);
        std::copy_n(back_.begin() + 6, 3, left_.begin() + 6);
        std::copy_n(right_.begin() + 6, 3, back_.begin() + 6);
        std::copy_n(temp.begin(), 3, right_.begin() + 6);
    } else {
        std::array<Color, 3> temp = {front_[6], front_[7], front_[8]};
        std::copy_n(right_.begin() + 6, 3, front_.begin() + 6);
        std::copy_n(back_.begin() + 6, 3, right_.begin() + 6);
        std::copy_n(left_.begin() + 6, 3, back_.begin() + 6);
        std::copy_n(temp.begin(), 3, left_.begin() + 6);
    }
}

void RubiksCube::rotateLeft(bool prime) {
    rotateFaceClockwise(left_, prime);

    if (prime) {
        std::array<Color, 3> temp = {up_[0], up_[3], up_[6]};
        up_[0] = back_[8]; up_[3] = back_[5]; up_[6] = back_[2];
        back_[2] = down_[0]; back_[5] = down_[3]; back_[8] = down_[6];
        down_[0] = front_[0]; down_[3] = front_[3]; down_[6] = front_[6];
        front_[0] = temp[0]; front_[3] = temp[1]; front_[6] = temp[2];
    } else {
        std::array<Color, 3> temp = {up_[0], up_[3], up_[6]};
        up_[0] = front_[0]; up_[3] = front_[3]; up_[6] = front_[6];
        front_[0] = down_[0]; front_[3] = down_[3]; front_[6] = down_[6];
        down_[0] = back_[8]; down_[3] = back_[5]; down_[6] = back_[2];
        back_[2] = temp[2]; back_[5] = temp[1]; back_[8] = temp[0];
    }
}

void RubiksCube::rotateRight(bool prime) {
    rotateFaceClockwise(right_, prime);

    if (prime) {
        std::array<Color, 3> temp = {up_[2], up_[5], up_[8]};
        up_[2] = front_[2]; up_[5] = front_[5]; up_[8] = front_[8];
        front_[2] = down_[2]; front_[5] = down_[5]; front_[8] = down_[8];
        down_[2] = back_[6]; down_[5] = back_[3]; down_[8] = back_[0];
        back_[0] = temp[2]; back_[3] = temp[1]; back_[6] = temp[0];
    } else {
        std::array<Color, 3> temp = {up_[2], up_[5], up_[8]};
        up_[2] = back_[6]; up_[5] = back_[3]; up_[8] = back_[0];
        back_[0] = down_[8]; back_[3] = down_[5]; back_[6] = down_[2];
        down_[2] = front_[2]; down_[5] = front_[5]; down_[8] = front_[8];
        front_[2] = temp[0]; front_[5] = temp[1]; front_[8] = temp[2];
    }
}

void RubiksCube::rotateFront(bool prime) {
    rotateFaceClockwise(front_, prime);

    if (prime) {
        std::array<Color, 3> temp = {up_[6], up_[7], up_[8]};
        up_[6] = right_[0]; up_[7] = right_[3]; up_[8] = right_[6];
        right_[0] = down_[2]; right_[3] = down_[1]; right_[6] = down_[0];
        down_[0] = left_[8]; down_[1] = left_[5]; down_[2] = left_[2];
        left_[2] = temp[0]; left_[5] = temp[1]; left_[8] = temp[2];
    } else {
        std::array<Color, 3> temp = {up_[6], up_[7], up_[8]};
        up_[6] = left_[2]; up_[7] = left_[5]; up_[8] = left_[8];
        left_[2] = down_[0]; left_[5] = down_[1]; left_[8] = down_[2];
        down_[0] = right_[6]; down_[1] = right_[3]; down_[2] = right_[0];
        right_[0] = temp[0]; right_[3] = temp[1]; right_[6] = temp[2];
    }
}

void RubiksCube::rotateBack(bool prime) {
    rotateFaceClockwise(back_, prime);

    if (prime) {
        std::array<Color, 3> temp = {up_[0], up_[1], up_[2]};
        up_[0] = left_[0]; up_[1] = left_[3]; up_[2] = left_[6];
        left_[0] = down_[8]; left_[3] = down_[5]; left_[6] = down_[2];
        down_[2] = right_[8]; down_[5] = right_[5]; down_[8] = right_[2];
        right_[2] = temp[0]; right_[5] = temp[1]; right_[8] = temp[2];
    } else {
        std::array<Color, 3> temp = {up_[0], up_[1], up_[2]};
        up_[0] = right_[2]; up_[1] = right_[5]; up_[2] = right_[8];
        right_[2] = down_[8]; right_[5] = down_[5]; right_[8] = down_[2];
        down_[2] = left_[6]; down_[5] = left_[3]; down_[8] = left_[0];
        left_[0] = temp[0]; left_[3] = temp[1]; left_[6] = temp[2];
    }
}

void RubiksCube::rotateFaceClockwise(std::array<Color, 9>& face, bool prime) {
    if (prime) {
        std::array<Color, 9> temp = face;
        face[0] = temp[2]; face[1] = temp[5]; face[2] = temp[8];
        face[5] = temp[7]; face[8] = temp[6]; face[7] = temp[3];
        face[6] = temp[0]; face[3] = temp[1];
    } else {
        std::array<Color, 9> temp = face;
        face[0] = temp[6]; face[1] = temp[3]; face[2] = temp[0];
        face[3] = temp[7]; face[5] = temp[1];
        face[6] = temp[8]; face[7] = temp[5]; face[8] = temp[2];
    }
}

std::array<Color, 9> RubiksCube::fillColor(Color color) {
    return {color, color, color, color, color, color, color, color, color};
}
