//
// Created by amemiya7 on 2024/6/18.
//

#include "Block.h"
//column row offsets
const static std::array<std::array<std::array<int,2>, 5>, 4> RotationOffsetI {{
    {{
             {0, 0}, {-1, 0}, {2, 0}, {-1, 0}, {2, 0}
    }},
    {{
             {-1, 0}, {0, 0}, {0, 0}, {0, 1}, {0, 2}
    }},
    {{
             {-1, 1}, {1, 1}, {-2, 1}, {1, 0}, {-2, 0}
    }},
    {{
             {0, 1}, {0, 1}, {0, 1}, {0, -1}, {0, 2}
    }}

}};

const static std::array<std::array<std::array<int, 2>, 1>, 4> RotationOffsetO {{
        {{0, 0}},
        {{0, -1}},
        {{-1, -1}},
        {{-1, 0}}
}};

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


bool Block::rotate(const Grid& grid) {
    Block temBlock = *this;

    unsigned int nextRotation = (rotation + 1) % 4;
    temBlock.rotate();
    const auto temShape = temBlock.getShape();

    std::vector<std::pair<int, int>> offsets;
    if (type == BlockType::I) {
        for (int i = 0; i < RotationOffsetI[rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetI[nextRotation][i][0] - RotationOffsetI[rotation][i][0],
                                 RotationOffsetI[nextRotation][i][1] - RotationOffsetI[rotation][i][1]);
        }
    } else if (type == BlockType::O) {
        for (int i = 0; i < RotationOffsetO[rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetO[nextRotation][i][0] - RotationOffsetO[rotation][i][0],
                                 RotationOffsetO[nextRotation][i][1] - RotationOffsetO[rotation][i][1]);
        }
    } else {
        for (int i = 0; i < RotationOffsetJ[rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetJ[nextRotation][i][0] - RotationOffsetJ[rotation][i][0],
                                 RotationOffsetJ[nextRotation][i][1] - RotationOffsetJ[rotation][i][1]);
        }
    }
    for (int k = 0; k < offsets.size(); k++) {
        bool flag = true;
        for (int row = 0; row < rowSize; row++) {
            for (int column = 0; column < columnSize; column++) {
                if (!temShape[column][row]) continue;
                if (grid.isOccupied(startRow + row - offsets[k].second, startColumn + column - offsets[k].first)) {
                    flag = false;
                    break;
                }
            }
            if (!flag) break;
        }
        if (flag) {
            if (k == 4) srs = true;
            else srs = false;
            startRow -= offsets[k].second;
            startColumn -= offsets[k].first;
            this->rotate();
            lastMovement = Movement::Rotate;
            return true;
        }
    }
    return false;
}

Block::Block(BlockType type) : type(type) {
    if (type == BlockType::I) {
        shape = {
                {0, 0, 0, 0, 0},
                {0, 0, 1, 0, 0},
                {0, 0, 1, 0, 0},
                {0, 0, 1, 0, 0},
                {0, 0, 1, 0, 0}
        };
        startRow = 18;
        startColumn = 2;
        color = sf::Color{0x81d8d0ff};
    } else if (type == BlockType::J) {
        shape = {
                {0, 1, 1},
                {0, 1, 0},
                {0, 1, 0}
        };
        startRow = 19;
        startColumn = 3;
        color = sf::Color{0x00dcffff};
    } else if (type == BlockType::L) {
        shape = {
                {0, 1, 0},
                {0, 1, 0},
                {0, 1, 1}
        };
        startRow = 19;
        startColumn = 3;
        color = sf::Color{0xff9943ff};
    } else if (type == BlockType::O) {
        shape = {
                {0, 0, 0},
                {0, 1, 1},
                {0, 1, 1}
        };
        startRow = 19;
        startColumn = 3;
        color = sf::Color{0xe2c000ff};
    } else if (type == BlockType::S) {
        shape = {
                {0, 1, 0},
                {0, 1, 1},
                {0, 0, 1}
        };
        startRow = 19;
        startColumn = 3;
        color = sf::Color{0x80e5aaff};
    } else if (type == BlockType::T) {
        shape = {
                {0, 1, 0},
                {0, 1, 1},
                {0, 1, 0}
        };
        startRow = 19;
        startColumn = 3;
        color = sf::Color{0xc6aaeeff};
    } else if (type == BlockType::Z) {
        shape = {
                {0, 0, 1},
                {0, 1, 1},
                {0, 1, 0}
        };
        startRow = 19;
        startColumn = 3;
        color = sf::Color{0xff97cfff};
    }
    rowSize = shape[0].size();
    columnSize = shape.size();
}

void Block::rotate() {
    std::vector<std::vector<int>> newShape = shape;
    std::vector<std::pair<int, int>> offsets;
    for (int row = 0; row < rowSize; row++) {
        for (int column = 0; column < columnSize; column++) {
            newShape[column][row] = shape[shape.size() - 1 - row][column];
        }
    }
    shape = newShape;
    rotation = (rotation + 1) % 4;
}

//rotate counter-clockwise
void Block::rotateCounterClockwise() {
    std::vector<std::vector<int>> newShape = shape;
    for (int row = 0; row < rowSize; row++) {
        for (int column = 0; column < columnSize; column++) {
            newShape[column][row] = shape[row][columnSize - 1 - column];
        }
    }
    shape = newShape;
    rotation = (rotation + 3) % 4;
}

Block::Block(const Block &block) {
    shape = block.shape;
    color = block.color;
    type = block.type;
    rotation = block.rotation;
    startRow = block.startRow;
    startColumn = block.startColumn;
    rowSize = block.rowSize;
    columnSize = block.columnSize;
}

