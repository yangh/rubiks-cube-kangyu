#ifndef CUBE_H
#define CUBE_H

#include "color.h"
#include "move.h"
#include <array>
#include <string>
#include <vector>

enum class Face {
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class RubiksCube {
public:
    RubiksCube() { reset(); };
    RubiksCube(const RubiksCube&) = default;
    RubiksCube& operator=(const RubiksCube&) = default;

    std::vector<Move> scramble(int numMoves = 20);
    void reset();
    void dump() const;

    void executeMove(Move move);
    void executeMove(Move move, bool recordHistory);
    void popMoveHistory();
    void pushToMoveHistory(Move move);
    void popRedoHistory();
    void pushToRedoHistory(Move move);

    void undo();
    void redo();
    Move getInverseMoveForUndo() const;
    Move getMoveForRedo() const;
    const std::vector<Move>& getMoveHistory() const { return moveHistory_; }
    const std::vector<Move>& getRedoHistory() const { return redoHistory_; }
    bool canRedo() const { return !redoHistory_.empty(); }

    bool isSolved() const;
    bool isValidColorConfiguration() const;
    std::string getValidationError() const;

    std::array<Color, 9> getFront() const { return front_; }
    std::array<Color, 9> getBack()  const { return back_;  }
    std::array<Color, 9> getLeft()  const { return left_;  }
    std::array<Color, 9> getRight() const { return right_; }
    std::array<Color, 9> getUp()    const { return up_;    }
    std::array<Color, 9> getDown()  const { return down_;  }

private:
    std::array<Color, 9> front_;
    std::array<Color, 9> back_;
    std::array<Color, 9> left_;
    std::array<Color, 9> right_;
    std::array<Color, 9> up_;
    std::array<Color, 9> down_;
    std::vector<Move> moveHistory_;
    std::vector<Move> redoHistory_;

    void rotateRowX(bool prime, int row);
    void rotateUp(bool prime);
    void rotateDown(bool prime);
    void rotateColY(bool prime, int col);
    void rotateLeft(bool prime);
    void rotateRight(bool prime);
    void rotateFront(bool prime);
    void rotateBack(bool prime);
    void rotateMiddle(bool prime);
    void rotateEquator(bool prime);
    void rotateStanding(bool prime);
    void rotateX(bool prime);
    void rotateY(bool prime);
    void rotateZ(bool prime);
    void rotateFaceClockwise(std::array<Color, 9>& face, bool prime);
};

#endif // CUBE_H
