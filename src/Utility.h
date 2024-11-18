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
    static void insertBlock(Grid &grid, const Block &block) {

        grid.lastBlock = block;
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
    bool checkMiniTSpin(const Grid& grid, const Block& block);
    bool rotate(const Grid& grid, Block& block);//right rotate
    bool moveLeft(const Grid& grid, Block& block);
    bool moveRight(const Grid& grid, Block& block);
    bool moveDown(const Grid& grid, Block& block);
    bool isValid(int startRow_, int startColumn_, const Grid& grid, const Block& block);
    bool valid(const Grid& grid, const Block& block);
    bool touch(const Grid& grid, const Block& block);
    bool rotateCounterClockwise(const Grid& grid, Block& block);
    void move(const Grid& grid, Block& block, const std::vector<Movement>& movements);
    bool move(const Grid& grid, Block& block, const Movement& movement);
}
