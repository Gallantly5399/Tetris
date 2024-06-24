//
// Created by amemiya7 on 2024/6/23.
//

#include "Generator.h"

Generator::Generator() {
    std::random_device rd;
    gen = std::mt19937(rd());
    std::shuffle(nextBlocks.begin(), nextBlocks.begin() + 7, gen);
    std::shuffle(nextBlocks.begin() + 7, nextBlocks.begin() + 14, gen);
}

Block Generator::nextBlock() {
    if (index == 7) {
        std::shuffle(nextBlocks.begin(), nextBlocks.begin() + 7, gen);
    }
    if (index == nextBlocks.size()) {
        index = 0;
        std::shuffle(nextBlocks.begin() + 7, nextBlocks.begin() + 14, gen);
    }
    return Block(static_cast<BlockType>(nextBlocks[index++ % 14]));
}

std::vector<Block> Generator::seeNextBlocks(int count) const{
    std::vector<Block> blocks;
    for (int i = 0; i < count; i++) {
        blocks.push_back(Block(static_cast<BlockType>(nextBlocks[(index + i) % 14])));
    }
    return blocks;
}

void Generator::clear() {
    index = 0;
    std::shuffle(nextBlocks.begin(), nextBlocks.begin() + 7, gen);
    std::shuffle(nextBlocks.begin() + 7, nextBlocks.begin() + 14, gen);
}
