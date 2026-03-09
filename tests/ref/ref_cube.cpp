#include "ref_cube.h"

namespace ref {

RubiksCube::RubiksCube()
    : front_(fillColor(Color::GREEN))
    , back_(fillColor(Color::BLUE))
    , left_(fillColor(Color::ORANGE))
    , right_(fillColor(Color::RED))
    , up_(fillColor(Color::WHITE))
    , down_(fillColor(Color::YELLOW))
{
}

std::array<Color, 9> RubiksCube::fillColor(Color color) {
    return {color, color, color, color, color, color, color, color, color};
}

void RubiksCube::rotateFace(std::array<Color, 9>& face, bool clockwise) {
    std::array<Color, 9> temp = face;
    if (clockwise) {
        // 0 1 2    3 4 5    6 7 8
        // 0->6, 1->3, 2->0
        // 3->7, 4->4
        // 6->8, 7->5, 8->2
        face[0] = temp[6];
        face[1] = temp[3];
        face[2] = temp[0];
        face[3] = temp[7];
        face[4] = temp[4];
        face[5] = temp[1];
        face[6] = temp[8];
        face[7] = temp[5];
        face[8] = temp[2];
    } else {
        // Counter-clockwise
        // 0->2, 1->5, 2->8
        // 3->1, 4->4
        // 6->0, 7->3, 8->6
        face[0] = temp[2];
        face[1] = temp[5];
        face[2] = temp[8];
        face[3] = temp[1];
        face[4] = temp[4];
        face[5] = temp[7];
        face[6] = temp[0];
        face[7] = temp[3];
        face[8] = temp[6];
    }
}

// U: Rotate Up face and adjacent faces
void RubiksCube::u(bool prime) {
    rotateFace(up_, !prime);

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

// D: Rotate Down face and adjacent faces
void RubiksCube::d(bool prime) {
    rotateFace(down_, !prime);

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

// L: Rotate Left face and adjacent faces
void RubiksCube::l(bool prime) {
    rotateFace(left_, !prime);

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
        back_[8] = down_[0]; back_[5] = down_[3]; back_[2] = down_[6];
        down_[0] = front_[0]; down_[3] = front_[3]; down_[6] = front_[6];
        front_[0] = temp[0]; front_[3] = temp[1]; front_[6] = temp[2];
    }
}

// R: Rotate Right face and adjacent faces
void RubiksCube::r(bool prime) {
    rotateFace(right_, !prime);

    if (prime) {
        // Counter-clockwise (R'): up <- back <- down <- front <- up
        // Note: Back face indices are reversed because we view from behind
        std::array<Color, 3> temp = {up_[2], up_[5], up_[8]};
        up_[2] = back_[6]; up_[5] = back_[3]; up_[8] = back_[0];
        back_[6] = down_[2]; back_[3] = down_[5]; back_[0] = down_[8];
        down_[2] = front_[2]; down_[5] = front_[5]; down_[8] = front_[8];
        front_[2] = temp[0]; front_[5] = temp[1]; front_[8] = temp[2];
    } else {
        // Clockwise (R): up <- front <- down <- back <- up
        std::array<Color, 3> temp = {up_[2], up_[5], up_[8]};
        up_[2] = front_[2]; up_[5] = front_[5]; up_[8] = front_[8];
        front_[2] = down_[2]; front_[5] = down_[5]; front_[8] = down_[8];
        down_[2] = back_[6]; down_[5] = back_[3]; down_[8] = back_[0];
        back_[6] = temp[0]; back_[3] = temp[1]; back_[0] = temp[2];
    }
}

// F: Rotate Front face and adjacent faces
void RubiksCube::f(bool prime) {
    rotateFace(front_, !prime);

    if (prime) {
        // Counter-clockwise: up <- right <- down <- left <- up
        std::array<Color, 3> temp = {up_[6], up_[7], up_[8]};
        up_[6] = right_[0]; up_[7] = right_[3]; up_[8] = right_[6];
        right_[0] = down_[2]; right_[3] = down_[1]; right_[6] = down_[0];
        down_[0] = left_[2]; down_[1] = left_[5]; down_[2] = left_[8];
        left_[2] = temp[2]; left_[5] = temp[1]; left_[8] = temp[0];
    } else {
        // Clockwise: up <- left <- down <- right <- up
        std::array<Color, 3> temp = {up_[6], up_[7], up_[8]};
        up_[6] = left_[8]; up_[7] = left_[5]; up_[8] = left_[2];
        left_[2] = down_[0]; left_[5] = down_[1]; left_[8] = down_[2];
        down_[0] = right_[6]; down_[1] = right_[3]; down_[2] = right_[0];
        right_[0] = temp[0]; right_[3] = temp[1]; right_[6] = temp[2];
    }
}

// B: Rotate Back face and adjacent faces
void RubiksCube::b(bool prime) {
    rotateFace(back_, !prime);

    if (prime) {
        // Clockwise: up <- left <- down <- right <- up
        std::array<Color, 3> temp = {up_[0], up_[1], up_[2]};
        up_[0] = left_[6]; up_[1] = left_[3]; up_[2] = left_[0];
        left_[0] = down_[6]; left_[3] = down_[7]; left_[6] = down_[8];
        down_[6] = right_[8]; down_[7] = right_[5]; down_[8] = right_[2];
        right_[2] = temp[0]; right_[5] = temp[1]; right_[8] = temp[2];
    } else {
        // Counter-clockwise: up <- right <- down <- left <- up
        std::array<Color, 3> temp = {up_[0], up_[1], up_[2]};
        up_[0] = right_[2]; up_[1] = right_[5]; up_[2] = right_[8];
        right_[2] = down_[8]; right_[5] = down_[7]; right_[8] = down_[6];
        down_[6] = left_[0]; down_[7] = left_[3]; down_[8] = left_[6];
        left_[0] = temp[2]; left_[3] = temp[1]; left_[6] = temp[0];
    }
}

// M: Rotate middle slice (between L and R)
void RubiksCube::m(bool prime) {
    // M affects: Up[1,4,7], Down[1,4,7], Front[1,4,7], Back[1,4,7]
    if (prime) {
        // M' (counter-clockwise from left): up <- front <- down <- back <- up
        // Note: Back indices are reversed (viewed from behind)
        std::array<Color, 3> temp = {up_[1], up_[4], up_[7]};
        up_[1] = front_[1]; up_[4] = front_[4]; up_[7] = front_[7];
        front_[1] = down_[1]; front_[4] = down_[4]; front_[7] = down_[7];
        down_[1] = back_[7]; down_[4] = back_[4]; down_[7] = back_[1];
        back_[7] = temp[0]; back_[4] = temp[1]; back_[1] = temp[2];
    } else {
        // M (clockwise from left): up <- back <- down <- front <- up
        // Note: Back indices are reversed (viewed from behind)
        std::array<Color, 3> temp = {up_[1], up_[4], up_[7]};
        up_[1] = back_[7]; up_[4] = back_[4]; up_[7] = back_[1];
        back_[7] = down_[1]; back_[4] = down_[4]; back_[1] = down_[7];
        down_[1] = front_[1]; down_[4] = front_[4]; down_[7] = front_[7];
        front_[1] = temp[0]; front_[4] = temp[1]; front_[7] = temp[2];
    }
}

// E: Rotate equator slice (between U and D)
void RubiksCube::e(bool prime) {
    // E affects: Front[3,4,5], Back[3,4,5], Left[3,4,5], Right[3,4,5]
    if (prime) {
        // E (clockwise from top): front -> right -> back -> left -> front
        std::array<Color, 3> temp = {front_[3], front_[4], front_[5]};
        front_[3] = right_[3]; front_[4] = right_[4]; front_[5] = right_[5];
        right_[3] = back_[3]; right_[4] = back_[4]; right_[5] = back_[5];
        back_[3] = left_[3]; back_[4] = left_[4]; back_[5] = left_[5];
        left_[3] = temp[0]; left_[4] = temp[1]; left_[5] = temp[2];
    } else {
        // E' (counter-clockwise from top): front -> left -> back -> right -> front
        std::array<Color, 3> temp = {front_[3], front_[4], front_[5]};
        front_[3] = left_[3]; front_[4] = left_[4]; front_[5] = left_[5];
        left_[3] = back_[3]; left_[4] = back_[4]; left_[5] = back_[5];
        back_[3] = right_[3]; back_[4] = right_[4]; back_[5] = right_[5];
        right_[3] = temp[0]; right_[4] = temp[1]; right_[5] = temp[2];
    }
}

// S: Rotate standing slice (between F and B)
void RubiksCube::s(bool prime) {
    // S affects: Up[3,4,5], Down[3,4,5], Left[1,4,7], Right[1,4,7]
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

void RubiksCube::executeMove(Move move) {
    switch (move) {
        case Move::U:  u(false); break;
        case Move::UP: u(true); break;
        case Move::D:  d(false); break;
        case Move::DP: d(true); break;
        case Move::L:  l(false); break;
        case Move::LP: l(true); break;
        case Move::R:  r(false); break;
        case Move::RP: r(true); break;
        case Move::F:  f(false); break;
        case Move::FP: f(true); break;
        case Move::B:  b(false); break;
        case Move::BP: b(true); break;
        case Move::M:  m(false); break;
        case Move::MP: m(true); break;
        case Move::E:  e(false); break;
        case Move::EP: e(true); break;
        case Move::S:  s(false); break;
        case Move::SP: s(true); break;
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

std::array<Color, 9> RubiksCube::getFace(Face face) const {
    switch (face) {
        case Face::FRONT: return front_;
        case Face::BACK:  return back_;
        case Face::LEFT:  return left_;
        case Face::RIGHT: return right_;
        case Face::UP:  return up_;
        case Face::DOWN: return down_;
    }
    return front_; // Should not reach here
}

void RubiksCube::reset() {
    front_ = fillColor(Color::GREEN);
    back_ = fillColor(Color::BLUE);
    left_ = fillColor(Color::ORANGE);
    right_ = fillColor(Color::RED);
    up_ = fillColor(Color::WHITE);
    down_ = fillColor(Color::YELLOW);
}

} // namespace ref
