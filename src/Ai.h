//
// Created by amemiya7 on 2024/6/24.
//

#pragma once

#include "Utility.h"
#include "Grid.h"
#include <random>
#include "Block.h"
#include <queue>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>

//TODO::Pack the movement data
struct MovementData {
    std::vector<Movement> data;
    int readIndex = 0;
    int writeIndex = 0;

    void write(Movement movement) {
        data[writeIndex] = movement;
        writeIndex++;
        if (writeIndex == data.size()) writeIndex = 0;
//        std::cout << "Write Movement: " << movement << std::endl;
    }

    [[nodiscard]] Movement read() {
        Movement movement = data[readIndex];
        readIndex++;
        if (readIndex == data.size()) readIndex = 0;
        return movement;
    }

    void clear() {
        readIndex = 0;
        writeIndex = 0;
    }

    [[nodiscard]] bool empty() const {
        return readIndex == writeIndex;
    }
};


class AI {
public:
    struct Candidate {
        int64_t score; //score for the grid
        std::vector<Movement> movements; //movements for the first block
        Grid grid;
        Block bestBlock;

        Candidate(int64_t _score, const Grid &_grid, const std::vector<Movement> &_movements) : score(_score),
                                                                                                grid(_grid),
                                                                                                movements(_movements) {}
//        Candidate(double _score, const std::vector<Block>& _blocks, const Grid &_grid) : score(_score), grid(_grid), blocks(_blocks) {}
    };

    AI(int32_t heightWeight, int32_t scoreWeight, int32_t holesWeight, int32_t bumpinessWeight, int32_t emptyLineWeight,
       int32_t backToBackWeight, int32_t comboWeight, int32_t tetrisWeight, int32_t perfectClearWeight,
       int32_t tSpinWeight, int32_t singleWeight, int32_t doubleWeight, int32_t tripleWeight, int32_t highestWeight) {
        this->heightWeight = heightWeight;
        this->scoreWeight = scoreWeight;
        this->holesWeight = holesWeight;
        this->bumpinessWeight = bumpinessWeight;
        this->emptyLineWeight = emptyLineWeight;
        this->backToBackWeight = backToBackWeight;
        this->comboWeight = comboWeight;
        this->tetrisWeight = tetrisWeight;
        this->perfectClearWeight = perfectClearWeight;
        this->tSpinWeight = tSpinWeight;
        this->singleWeight = singleWeight;
        this->doubleWeight = doubleWeight;
        this->tripleWeight = tripleWeight;
        this->tetrisWeight = tetrisWeight;
        this->highestWeight = highestWeight;
    }

    void best(MovementData &movements) {
        while (!isStop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (blockQueue.empty()) continue;
            auto [workingPieces, grid] = blockQueue.front();
            blockQueue.pop();
            auto [bestScore, bestMovement, bestGrid, bestBlock] = this->best_(grid, workingPieces);
            for (auto movement: bestMovement) {
                movements.write(movement);
            }
//            Block temBlock = workingPieces[0];
//            Block bestBlock = bestBlocks[1];
//            std::cout << "bestScore: " << bestScore << '\n';
//
//            if (bestBlock.getType() == BlockType::None) {
//                movements.write(Movement::Down);
//                continue;
//            }
//            if (bestMovement.size() == 1 && bestMovement[0] == Movement::Hold) {
//                movements.write(Movement::Hold);
//                continue;
//            }
//            int rotate_count = 0;
//            while (temBlock.getRotation() != bestBlock.getRotation()) {
//                rotate_count ++;
//                utility::rotate(grid, temBlock);
//            }
//            Movement movement;
//            if (rotate_count <= 2) {
//                movement = Movement::Rotate;
//            } else {
//                rotate_count = 4 - rotate_count;
//                movement = Movement::RotateCounterClockwise;
//            }
//            for (int i = 0;i < rotate_count; i++) {
//
//                movements.write(movement);
//            }
//
//            auto [aiRow, aiColumn] = bestBlock.getPosition();
//            auto [blockRow, blockColumn] = temBlock.getPosition();
//            if (aiColumn > blockColumn) {
//                for (int i = 0; i < aiColumn - blockColumn; i++) movements.write(Movement::Right);
//            } else if (aiColumn < blockColumn) {
//                for (int i = 0; i < blockColumn - aiColumn; i++) movements.write(Movement::Left);
//            }
//            movements.write(Movement::Down);
        }
    }

    void stop() {
        isStop = true;
    }

    void add(const std::vector<Block> &blocks, const Grid &grid) {
        blockQueue.emplace(blocks, grid);
//        std::cout << "Add block to queue\n";
    }

