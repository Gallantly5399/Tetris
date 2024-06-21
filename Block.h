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
#include "GLFW/glfw3.h"
#include <SFML/Graphics.hpp>


enum class BlockType{
    O = 0,
    S = 1,
    Z = 2,
    J = 3,
    L = 4,
    T = 5,
    I = 6
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
    //rotate if valid
    bool rotate(const Grid& grid);

    //just rotate in the local coordinate
    void rotate();
    BlockPosition getPosition() const;
    bool moveLeft(const Grid& grid);
    bool moveRight(const Grid& grid);
    bool moveDown(const Grid& grid);
    void setStartRow(int row) {
        startRow = row;
    }
    BlockType getType() const {
        return type;
    }
    void setStartColumn(int column) {
        startColumn = column;
    }
    Block() {
        type = BlockType::O;
    }
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

    //FIXME:: maybe something wrong
    std::pair<int, int> getScreenPosition(int row, int column, int blockWidth, int stripeWidth, int screenWidth, int screenHeight, int startPosX, int startPosY, bool reverseY = true, bool startFromLeftTop = true) const {
        row += startRow;
        column += startColumn;
        int posX = startPosX + column * (blockWidth + stripeWidth);
        int posY = startPosY + row * (blockWidth + stripeWidth);
        if (startFromLeftTop) {
            posY += blockWidth + stripeWidth;
        }
        if (reverseY) {
            posY = screenHeight - posY;
        }
        return {posX, posY};
    }
    int getStartRow() const {
        return startRow;
    }

    int getStartColumn() const {
        return startColumn;
    }
    void output() {
#ifndef NDEBUG
        for (int i = shape.size() - 1;i >= 0;i --) {
            for (auto j : shape[i]) {
                std::cout << j << " ";
            }
            std::cout << std::endl;
        }
#endif
    }


private:
    std::vector<std::vector<int>> shape;
    BlockType type;
    sf::Color color;
    int startRow = 0;
    int startColumn = 0;
    unsigned int rotation = 0;
};
