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
    }

    Movement read() {
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
    AI(double heightWeight, double scoreWeight, double holesWeight, double bumpinessWeight, double emptyLineWeight) {
        this->heightWeight = heightWeight;
        this->scoreWeight = scoreWeight;
        this->holesWeight = holesWeight;
        this->bumpinessWeight = bumpinessWeight;
        this->emptyLineWeight = emptyLineWeight;
    }

    void best(MovementData &movements) {
        while (!isStop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (blockQueue.empty()) continue;
            auto [workingPieces, grid] = blockQueue.front();
            blockQueue.pop();
            auto [bestBlock, bestScore, bestGrid] = this->best_(grid, workingPieces);
            Block temBlock = workingPieces[0];
            if (bestBlock.getType() == BlockType::None) {
                movements.write(Movement::Down);
                continue;
            }
            int rotate_count = 0;
            while (temBlock.getRotation() != bestBlock.getRotation()) {
//                movements.write(Movement::Rotate);
                rotate_count ++;
                temBlock.rotate(grid);
            }
            Movement movement;
            if (rotate_count <= 2) {
                movement = Movement::Rotate;
            } else {
                rotate_count = 4 - rotate_count;
                movement = Movement::RotateCounterClockwise;
            }
            for (int i = 0;i < rotate_count; i++) {
                movements.write(movement);
            }

            auto [aiRow, aiColumn] = bestBlock.getPosition();
            auto [blockRow, blockColumn] = temBlock.getPosition();
            if (aiColumn > blockColumn) {
                for (int i = 0; i < aiColumn - blockColumn; i++) movements.write(Movement::Right);
            } else if (aiColumn < blockColumn) {
                for (int i = 0; i < blockColumn - aiColumn; i++) movements.write(Movement::Left);
            }
            movements.write(Movement::Down);
        }
    }

    void stop() {
        isStop = true;
    }

    void add(std::vector<Block> blocks, Grid grid) {
        blockQueue.emplace(blocks, grid);
    }
    std::tuple<Block, double, Grid> best_(const Grid &grid, const std::vector<Block> &workingPieces) {
        //TODO:: beam search instead of dfs
        std::vector<Candidate> candidates, totalCandidates;
        candidates.emplace_back(std::numeric_limits<double>::lowest(), Block{}, grid);
        for (int workingIndex = 0; workingIndex < 10; workingIndex++) {
            for (auto &[_, blocks, workingGrid]: candidates) {
                //TODO::add soft drop
                for (int rotation = 0; rotation < 4; rotation++) {
                    Block _piece = workingPieces[workingIndex];
                    for (int i = 0; i < rotation; i++) {
                        _piece.rotate(workingGrid);
                    }

                    while (_piece.moveLeft(workingGrid));
                    do {
                        if (isStop) return {candidates[0].blocks[1], candidates[0].score, candidates[0].grid};;
                        Block _pieceSet = _piece;
                        while (_pieceSet.moveDown(workingGrid));

                        Grid _grid = workingGrid;
                        utility::insertBlock(_grid, _pieceSet);

                        double currentScore =
                                -heightWeight * _grid.aggregateHeight() - holesWeight * _grid.holes()
                                - bumpinessWeight * _grid.bumpiness() + emptyLineWeight * _grid.sumOfContinuousEmptyLines();
                        currentScore += scoreWeight * utility::scoreForAI(_grid, _pieceSet);
                        if (_grid.exceed()) currentScore = std::numeric_limits<double>::lowest();
                        auto temBlocks = blocks;
                        temBlocks.push_back(_pieceSet);
                        totalCandidates.emplace_back(currentScore, temBlocks, _grid);
                    } while (_piece.moveRight(workingGrid));
                }
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
        return {candidates[0].blocks[1], candidates[0].score, candidates[0].grid};
    }
private:
    std::queue<std::pair<std::vector<Block>, Grid>> blockQueue;
    bool isStop = false;
    //millisecond time for per block
    double heightWeight;
    double scoreWeight;
    double holesWeight;
    double bumpinessWeight;
    double emptyLineWeight;

    struct Candidate {
        double score;
        std::vector<Block> blocks;
        Grid grid;

        Candidate(double _score, const Block &_block, const Grid &_grid) : score(_score), grid(_grid) {
            blocks.push_back(_block);
        }

        Candidate(double _score, const std::vector<Block> &_blocks, const Grid &_grid) : score(_score), blocks(_blocks),
                                                                                         grid(_grid) {}
    };


};
