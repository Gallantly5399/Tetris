//
// Created by amemiya7 on 2024/11/15.
//

#include "Utility.h"
#include <iostream>
#include <cassert>

const static std::array<std::array<std::array<int, 2>, 5>, 4> RotationOffsetI{{
                                                                                      {{
                                                                                               {0, 0}, {-1, 0}, {2, 0},
                                                                                               {-1, 0}, {2, 0}
                                                                                       }},
                                                                                      {{
                                                                                               {-1, 0}, {0, 0}, {0, 0},
                                                                                               {0, 1}, {0, 2}
                                                                                       }},
                                                                                      {{
                                                                                               {-1, 1}, {1, 1}, {-2, 1},
                                                                                               {1, 0}, {-2, 0}
                                                                                       }},
                                                                                      {{
                                                                                               {0, 1}, {0, 1}, {0, 1},
                                                                                               {0, -1}, {0, 2}
                                                                                       }}

                                                                              }};

const static std::array<std::array<std::array<int, 2>, 1>, 4> RotationOffsetO{{
                                                                                      {{0, 0}},
                                                                                      {{0, -1}},
                                                                                      {{-1, -1}},
                                                                                      {{-1, 0}}
                                                                              }};

const static std::array<std::array<std::array<int, 2>, 5>, 4> RotationOffsetJ{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 1, -1, 0, 2, 1, 2,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, -1, 0, -1, 1, 0, 2, -1, 2
};

const static std::array<std::array<std::array<int, 2>, 5>, 4> &RotationOffsetL = RotationOffsetJ;
const static std::array<std::array<std::array<int, 2>, 5>, 4> &RotationOffsetS = RotationOffsetJ;
const static std::array<std::array<std::array<int, 2>, 5>, 4> &RotationOffsetZ = RotationOffsetJ;
const static std::array<std::array<std::array<int, 2>, 5>, 4> &RotationOffsetT = RotationOffsetJ;

const uint32_t comboScores[] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5};

//TODO::read from config file
uint32_t utility::scoreTypeToInt(ScoreType scoreType) {
    if (scoreType == ScoreType::None) return 0;
    else if (scoreType == ScoreType::Single) return 0;
    else if (scoreType == ScoreType::Double) return 1;
    else if (scoreType == ScoreType::Triple) return 2;
    else if (scoreType == ScoreType::Tetris) return 4;
//    else if (scoreType == ScoreType::Combo) return 50;
    else if (scoreType == ScoreType::TSpinMiniNoLines) return 0;
    else if (scoreType == ScoreType::TSpinMiniSingle) return 0;
    else if (scoreType == ScoreType::TSpinMiniDouble) return 1;
    else if (scoreType == ScoreType::TSpinNoLines) return 0;
    else if (scoreType == ScoreType::TSpinSingle) return 2;
    else if (scoreType == ScoreType::TSpinDouble) return 4;
    else if (scoreType == ScoreType::TSpinTriple) return 6;
    else if (scoreType == ScoreType::SinglePerfectClear) return 4;
    else if (scoreType == ScoreType::DoublePerfectClear) return 4;
    else if (scoreType == ScoreType::TriplePerfectClear) return 4;
    else if (scoreType == ScoreType::TetrisPerfectClear) return 4;
    else if (scoreType == ScoreType::BackToBackTetrisPerfectClear) return 4;
    else return 0;
}

uint32_t utility::getScore(Grid &grid, const Block &block) {
    ScoreType scoreType = getScoreType(grid, block);
    uint32_t score = 0;
    score += scoreTypeToInt(scoreType);
    if (scoreType == ScoreType::TetrisPerfectClear || scoreType == ScoreType::DoublePerfectClear ||
        scoreType == ScoreType::TriplePerfectClear || scoreType == ScoreType::SinglePerfectClear) {
        score += 4;
    }
    if (isDifficultScore(scoreType)) {
        grid.backToBack = true;
        grid.comboCount++;
        score += grid.backToBack;
    } else if (scoreType == ScoreType::Single || scoreType == ScoreType::Double || scoreType == ScoreType::Triple) {
        grid.backToBack = false;
        grid.comboCount++;
        score += grid.backToBack;
    } else {
        assert(scoreTypeToInt(scoreType) <= 0);
        grid.comboCount = -1;
    }
    score += comboScores[std::min(std::max(grid.comboCount, 0), 10)];
    grid.clearLines();
    return score;
}

