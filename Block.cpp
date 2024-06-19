//
// Created by amemiya7 on 2024/6/18.
//

#include "Block.h"

bool Block::rotate(Grid grid) {
    Block temBlock = *this;
    unsigned int nextRotation = (rotation + 1) % 4;
    temBlock.rotate();
    const auto temShape = temBlock.getShape();

    std::vector<std::pair<int, int>> offsets;
    if (type == BlockType::I) {
        for (int i = 0; i < RotationOffsetI.size(); i++) {
            offsets.emplace_back(RotationOffsetI[i][nextRotation][0] - RotationOffsetI[i][rotation][0],
                                 RotationOffsetI[i][nextRotation][1] - RotationOffsetI[i][rotation][1]);
        }
    } else if (type == BlockType::O) {
        for (int i = 0; i < RotationOffsetO.size(); i++) {
            offsets.emplace_back(RotationOffsetO[i][nextRotation][0] - RotationOffsetO[i][rotation][0],
                                 RotationOffsetO[i][nextRotation][1] - RotationOffsetO[i][rotation][1]);
        }
    } else {
        for (int i = 0; i < RotationOffsetJ.size(); i++) {
            offsets.emplace_back(RotationOffsetJ[i][nextRotation][0] - RotationOffsetJ[i][rotation][0],
                                 RotationOffsetJ[i][nextRotation][1] - RotationOffsetJ[i][rotation][1]);
        }
    }
    for (int k = 0; k < offsets.size(); k++) {
        bool flag = true;
        for (int i = 0; i < temShape.size(); i++) {
            for (int j = 0; j < temShape[i].size(); j++) {
                if (grid.isOccupied(startRow + offsets[k].first, startColumn + offsets[k].second)) {
                    flag = false;
                    break;
                }
            }
            if (!flag) break;
        }
        if (flag) {
            startRow += offsets[k].first;
            startColumn += offsets[k].second;
            this->rotate();
            return true;
        }
    }
    return false;
}

Block::Block(BlockType type) : type(type) {
    if (type == BlockType::I) {
        shape = {{0, 0, 0, 0, 0},
                 {0, 0, 0, 0, 0},
                 {0, 1, 1, 1, 1},
                 {0, 0, 0, 0, 0},
                 {0, 0, 0, 0, 0}};
        startRow = 18;
        startColumn = 2;
    } else if (type == BlockType::J) {
        shape = {{0, 0, 0},
                 {1, 1, 1},
                 {1, 0, 0}};
        startRow = 19;
        startColumn = 3;
    } else if (type == BlockType::L) {
        shape = {{0, 0, 0},
                 {1, 1, 1},
                 {0, 0, 1}};
        startRow = 19;
        startColumn = 3;
    } else if (type == BlockType::O) {
        shape = {{0, 0, 0},
                 {0, 1, 1},
                 {0, 1, 1}};
        startRow = 19;
        startColumn = 3;
    } else if (type == BlockType::S) {
        shape = {{0, 0, 0},
                 {1, 1, 0},
                 {0, 1, 1}};
        startRow = 19;
        startColumn = 3;
    } else if (type == BlockType::T) {
        shape = {{0, 0, 0},
                 {1, 1, 1},
                 {0, 1, 0}};
        startRow = 19;
        startColumn = 3;
    } else if (type == BlockType::Z) {
        shape = {{0, 0, 0},
                 {0, 1, 1},
                 {1, 1, 0}};
        startRow = 19;
        startColumn = 3;
    }
}

void Block::rotate() {
    std::vector<std::vector<int>> newShape = shape;
    std::vector<std::pair<int, int>> offsets;
    for (int i = 0; i < shape.size(); i++) {
        for (int j = 0; j < shape[i].size(); j++) {
            newShape[i][j] = shape[j][shape.size() - 1 - i];
        }
    }
    shape = newShape;
    rotation = (rotation + 1) % 4;
}

Block::Block(const Block &block) {
    shape = block.shape;
    type = block.type;
    rotation = block.rotation;
    startRow = block.startRow;
    startColumn = block.startColumn;
}

Block &Block::operator=(const Block &block) {
    shape = block.shape;
    type = block.type;
    rotation = block.rotation;
    startRow = block.startRow;
    startColumn = block.startColumn;
    return *this;
}

Block::Block(Block &&block) noexcept {
    shape = std::move(block.shape);
    type = block.type;
    rotation = block.rotation;
    startRow = block.startRow;
    startColumn = block.startColumn;
}

Block &Block::operator=(Block &&block) noexcept {
    shape = std::move(block.shape);
    type = block.type;
    rotation = block.rotation;
    startRow = block.startRow;
    startColumn = block.startColumn;
    return *this;
}

const std::vector<std::vector<int>>& Block::getShape() const {
    return shape;
}

bool Block::moveLeft(Grid grid) {
    int currentRow = startRow;
    int currentColumn = startColumn - 1;
    const auto temShape = this->getShape();
    for (int i = 0; i < temShape.size(); i++) {
        for (int j = 0; j < temShape[i].size(); j++) {
            if (temShape[i][j] == 1 && grid.isOccupied(currentRow + i, currentColumn + j)) {
                return false;
            }
        }
    }
    startColumn -= 1;
    return true;
}

bool Block::moveRight(Grid grid) {
    int currentRow = startRow;
    int currentColumn = startColumn + 1;
    const auto temShape = this->getShape();
    for (int i = 0; i < temShape.size(); i++) {
        for (int j = 0; j < temShape[i].size(); j++) {
            if (temShape[i][j] == 1 && grid.isOccupied(currentRow + i, currentColumn + j)) {
                return false;
            }
        }
    }
    startColumn += 1;
    return true;
}

bool Block::moveDown(Grid grid) {
    const auto temShape = this->getShape();
    int currentRow = startRow -1;
    int currentColumn = startColumn;
    for (int i = 0; i < temShape.size(); i++) {
        for (int j = 0; j < temShape[i].size(); j++) {
            if (temShape[i][j] == 1 && grid.isOccupied(currentRow + i, currentColumn + j)) {
                return false;
            }
        }
    }
    startRow -= 1;
    return true;
}

BlockPosition Block::getPosition() const {
    return {startRow, startColumn};
}

bool Block::touch(Grid grid) {
    const auto temShape = this->getShape();
    return false;
}

glm::vec3 Block::getColor() const{
    if (type == BlockType::I) {

    }
}