Block &Block::operator=(const Block &block) {
    color = block.color;
    shape = block.shape;
    type = block.type;
    rotation = block.rotation;
    startRow = block.startRow;
    startColumn = block.startColumn;
    rowSize = block.rowSize;
    columnSize = block.columnSize;
    return *this;
}

Block::Block(Block &&block) noexcept {
    shape = std::move(block.shape);
    type = block.type;
    color = block.color;
    rotation = block.rotation;
    startRow = block.startRow;
    startColumn = block.startColumn;
    rowSize = block.rowSize;
    columnSize = block.columnSize;
}

Block &Block::operator=(Block &&block) noexcept {
    shape = std::move(block.shape);
    color = block.color;
    type = block.type;
    rotation = block.rotation;
    startRow = block.startRow;
    startColumn = block.startColumn;
    rowSize = block.rowSize;
    columnSize = block.columnSize;
    return *this;
}

const std::vector<std::vector<int>>& Block::getShape() const {
    return shape;
}

bool Block::moveLeft(const Grid& grid) {
    if (!isValid(startRow, startColumn - 1, grid)) return false;
    lastMovement = Movement::Left;
    startColumn -= 1;
    return true;
}

bool Block::moveRight(const Grid& grid) {
    if (!isValid(startRow, startColumn + 1, grid)) return false;
    lastMovement = Movement::Right;
    startColumn += 1;
    return true;
}

bool Block::moveDown(const Grid& grid) {
    if (!isValid(startRow - 1, startColumn, grid)) return false;
    lastMovement = Movement::Down;
    startRow -= 1;
    return true;
}

BlockPosition Block::getPosition() const {
    return {startRow, startColumn};
}

bool Block::touch(const Grid& grid) const {
    return !isValid(startRow - 1, startColumn, grid);
}

sf::Color Block::getColor() const{
    return color;
}

bool Block::isValid(int startRow_, int startColumn_, const Grid &grid) const {
    const auto& temShape = this->getShape();
    for (int row = 0; row < rowSize; row++) {
        for (int column = 0; column < columnSize; column++) {
            if (temShape[column][row] == 1 && grid.isOccupied(startRow_ + row, startColumn_ + column)) {
                return false;
            }
        }
    }
    return true;
}

bool Block::rotateCounterClockwise(const Grid &grid) {
    Block temBlock = *this;

    unsigned int nextRotation = (rotation + 3) % 4;
    temBlock.rotateCounterClockwise();
    const auto temShape = temBlock.getShape();

    std::vector<std::pair<int, int>> offsets;
    if (type == BlockType::I) {
        for (int i = 0; i < RotationOffsetI[rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetI[nextRotation][i][0] - RotationOffsetI[rotation][i][0],
                                 RotationOffsetI[nextRotation][i][1] - RotationOffsetI[rotation][i][1]);
        }
    } else if (type == BlockType::O) {
        for (int i = 0; i < RotationOffsetO[rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetO[nextRotation][i][0] - RotationOffsetO[rotation][i][0],
                                 RotationOffsetO[nextRotation][i][1] - RotationOffsetO[rotation][i][1]);
        }
    } else {
        for (int i = 0; i < RotationOffsetJ[rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetJ[nextRotation][i][0] - RotationOffsetJ[rotation][i][0],
                                 RotationOffsetJ[nextRotation][i][1] - RotationOffsetJ[rotation][i][1]);
        }
    }
    for (int k = 0; k < offsets.size(); k++) {
        bool flag = true;
        for (int row = 0; row < rowSize; row++) {
            for (int column = 0; column < columnSize; column++) {
                if (!temShape[column][row]) continue;
                if (grid.isOccupied(startRow + row - offsets[k].second, startColumn + column - offsets[k].first)) {
                    flag = false;
                    break;
                }
            }
            if (!flag) break;
        }
        if (flag) {
            if (k == 4) srs = true;
            else srs = false;
            startRow -= offsets[k].second;
            startColumn -= offsets[k].first;
            lastMovement = Movement::Rotate;
            this->rotateCounterClockwise();
            return true;
        }
    }
    return false;
}

Block Block::getTransparentBlock() const {
    Block block = *this;
    block.color.a = 0x44;
    return block;
}

int Block::getStartColumn() const {
    return startColumn;
}

int Block::getStartRow() const {
    return startRow;
}

std::pair<int, int>
Block::getScreenPosition(int row, int column, int blockWidth, int stripeWidth, int screenWidth, int screenHeight,
                         int startPosX, int startPosY, bool reverseY, bool startFromLeftTop) const {
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

BlockType Block::getType() const {
    return type;
}

void Block::setStartColumn(int column) {
    startColumn = column;
}

bool Block::empty() const{
    if (type == BlockType::None) return true;
    return false;
}

Block::Block() {
    type = BlockType::None;
}

Movement Block::getLastMovement() const {
    return lastMovement;
}

bool Block::getSrs() const {
    return srs;
}

bool Block::checkMiniTSpin(const Grid &grid) const {
    if (rotation == 0) return grid.isOccupied(startRow + 2, startColumn) && grid.isOccupied(startRow + 2, startColumn + 2);
    if (rotation == 1) return grid.isOccupied(startRow + 2, startColumn + 2) && grid.isOccupied(startRow, startColumn + 2);
    if (rotation == 2) return grid.isOccupied(startRow, startColumn + 2) && grid.isOccupied(startRow, startColumn);
    if (rotation == 3) return grid.isOccupied(startRow, startColumn) && grid.isOccupied(startRow + 2, startColumn);
}