bool utility::TSpin(const Grid &grid, const Block &block) {
    if (block.getType() != BlockType::T || (block.getLastMovement() != Movement::Rotate && block.getLastMovement() != Movement::RotateCounterClockwise)) {
        return false;
    }
    const auto &shape = block.getShape();
    auto [startRow, startColumn] = block.getPosition();
    int count = grid.isOccupied(startRow, startColumn) + grid.isOccupied(startRow + 2, startColumn) +
                grid.isOccupied(startRow, startColumn + 2) + grid.isOccupied(startRow + 2, startColumn + 2);
    return count >= 3;
}

bool utility::isDifficultScore(const ScoreType &scoreType) {
    //TODO::read from config file
    if (scoreType == ScoreType::Tetris || scoreType == ScoreType::TSpinMiniSingle ||
        scoreType == ScoreType::TSpinMiniDouble ||
        scoreType == ScoreType::TSpinSingle || scoreType == ScoreType::TSpinDouble ||
        scoreType == ScoreType::TSpinTriple || scoreType == ScoreType::BackToBackTetrisPerfectClear ||
        scoreType == ScoreType::DoublePerfectClear || scoreType == ScoreType::TriplePerfectClear ||
        scoreType == ScoreType::TetrisPerfectClear || scoreType == ScoreType::SinglePerfectClear) {
        return true;
    }
    return false;
}

int utility::scoreForAI(Grid &grid, const Block &block) {
    return getScore(grid, block);
//    ScoreType scoreType = getScoreType(grid, block);
//    switch (scoreType) {
//        case ScoreType::Single:
//            return -2;
//        case ScoreType::None:
//            return 0;
//        case ScoreType::Double:
//            return -5;
//        case ScoreType::Triple:
//            return 1;
//        case ScoreType::Tetris:
//            return 4;
//        case ScoreType::TSpinMiniNoLines:
//            return -2;
//        case ScoreType::TSpinMiniSingle:
//            return -1;
//        case ScoreType::TSpinMiniDouble:
//            return 2;
//        case ScoreType::TSpinNoLines:
//            return -2;
//        case ScoreType::TSpinSingle:
//            return -2;
//        case ScoreType::TSpinDouble:
//            return 2;
//        case ScoreType::TSpinTriple:
//            return 4;
//        case ScoreType::SinglePerfectClear:
//            return 2;
//        case ScoreType::DoublePerfectClear:
//            return 4;
//        case ScoreType::TriplePerfectClear:
//            return 6;
//        case ScoreType::TetrisPerfectClear:
//            return 8;
//        case ScoreType::BackToBackTetrisPerfectClear:
//            return 10;
//        case ScoreType::Combo: //should not be here, maybe add log here
//            return 0;
//    }
}

ScoreType utility::getScoreType(const Grid &grid, const Block &block) {
    bool isSrs = block.getSrs();
    Grid tempGrid = grid;
    int lines = tempGrid.clearLines();
    ScoreType scoreType = ScoreType::None;
    if (tempGrid.empty()) { //perfect clear
        if (lines == 4 && tempGrid.lines() == 0) {
            if (grid.backToBack) scoreType = ScoreType::BackToBackTetrisPerfectClear;
            else scoreType = ScoreType::TetrisPerfectClear;
        } else if (lines == 3 && tempGrid.lines() == 0) {
            scoreType = ScoreType::TriplePerfectClear;
        } else if (lines == 2 && tempGrid.lines() == 0) {
            scoreType = ScoreType::DoublePerfectClear;
        } else if (lines == 1 && tempGrid.lines() == 0) {
            scoreType = ScoreType::SinglePerfectClear;
        }
    } else if (TSpin(grid, block)) {//T-spin
        //T-spin mini
        if (checkMiniTSpin(grid, block)) {
            if (lines == 0) scoreType = ScoreType::TSpinMiniNoLines;
            else if (lines == 1) scoreType = ScoreType::TSpinMiniSingle;
            else if (lines == 2) scoreType = ScoreType::TSpinMiniDouble;
            else if (lines == 3) scoreType = ScoreType::TSpinTriple;
        } else {
            //T-spin
            if (lines == 0) scoreType = ScoreType::TSpinNoLines;
            else if (lines == 1) scoreType = ScoreType::TSpinSingle;
            else if (lines == 2) scoreType = ScoreType::TSpinDouble;
            else if (lines == 3) scoreType = ScoreType::TSpinTriple;
        }
    } else {
        if (lines == 1) scoreType = ScoreType::Single;
        else if (lines == 2) scoreType = ScoreType::Double;
        else if (lines == 3) scoreType = ScoreType::Triple;
        else if (lines == 4) scoreType = ScoreType::Tetris;
    }
    return scoreType;
}

