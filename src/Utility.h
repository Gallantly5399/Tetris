//
// Created by amemiya7 on 2024/11/13.
//

#pragma once
#include "Grid.h"
#include "Block.h"
enum class ScoreType {
    None = 0,
    Single,
    Double,
    Triple,
    Tetris,
    Combo,
    TSpinMiniNoLines,
    TSpinMiniSingle,
    TSpinMiniDouble,
    TSpinNoLines,
    TSpinSingle,
    TSpinDouble,
    TSpinTriple,
    SinglePerfectClear,
    DoublePerfectClear,
    TriplePerfectClear,
    TetrisPerfectClear,
    BackToBackTetrisPerfectClear,
};

namespace utility {
//const static std::array<std::array<std::array<int,2>, 5>, 4> RotationOffsetI {{
//                                                                                      {{
//                                                                                               {0, 0}, {-1, 0}, {2, 0}, {-1, 0}, {2, 0}
//                                                                                       }},
//                                                                                      {{
//                                                                                               {-1, 0}, {0, 0}, {0, 0}, {0, 1}, {0, 2}
//                                                                                       }},
//                                                                                      {{
//                                                                                               {-1, 1}, {1, 1}, {-2, 1}, {1, 0}, {-2, 0}
//                                                                                       }},
//                                                                                      {{
//                                                                                               {0, 1}, {0, 1}, {0, 1}, {0, -1}, {0, 2}
//                                                                                       }}
//
//                                                                              }};
//
//const static std::array<std::array<std::array<int, 2>, 1>, 4> RotationOffsetO {{
//                                                                                       {{0, 0}},
//                                                                                       {{0, -1}},
//                                                                                       {{-1, -1}},
//                                                                                       {{-1, 0}}
//                                                                               }};
//
//const static std::array<std::array<std::array<int,2>, 5>, 4> RotationOffsetJ {
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        0, 0, 1, 0, 1, -1, 0, 2, 1, 2,
//        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//        0, 0, -1, 0, -1, 1, 0, 2, -1, 2
//};
//
//const static std::array<std::array<std::array<int,2>, 5>, 4>& RotationOffsetL = RotationOffsetJ;
//const static std::array<std::array<std::array<int,2>, 5>, 4>& RotationOffsetS = RotationOffsetJ;
//const static std::array<std::array<std::array<int,2>, 5>, 4>& RotationOffsetZ = RotationOffsetJ;
//const static std::array<std::array<std::array<int,2>, 5>, 4>& RotationOffsetT = RotationOffsetJ;

//bool rotate(const Grid& grid, Block& block) {
//    Block temBlock = block;
//
//    unsigned int nextRotation = (block.rotation + 1) % 4;
//    temBlock.rotate();
//    const auto temShape = temBlock.getShape();
//
//    std::vector<std::pair<int, int>> offsets;
//    if (block.type == BlockType::I) {
//        for (int i = 0; i < RotationOffsetI[block.rotation].size(); i++) {
//            offsets.emplace_back(RotationOffsetI[nextRotation][i][0] - RotationOffsetI[block.rotation][i][0],
//                                 RotationOffsetI[nextRotation][i][1] - RotationOffsetI[block.rotation][i][1]);
//        }
//    } else if (block.type == BlockType::O) {
//        for (int i = 0; i < RotationOffsetO[block.rotation].size(); i++) {
//            offsets.emplace_back(RotationOffsetO[nextRotation][i][0] - RotationOffsetO[block.rotation][i][0],
//                                 RotationOffsetO[nextRotation][i][1] - RotationOffsetO[block.rotation][i][1]);
//        }
//    } else {
//        for (int i = 0; i < RotationOffsetJ[block.rotation].size(); i++) {
//            offsets.emplace_back(RotationOffsetJ[nextRotation][i][0] - RotationOffsetJ[block.rotation][i][0],
//                                 RotationOffsetJ[nextRotation][i][1] - RotationOffsetJ[block.rotation][i][1]);
//        }
//    }
//    for (int k = 0; k < offsets.size(); k++) {
//        bool flag = true;
//        for (int row = 0; row < block.rowSize; row++) {
//            for (int column = 0; column < block.columnSize; column++) {
//                if (!temShape[column][row]) continue;
//                if (grid.isOccupied(block.startRow + row - offsets[k].second, block.startColumn + column - offsets[k].first)) {
//                    flag = false;
//                    break;
//                }
//            }
//            if (!flag) break;
//        }
//        if (flag) {
//            if (k == 4) block.srs = true;
//            else block.srs = false;
//            block.startRow -= offsets[k].second;
//            block.startColumn -= offsets[k].first;
//            block.rotate();
//            block.lastMovement = Movement::Rotate;
//            return true;
//        }
//    }
//    return false;
//}

    static void insertBlock(Grid &grid, const Block &block) {
        const auto &shape = block.getShape();
        const auto &color = block.getColor();
        int startRow = block.getStartRow();
        int startColumn = block.getStartColumn();
        for (int column = 0; column < shape.size(); column++) {
            for (int row = 0; row < shape.size(); row++) {
                if (shape[column][row] == 1) {
                    grid.fill(startRow + row, startColumn + column, color);
                }
            }
        }
    }
    bool TSpin(const Grid &grid, const Block &block);
    bool isDifficultScore(const ScoreType &scoreType);
    uint32_t scoreTypeToInt(ScoreType scoreType);
    uint32_t getScore(Grid &grid, const Block &block);
    int scoreForAI(Grid &grid, const Block &block);
    ScoreType getScoreType(const Grid &grid, const Block &block);
}