    Candidate best_(const Grid &grid, const std::vector<Block> &workingPieces) {
        std::vector<Candidate> candidates, totalCandidates;

        candidates.emplace_back(evaluateFirst(grid), grid, std::vector<Movement>{});
        for (int workingIndex = 0; workingIndex < 10; workingIndex++) {
            for (auto &[score, movements, workingGrid, _bestBlock_nouse]: candidates) {
                //TODO::add soft drop
                //TODO::add hold
                const Block &workingPiece = workingPieces[workingIndex];
                Grid temGrid = workingGrid;
                if (temGrid.holdable) {
                    temGrid.holdable = false;
                    Block holdBlock = temGrid.holdBlock;
                    temGrid.holdBlock = workingPiece;
                    if (workingIndex == 0) {
                        movements.push_back(Movement::Hold);
                    }
                    if (holdBlock.empty()) {
                        totalCandidates.push_back(Candidate(score, temGrid, movements));

                    } else {
                        search(temGrid, holdBlock, totalCandidates, movements);
                    }
                }
                search(workingGrid, workingPiece, totalCandidates, movements, workingIndex == 0);
                if (isStop) return candidates[0];
            }
            std::sort(totalCandidates.begin(), totalCandidates.end(), [](const Candidate &a, const Candidate &b) {
                return a.score > b.score;
            });
            candidates.clear();
            for (int i = 0; i < std::min(10, (int) totalCandidates.size()); i++) {
                candidates.push_back(totalCandidates[i]);
            }
            totalCandidates.clear();
        }
        return candidates[0];
    }

    [[nodiscard]] int64_t evaluateFirst(const Grid &grid) const {
        if (grid.exceed()) return std::numeric_limits<int64_t>::lowest();
        ScoreType scoreType = utility::getScoreType(grid, grid.lastBlock);
        int64_t currentScore =
                (int64_t) -heightWeight * grid.aggregateHeight() - holesWeight * grid.holes()
                - bumpinessWeight * grid.bumpiness() + emptyLineWeight * grid.sumOfContinuousEmptyLines()
                + backToBackWeight * grid.backToBack + comboWeight * grid.comboCount + tetrisWeight * (scoreType == ScoreType::Tetris)
                + tSpinWeight * utility::isTspin(scoreType) + perfectClearWeight * utility::isPerfectClear(scoreType)
                - singleWeight * (scoreType == ScoreType::Single) - doubleWeight * (scoreType == ScoreType::Double)
                + tripleWeight * (scoreType == ScoreType::Triple)
                - highestWeight * grid.highest();
        return currentScore;
    }

    [[nodiscard]] int64_t evaluate(Grid &grid) const {
        int64_t score = evaluateFirst(grid);
        if (!grid.lastBlock.empty())
            score += scoreWeight * utility::scoreForAI(grid, grid.lastBlock);
        return score;
    }

private:
    ///search for the best movement for the block
    ///block is the block to be inserted
    ///candidates is the vector to store the candidates
    void
    search(const Grid &grid, const Block &block, std::vector<Candidate> &candidates, std::vector<Movement> bestMovement,
           bool record = false) const {
        for (int rotation = 0; rotation < 4; rotation++) {
            std::vector<Movement> movements;
            Block _piece = block;
            if (rotation <= 2) {
                for (int i = 0; i < rotation; i++) {
                    movements.push_back(Movement::Rotate);
                }
            } else {
                for (int i = 0; i < 4 - rotation; i++) {
                    movements.push_back(Movement::RotateCounterClockwise);
                }
            }
            for (int i = 0; i < rotation; i++) {
                utility::rotate(grid, _piece);
//                movements.push_back(Movement::Rotate);
            }

            while (utility::moveLeft(grid, _piece)) {
                movements.push_back(Movement::Left);
            };
            do {
                if (isStop) return;
                Block _pieceSet = _piece;
                while (utility::moveDown(grid, _pieceSet));

                Grid _grid = grid;
                utility::insertBlock(_grid, _pieceSet);

                int64_t currentScore = evaluate(_grid);
                auto temMovement = movements;
                temMovement.push_back(Movement::HardDrop);
                if (record) candidates.emplace_back(currentScore, _grid, temMovement);
                else candidates.emplace_back(currentScore, _grid, bestMovement);
                if (!movements.empty() && movements.back() == Movement::Left) movements.pop_back();
                else movements.push_back(Movement::Right);
            } while (utility::moveRight(grid, _piece));
        }
    }

    std::queue<std::pair<std::vector<Block>, Grid>> blockQueue;
    bool isStop = false;
    //millisecond time for per block
    int32_t heightWeight;
    int32_t scoreWeight;
    int32_t holesWeight;
    int32_t bumpinessWeight;
    int32_t emptyLineWeight;
    int32_t backToBackWeight;
    int32_t comboWeight;
    int32_t tetrisWeight;
    int32_t perfectClearWeight;
    int32_t tSpinWeight;
    int32_t singleWeight;
    int32_t doubleWeight;
    int32_t tripleWeight;
    int32_t highestWeight;
};