bool utility::checkMiniTSpin(const Grid &grid, const Block &block) {
    if (block.rotation == 0)
        return !grid.isOccupied(block.startRow + 2, block.startColumn) ||
               !grid.isOccupied(block.startRow + 2, block.startColumn + 2);
    if (block.rotation == 1)
        return !grid.isOccupied(block.startRow + 2, block.startColumn + 2) ||
               !grid.isOccupied(block.startRow, block.startColumn + 2);
    if (block.rotation == 2)
        return !grid.isOccupied(block.startRow, block.startColumn + 2) ||
               !grid.isOccupied(block.startRow, block.startColumn);
    if (block.rotation == 3)
        return !grid.isOccupied(block.startRow, block.startColumn) ||
               !grid.isOccupied(block.startRow + 2, block.startColumn);
}

bool utility::rotate(const Grid &grid, Block &block) {
    Block temBlock = block;

    unsigned int nextRotation = (block.rotation + 1) % 4;
    temBlock.rotate();
    const auto temShape = temBlock.getShape();

    std::vector<std::pair<int, int>> offsets;
    if (block.type == BlockType::I) {
        for (int i = 0; i < RotationOffsetI[block.rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetI[nextRotation][i][0] - RotationOffsetI[block.rotation][i][0],
                                 RotationOffsetI[nextRotation][i][1] - RotationOffsetI[block.rotation][i][1]);
        }
    } else if (block.type == BlockType::O) {
        for (int i = 0; i < RotationOffsetO[block.rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetO[nextRotation][i][0] - RotationOffsetO[block.rotation][i][0],
                                 RotationOffsetO[nextRotation][i][1] - RotationOffsetO[block.rotation][i][1]);
        }
    } else {
        for (int i = 0; i < RotationOffsetJ[block.rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetJ[nextRotation][i][0] - RotationOffsetJ[block.rotation][i][0],
                                 RotationOffsetJ[nextRotation][i][1] - RotationOffsetJ[block.rotation][i][1]);
        }
    }
    for (int k = 0; k < offsets.size(); k++) {
        bool flag = true;
        for (int row = 0; row < block.rowSize; row++) {
            for (int column = 0; column < block.columnSize; column++) {
                if (!temShape[column][row]) continue;
                if (grid.isOccupied(block.startRow + row - offsets[k].second,
                                    block.startColumn + column - offsets[k].first)) {
                    flag = false;
                    break;
                }
            }
            if (!flag) break;
        }
        if (flag) {
            if (k == 4) block.srs = true;
            else block.srs = false;
            block.startRow -= offsets[k].second;
            block.startColumn -= offsets[k].first;
            block.rotate();
            block.lastMovement = Movement::Rotate;
            block.totalMovements ++;
            return true;
        }
    }
    return false;
}

bool utility::moveLeft(const Grid &grid, Block &block) {
    if (!isValid(block.startRow, block.startColumn - 1, grid, block)) return false;
    block.lastMovement = Movement::Left;
    block.startColumn -= 1;
    block.totalMovements ++;
    return true;
}

bool utility::isValid(int startRow_, int startColumn_, const Grid &grid, const Block &block) {
    const auto &temShape = block.getShape();
    for (int row = 0; row < block.rowSize; row++) {
        for (int column = 0; column < block.columnSize; column++) {
            if (temShape[column][row] == 1 && grid.isOccupied(startRow_ + row, startColumn_ + column)) {
                return false;
            }
        }
    }
    return true;
}

bool utility::moveRight(const Grid &grid, Block &block) {
    if (!isValid(block.startRow, block.startColumn + 1, grid, block)) return false;
    block.lastMovement = Movement::Right;
    block.startColumn += 1;
    block.totalMovements ++;
    return true;
}

