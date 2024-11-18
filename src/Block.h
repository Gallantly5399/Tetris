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
#include <SFML/Graphics.hpp>

enum class Movement {
    Rotate,
    Left,
    Right,
    Down,
    RotateCounterClockwise,
    Hold,
    HardDrop,
};

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

std::ostream &operator<<(std::ostream &os, const Movement &movement);
std::ostream &operator<<(std::ostream &os, const BlockType &blockType);

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

    explicit Block(BlockType type);
    Block();
    //rotate if valid
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
    Block(const Block& block) = default;
    //Copy assignment
    Block& operator=(const Block& block) = default;
    //Move constructor
    Block(Block&& block) noexcept = default;
    //Move assignment
    Block& operator=(Block&& block) noexcept = default;
    //return shape
    const std::vector<std::vector<int>>& getShape() const;
    sf::Color getColor() const;
    std::pair<int, int> getScreenPosition(int row, int column, int blockWidth, int stripeWidth, int screenWidth, int screenHeight, int startPosX, int startPosY, bool reverseY = true, bool startFromLeftTop = true) const;
    int getStartRow() const;
    Block getTransparentBlock() const;
    int getStartColumn() const;
    Movement getLastMovement() const;
    bool getSrs() const;
    int getRotation() const;
    void rotate();//rightRotate
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
    bool isNotSent = true;
};
