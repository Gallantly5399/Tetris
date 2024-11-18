//
// Created by amemiya7 on 2024/6/18.
//

#include "Block.h"
//column row offsets



Block::Block(BlockType type) : type(type) {
    if (type == BlockType::None) return;
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
    isNotSent = true;
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

//Block::Block(const Block &block) {
//    shape = block.shape;
//    color = block.color;
//    type = block.type;
//    rotation = block.rotation;
//    startRow = block.startRow;
//    startColumn = block.startColumn;
//    rowSize = block.rowSize;
//    columnSize = block.columnSize;
//}
//
//Block &Block::operator=(const Block &block) {
//    color = block.color;
//    shape = block.shape;
//    type = block.type;
//    rotation = block.rotation;
//    startRow = block.startRow;
//    startColumn = block.startColumn;
//    rowSize = block.rowSize;
//    columnSize = block.columnSize;
//    return *this;
//}
//
//Block::Block(Block &&block) noexcept {
//    shape = std::move(block.shape);
//    type = block.type;
//    color = block.color;
//    rotation = block.rotation;
//    startRow = block.startRow;
//    startColumn = block.startColumn;
//    rowSize = block.rowSize;
//    columnSize = block.columnSize;
//}
//
//Block &Block::operator=(Block &&block) noexcept {
//    shape = std::move(block.shape);
//    color = block.color;
//    type = block.type;
//    rotation = block.rotation;
//    startRow = block.startRow;
//    startColumn = block.startColumn;
//    rowSize = block.rowSize;
//    columnSize = block.columnSize;
//    return *this;
//}

const std::vector<std::vector<int>>& Block::getShape() const {
    return shape;
}

BlockPosition Block::getPosition() const {
    return {startRow, startColumn};
}


sf::Color Block::getColor() const{
    return color;
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



int Block::getRotation() const {
    return rotation;
}

std::ostream &operator<<(std::ostream &os, const Movement &movement) {
    switch (movement) {
        case Movement::Down:
            os << "Down";
            break;
        case Movement::Left:
            os << "Left";
            break;
        case Movement::Right:
            os << "Right";
            break;
        case Movement::Rotate:
            os << "Rotate";
            break;
        case Movement::RotateCounterClockwise:
            os << "RotateCounterClockwise";
            break;
        case Movement::Hold:
            os << "Hold";
            break;
        case Movement::HardDrop:
            os << "HardDrop";
            break;
    }
    return os;

}
std::ostream &operator<<(std::ostream &os, const BlockType &blockType) {
    switch (blockType) {
        case BlockType::I:
            os << "I";
            break;
        case BlockType::J:
            os << "J";
            break;
        case BlockType::L:
            os << "L";
            break;
        case BlockType::O:
            os << "O";
            break;
        case BlockType::S:
            os << "S";
            break;
        case BlockType::T:
            os << "T";
            break;
        case BlockType::Z:
            os << "Z";
            break;
        case BlockType::None:
            os << "None";
            break;
    }
}