bool utility::moveDown(const Grid &grid, Block &block) {
    if (!isValid(block.startRow - 1, block.startColumn, grid, block)) return false;
    block.lastMovement = Movement::Down;
    block.startRow -= 1;
    return true;
}

bool utility::valid(const Grid &grid, const Block &block) {
    return isValid(block.startRow, block.startColumn, grid, block);
}

bool utility::touch(const Grid &grid, const Block &block) {
    return !isValid(block.startRow - 1, block.startColumn, grid, block);
}

bool utility::rotateCounterClockwise(const Grid &grid, Block &block) {
    Block temBlock = block;

    unsigned int nextRotation = (block.rotation + 3) % 4;
    temBlock.rotateCounterClockwise();
    const auto temShape = temBlock.getShape();

    std::vector<std::pair<int, int>> offsets;
    if (block.type == BlockType::I) {
        for (int i = 0; i < RotationOffsetI[block.rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetI[nextRotation][i][0] - RotationOffsetI[block.rotation][i][0],
                                 RotationOffsetI[nextRotation][i][1] - RotationOffsetI[block.rotation][i][1]);
        }
    } else if (block.type == BlockType::O) {
        for (int i = 0; i < RotationOffsetO[block.rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetO[nextRotation][i][0] - RotationOffsetO[block.rotation][i][0],
                                 RotationOffsetO[nextRotation][i][1] - RotationOffsetO[block.rotation][i][1]);
        }
    } else {
        for (int i = 0; i < RotationOffsetJ[block.rotation].size(); i++) {
            offsets.emplace_back(RotationOffsetJ[nextRotation][i][0] - RotationOffsetJ[block.rotation][i][0],
                                 RotationOffsetJ[nextRotation][i][1] - RotationOffsetJ[block.rotation][i][1]);
        }
    }
    for (int k = 0; k < offsets.size(); k++) {
        bool flag = true;
        for (int row = 0; row < block.rowSize; row++) {
            for (int column = 0; column < block.columnSize; column++) {
                if (!temShape[column][row]) continue;
                if (grid.isOccupied(block.startRow + row - offsets[k].second,
                                    block.startColumn + column - offsets[k].first)) {
                    flag = false;
                    break;
                }
            }
            if (!flag) break;
        }
        if (flag) {
            if (k == 4) block.srs = true;
            else block.srs = false;
            block.startRow -= offsets[k].second;
            block.startColumn -= offsets[k].first;
            block.totalMovements ++;
            block.lastMovement = Movement::RotateCounterClockwise;
            block.rotateCounterClockwise();
            return true;
        }
    }
    return false;
}

bool utility::move(const Grid &grid, Block &block, const Movement &movements) {
    switch (movements) {
        case ::Movement::Rotate:
            return rotate(grid, block);
            break;
        case ::Movement::Left:
            return moveLeft(grid, block);
            break;
        case ::Movement::Right:
            return moveRight(grid, block);
            break;
        case ::Movement::Down:
            return moveDown(grid, block);
            break;
        case ::Movement::RotateCounterClockwise:
            return rotateCounterClockwise(grid, block);
            break;
        case ::Movement::Hold:
            //TODO:: log here which should not be here
            break;
        case ::Movement::HardDrop:
            while (moveDown(grid, block));
            return true;
            break;
        case Movement::softDrop:
            break;
        case Movement::softDropEnd:
            break;
        case Movement::None:
            break;
    }
    return false;
}

void utility::move(const Grid &grid, Block &block, const std::vector<Movement> &movements) {
    for (auto movement: movements) {
        switch (movement) {
            case ::Movement::Rotate:
                rotate(grid, block);
                break;
            case ::Movement::Left:
                moveLeft(grid, block);
                break;
            case ::Movement::Right:
                moveRight(grid, block);
                break;
            case ::Movement::Down:
                while (moveDown(grid, block));
                break;
            case ::Movement::RotateCounterClockwise:
                rotateCounterClockwise(grid, block);
                break;
            case::Movement::Hold:
                //TODO:: log here which should not be here
                break;
            case ::Movement::HardDrop:
                while (moveDown(grid, block));
                break;
        }
    }
}

bool utility::isTspin(ScoreType scoreType) {
    if (scoreType == ScoreType::TSpinSingle || scoreType == ScoreType::TSpinDouble ||
        scoreType == ScoreType::TSpinTriple) {
        return true;
    }
    return false;
}

