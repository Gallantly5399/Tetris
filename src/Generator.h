//
// Created by amemiya7 on 2024/6/23.
//

#pragma once
#include <random>
#include "Block.h"
class Generator {
public:
    Generator();
    Block nextBlock();
    std::vector<Block> seeNextBlocks(int count) const;
    void clear();
private:
    std::mt19937 gen;
    int index = 0;
    std::array<int, 14> nextBlocks = {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6};
};
