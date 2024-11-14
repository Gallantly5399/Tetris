//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <array>
#include <optional>
#ifndef NDEBUG
#include <iostream>
#endif
#include "Grid.h"
#include <SFML/Graphics.hpp>

//column first vector
enum class BlockType{
    O = 0,
    S = 1,
    Z = 2,
    J = 3,
    L = 4,
    T = 5,
    I = 6,
    None = 7
};

enum class Movement {
    Rotate,
    Left,
    Right,
    Down
};
// local coordinate
// for example, the shape of I block is
//
// |0 0 0 0 0
// |0 0 0 0 0
// |0 1 1 1 1
// |0 0 0 0 0
// |0 0 0 0 0
//  --------->
// the left corner point's world position is (startColumn, startRow)
struct BlockPosition {
    int startRow;
    int startColumn;
};

class Block {
public:

    Block(BlockType type);
    Block();
    bool checkMiniTSpin(const Grid& grid) const;
    //rotate if valid
    bool rotate(const Grid& grid);
    bool moveLeft(const Grid& grid);
    bool moveRight(const Grid& grid);
    bool moveDown(const Grid& grid);
    BlockPosition getPosition() const;
    void setStartRow(int row) {
        startRow = row;
    }

//    void swapForHold(Block& otherBlock);
//    void copyForHold(Block& otherBlock);
    bool empty() const;
    BlockType getType() const;
    void setStartColumn(int column);
//    Block() = delete;
    //Copy constructor
    Block(const Block& block);
    //Copy assignment
    Block& operator=(const Block& block);
    //Move constructor
    Block(Block&& block) noexcept;
    //Move assignment
    Block& operator=(Block&& block) noexcept;
    //return shape
    const std::vector<std::vector<int>>& getShape() const;

    sf::Color getColor() const;
    bool touch(const Grid& grid) const;
    bool rotateCounterClockwise(const Grid& grid);
    std::pair<int, int> getScreenPosition(int row, int column, int blockWidth, int stripeWidth, int screenWidth, int screenHeight, int startPosX, int startPosY, bool reverseY = true, bool startFromLeftTop = true) const;
    int getStartRow() const;
    Block getTransparentBlock() const;
    int getStartColumn() const;
    bool isValid(int startRow_, int startColumn_, const Grid& grid) const;
    bool valid(const Grid& grid) const;
    Movement getLastMovement() const;
    bool getSrs() const;
    int getRotation() const;
    void rotate();
    void rotateCounterClockwise();

//private:
    //just rotate in the local coordinate
    std::vector<std::vector<int>> shape;
    BlockType type;
    sf::Color color;
    int startRow = 0;
    int startColumn = 0;
    int rowSize = 0;
    int columnSize = 0;
    bool srs = false;
    unsigned int rotation = 0;
    Movement lastMovement;
};