bool utility::isPerfectClear(ScoreType scoreType) {
    if (scoreType == ScoreType::SinglePerfectClear || scoreType == ScoreType::DoublePerfectClear ||
        scoreType == ScoreType::TriplePerfectClear || scoreType == ScoreType::TetrisPerfectClear) {
        return true;
    }
    return false;
}

bool utility::occupy(const Grid& grid, const Block& block) {
    const auto& shape = block.getShape();
    for (int column = 0; column < shape.size(); column++) {
        for (int row = 0; row < shape.size(); row++) {
            if (shape[column][row] == 1) {
                for (int i = block.startRow + row + 1; i < grid.getHeight(); i++) {
                    if (grid.isOccupied(i, block.startColumn + column)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> utility::getTSpinHole(const Grid &grid) {
    uint32_t halfTSpinDoubleHoles = 0, halfTSpinTripleHoles = 0, fullTSpinDoubleHoles = 0, fullTSpinTripleHoles = 0;
    static std::array<int, 6> tSpinDouble = {1, 0, 1, 0, 0, 0};
    static std::array<int, 12> tSpinTripleOne = {1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1};
    static std::array<int, 12> tSpinTripleTwo = {1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1};
    static std::array<int, 8> tSpinTripleOneJudge = {0, 0, 0, 1, 0, 0, 1, 1};
    static std::array<int, 8> tSpinTripleTwoJudge = {1, 0, 0, 0, 1, 1, 0, 0};


    for (int column = 1;column < grid.getWidth() - 1; column++) {
        for (int row = 1;row < grid.getHeight() - 1;row ++) {
            if (!grid.isOccupied(row, column)) {
                //x is the current position
                //0x0
                //101
                std::array<int, 6> t1;
                int count = 0;
                for (int i = row - 1; i <= row; i ++) {
                    for (int j = column - 1;j <= column + 1;j ++) {
                        t1[count++] = grid.isOccupied(i, j);
                    }
                }
                //100    001
                //0x0 or 0x0
                //101    101
                if (t1 == tSpinDouble) {
                    if ((grid.isOccupied(row + 1, column - 1) && !grid.isOccupied(row + 1, column)  && !grid.isOccupied(row + 1, column + 1))
                    ||  (!grid.isOccupied(row + 1, column - 1) && !grid.isOccupied(row + 1, column)  && grid.isOccupied(row + 1, column + 1))) {
                        fullTSpinDoubleHoles ++;
                    } else {
                        halfTSpinDoubleHoles ++;
                    }
                }
                //
                //1101
                //1x01
                //1101
                count = 0;
                std::array<int, 12> t2;
                for (int i = row - 1; i <= row + 1;i ++) {
                    for (int j = column - 1; j <= column + 2; j ++) {
                        t2[count ++] = grid.isOccupied(i, j);
                    }
                }
                //0011
                //0001
                //1101
                //1x01
                //1101
                count = 0;
                if (t2 == tSpinTripleOne) {
                    std::array<int, 8> tem;
                    for (int i = row + 2; i <= row + 3; i ++) {
                        for (int j = column - 1; j <= column + 2; j ++) {
                            tem[count ++] = grid.isOccupied(i, j);
                        }
                    }
                    if (tem == tSpinTripleOneJudge) {
                        fullTSpinTripleHoles ++;
                    } else {
                        halfTSpinTripleHoles ++;
                    }
                }
                count = 0;
                for (int i = row - 1;i <= row + 1;i ++) {
                    for (int j = column - 1;j <= column + 2;j ++) {
                        t2[count ++] = grid.isOccupied(i, j);
                    }
                }
                count = 0;
                if (t2 == tSpinTripleTwo) {
                    std::array<int, 8> tem;
                    for (int i = row + 2; i <= row + 3;i ++) {
                        for (int j = column - 1;j <= column + 2;j ++) {
                            tem[count++] = grid.isOccupied(i, j);
                        }
                    }
                    if (tem == tSpinTripleTwoJudge) {
                        fullTSpinTripleHoles ++;
                    } else {
                        halfTSpinTripleHoles ++;
                    }
                }
                //1100
                //1000
                //1011
                //1x01
                //1011

            }
        }
    }

    return {halfTSpinDoubleHoles, fullTSpinDoubleHoles,  halfTSpinTripleHoles, fullTSpinTripleHoles};
}

