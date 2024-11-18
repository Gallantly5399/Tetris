//
// Created by amemiya7 on 2024/6/23.
//

#include "Generator.h"
Generator::Generator() {
    gen = std::mt19937(std::random_device{}());
    for (int i = 0; i < 20; i++) {
        blockSets.emplace_back(gen);
    }
}

Block Generator::nextBlock() {
    if (index % 7 == 0) {
        blockSets.pop_front();
    }
    if (blockSets.empty()) {
        blockSets.emplace_back(gen);
    }
    return Block{blockSets.front().blocks[index++ % 7]};
}

std::vector<Block> Generator::seeNextBlocks(int count) {
    int virtualIndex = index;
    int blockSetIndex = 0;
    std::vector<Block> blocks(count);
    for (auto &block: blocks) {
        if (virtualIndex % 7 == 0) {
            blockSetIndex++;
            if (blockSetIndex == blockSets.size()) {
                blockSets.emplace_back(gen);
            }
        }
        block = Block{blockSets[blockSetIndex].blocks[virtualIndex++ % 7]};
    }
    return blocks;
}

void Generator::clear() {
    index = 0;
    blockSets.clear();
    for (int i = 0; i < 20; i++) {
        blockSets.emplace_back(gen);
    }
}

Generator::BlockSet::BlockSet(std::mt19937 &gen) {
    for (int i = 0; i < blocks.size(); i++) {
        blocks[i] = {static_cast<BlockType>(i)};
    }
    std::shuffle(blocks.begin(), blocks.end(), gen);
}
