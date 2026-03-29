#ifndef CUBE_H
#define CUBE_H

#include "color.h"
#include "move.h"
#include <cassert>

typedef std::array<Color, 9> FaceColor;
FaceColor fillFaceColor(Color color);

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

    Move getLastMove() const { assert(!moveHistory_.empty()); return moveHistory_.back(); }
    Move getLastRedo() const { assert(!redoHistory_.empty()); return redoHistory_.back(); }
    const std::vector<Move>& getMoveHistory() const { return moveHistory_; }
    const std::vector<Move>& getRedoHistory() const { return redoHistory_; }
    bool canUndo() const { return !moveHistory_.empty(); }
    bool canRedo() const { return !redoHistory_.empty(); }

    void undo();
    void redo();

    bool isSolved() const;
    bool isValidColorConfiguration() const;
    std::string getValidationError() const;

    const FaceColor& getFront() const { return front_; }
    const FaceColor& getBack()  const { return back_;  }
    const FaceColor& getLeft()  const { return left_;  }
    const FaceColor& getRight() const { return right_; }
    const FaceColor& getUp()    const { return up_;    }
    const FaceColor& getDown()  const { return down_;  }

private:
    FaceColor front_;
    FaceColor back_;
    FaceColor left_;
    FaceColor right_;
    FaceColor up_;
    FaceColor down_;
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
    void rotateFaceClockwise(FaceColor& face, bool prime);
};

#endif // CUBE_H
