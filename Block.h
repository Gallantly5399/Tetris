//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <array>
#include <optional>
#include <glm/glm.hpp>
#ifndef NDEBUG
#include <iostream>
#endif
#include "Grid.h"
#include "GLFW/glfw3.h"

const static std::array<std::array<std::array<int,2>, 5>, 4> RotationOffsetI {
        0, 0, -1, 0, 2, 0, -1, 0, 2, 0,
        -1, 0,  0, 0, 0, 0, 0, 1, 0, -2,
        -1, 1, 1, 1, -2, 1, 1, 0, -2, 0,
        0, 1, 0, 1, 0, 1, 0, -1, 0, 2
};

const static std::array<std::array<std::array<int,2>, 1>, 4> RotationOffsetO {
        0, 0,
        0, -1,
        -1, -1,
        -1, 0
};

const static std::array<std::array<std::array<int,2>, 5>, 4> RotationOffsetJ {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 1, -1, 0, 2, 1, 2,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, -1, 0, -1, 1, 0, 2, -1, 2
};

const static std::array<std::array<std::array<int,2>, 5>, 4>& RotationOffsetL = RotationOffsetJ;
const static std::array<std::array<std::array<int,2>, 5>, 4>& RotationOffsetS = RotationOffsetJ;
const static std::array<std::array<std::array<int,2>, 5>, 4>& RotationOffsetZ = RotationOffsetJ;
const static std::array<std::array<std::array<int,2>, 5>, 4>& RotationOffsetT = RotationOffsetJ;

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
    bool rotate(Grid grid);

    //just rotate in the local coordinate
    void rotate();
    BlockPosition getPosition() const;
    bool moveLeft(Grid grid);
    bool moveRight(Grid grid);
    bool moveDown(Grid grid);

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

    glm::vec3 getColor() const;
    bool touch(Grid grid);

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
    glm::vec3 color;
    int startRow = 0;
    int startColumn = 0;
    unsigned int rotation = 0;
};
