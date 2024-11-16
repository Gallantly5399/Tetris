//
// Created by amemiya7 on 2024/6/23.
//

#pragma once
#include <random>
#include "Block.h"
#include <queue>
#include <array>

class Generator {
public:
    Generator();
    [[nodiscard]] Block nextBlock();
    [[nodiscard]] std::vector<Block> seeNextBlocks(int count);
    void clear();
private:
    struct BlockSet {
        std::array<BlockType, 7> blocks;
        BlockSet() = delete;
        BlockSet(std::mt19937& gen);
    };
    std::mt19937 gen;
    uint32_t index = 0;
    std::deque<BlockSet> blockSets;
};
