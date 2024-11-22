//
// Created by amemiya7 on 2024/11/13.
//

#pragma once
#include "Grid.h"
#include "Block.h"
//TODO:: add message system
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
inline std::ostream &operator<<(std::ostream &os, const ScoreType &scoreType) {
    switch (scoreType) {
        case ScoreType::None:
            os << "None";
            break;
        case ScoreType::Single:
            os << "Single";
            break;
        case ScoreType::Double:
            os << "Double";
            break;
        case ScoreType::Triple:
            os << "Triple";
            break;
        case ScoreType::Tetris:
            os << "Tetris";
            break;
        case ScoreType::Combo:
            os << "Combo";
            break;
        case ScoreType::TSpinMiniNoLines:
            os << "TSpinMiniNoLines";
            break;
        case ScoreType::TSpinMiniSingle:
            os << "TSpinMiniSingle";
            break;
        case ScoreType::TSpinMiniDouble:
            os << "TSpinMiniDouble";
            break;
        case ScoreType::TSpinNoLines:
            os << "TSpinNoLines";
            break;
        case ScoreType::TSpinSingle:
            os << "TSpinSingle";
            break;
        case ScoreType::TSpinDouble:
            os << "TSpinDouble";
            break;
        case ScoreType::TSpinTriple:
            os << "TSpinTriple";
            break;
        case ScoreType::SinglePerfectClear:
            os << "SinglePerfectClear";
            break;
        case ScoreType::DoublePerfectClear:
            os << "DoublePerfectClear";
            break;
        case ScoreType::TriplePerfectClear:
            os << "TriplePerfectClear";
            break;
        case ScoreType::TetrisPerfectClear:
            os << "TetrisPerfectClear";
            break;
        case ScoreType::BackToBackTetrisPerfectClear:
            os << "BackToBackTetrisPerfectClear";
            break;
    }
}
enum class TSpinHole {
    None,
    Single,
    Double,
    Triple,
};
enum class Message {
    BlockTouchGround,
};
struct MessageData {
    std::vector<Message> data;
    int readIndex = 0;
    int writeIndex = 0;

    void write(Message message) {
        data[writeIndex] = message;
        writeIndex++;
        if (writeIndex == data.size()) writeIndex = 0;
    }

    [[nodiscard]] Message read() {
        auto message = data[readIndex];
        readIndex++;
        if (readIndex == data.size()) readIndex = 0;
        return message;
    }

    void clear() {
        readIndex = 0;
        writeIndex = 0;
    }

    [[nodiscard]] bool empty() const {
        return readIndex == writeIndex;
    }
};


//TODO:: srs judge maybe is not correct
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
    bool isTspin(ScoreType scoreType);
    bool occupy(const Grid& grid, const Block& block); //check if there is block above the block
    bool isPerfectClear(ScoreType scoreType);
    std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> getTSpinHole(const Grid& grid); //half(TSpin double and TSpin triple) holes amount and full(TSpin single, TSpin double, TSpin triple) holes amount
    ;
}
