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
#include <cassert>

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

        Candidate(int64_t _score, const Grid &_grid, const std::vector<Movement> &_movements, const Block& _bestBlock) : score(_score),
                                                                                                grid(_grid), movements(_movements), bestBlock(_bestBlock) {}
//        Candidate(double _score, const std::vector<Block>& _blocks, const Grid &_grid) : score(_score), grid(_grid), blocks(_blocks) {}
    };
    AI(int32_t heightWeight, int32_t scoreWeight, int32_t holesWeight, int32_t bumpinessWeight, int32_t emptyLineWeight,
       int32_t backToBackWeight, int32_t comboWeight, int32_t tetrisWeight, int32_t perfectClearWeight,
       int32_t tSpinWeight, int32_t singleWeight, int32_t doubleWeight, int32_t tripleWeight, int32_t highestWeight,
       int32_t movementWeight) {
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
        this->movementWeight = movementWeight;
    }

    void best(MovementData &movements, MessageData &messageData, std::queue<Block>& checkQueue, Block& gameBestBlock) {
        while (!isStop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (blockQueue.empty()) continue;
            auto lastTime = std::chrono::system_clock::now();
            auto [workingPieces, grid] = blockQueue.front();
            blockQueue.pop();
            const auto& [bestScore, bestMovement, bestGrid, bestBlock] = this->best_(grid, workingPieces);
//            assert(bestBlock.empty() == false);
            gameBestBlock = bestBlock;
//            std::cout << bestScore << '\n';
//        for (auto movement : bestMovement) {
//            std::cout << movement << '\n';
//        }
//
//            std::cout << "AI: ";
//            std::cout << "rotation: " << bestBlock.rotation << ", ";
////            std::cout << "type: " << bestBlock.type << ", ";
//            std::cout << "startRow: " << bestBlock.startRow << ", ";
//            std::cout << "startColumn: " << bestBlock.startColumn << ", ";
////            std::cout << "lastMovement: " << bestBlock.lastMovement << ", ";
////            std::cout << "totalMovements: " << bestBlock.totalMovements << '\n';
//            std::cout << '\n';

            if (isStop) continue;
            auto totalOperations = bestMovement.size();
            double durationTime = 0.;
            messageData.clear();
            for (int i = 0;i < bestMovement.size();) {
                durationTime += std::chrono::duration<double>(lastTime.time_since_epoch()).count(); //seconds
                if (durationTime >= 1.0 / PPS / totalOperations) {
                    durationTime -= 1.0 / PPS / totalOperations;
                    auto movement = bestMovement[i ++];
                    movements.write(movement);
                    lastTime = std::chrono::system_clock::now();
                    if (movement == Movement::softDrop) {
                        //wait until touch ground message
                        while (messageData.empty()) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                        auto message = messageData.read();
                        if (message == Message::BlockTouchGround) {
                            movements.write(Movement::softDropEnd);
                        }
                    }
                }
            }
//            if (bestBlock.empty()) continue;
//            while (checkQueue.empty()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
//            auto gameBlock = checkQueue.front();
//            checkQueue.pop();
//            if (gameBlock != bestBlock) {
//                std::cout << gameBlock.type << ' ' << bestBlock.type << '\n';
//                std::cout << "Game Block is not equal to best block\n";
////                exit(-1);
//            }
////            assert(gameBlock == bestBlock);
        }
    }

    void stop() {
        isStop = true;
    }

    void add(const std::vector<Block> &blocks, const Grid &grid) {
        blockQueue.emplace(blocks, grid);
    }
    Candidate best_(const Grid &grid, const std::vector<Block> &workingPieces) {
        std::vector<Candidate> candidates, totalCandidates;

        candidates.emplace_back(evaluateFirst(grid), grid, std::vector<Movement>{}, Block{});
        for (int workingIndex = 0; workingIndex < 10; workingIndex++) {
            for (const auto &[score, _movements, workingGrid, _bestBlock]: candidates) {
                //TODO::
                const Block &workingPiece = workingPieces[workingIndex];
                Grid temGrid = workingGrid;
                if (temGrid.holdable) {
                    auto movements = _movements;
                    temGrid.holdable = false;
                    Block holdBlock = temGrid.holdBlock;
                    temGrid.holdBlock = workingPiece;
                    if (workingIndex == 0) {
                        movements.clear();
                        movements.push_back(Movement::Hold);
                    }
                    if (holdBlock.empty()) {
                        totalCandidates.push_back(Candidate(score, temGrid, movements, Block{}));
                    } else {
                        search(temGrid, holdBlock, totalCandidates, movements, _bestBlock, false,false);
                    }
                }

                search(workingGrid, workingPiece, totalCandidates, _movements, _bestBlock, workingIndex == 0, workingIndex == 0);
                if (isStop) return candidates[0];
            }
            std::sort(totalCandidates.begin(), totalCandidates.end(), [](const Candidate &a, const Candidate &b) {
                return a.score > b.score;
            });
            candidates.clear();
            for (int i = 0; i < std::min(beamWidth, (uint32_t) totalCandidates.size()); i++) {
                candidates.push_back(totalCandidates[i]);
//                std::cout << totalCandidates[i].score << ' ';
            }
//            std::cout << '\n';
            totalCandidates.clear();
        }
        const auto& bestMovement = candidates[0].movements;
        if (bestMovement[0] != Movement::Hold) {
            Block bestBlock = workingPieces[0];
            for (auto movement : bestMovement) {
                if (movement == Movement::softDrop) {
                    while(utility::moveDown(grid, bestBlock));
                    bestBlock.totalMovements += 10;
                } else {
                    utility::move(grid, bestBlock, movement);
                }
            }
            candidates[0].bestBlock = bestBlock;
        }
        return candidates[0];
    }

    [[nodiscard]] int64_t evaluateFirst(const Grid &grid) const {
        if (grid.exceed()) return std::numeric_limits<int64_t>::lowest();
        ScoreType scoreType = utility::getScoreType(grid, grid.lastBlock);
        int64_t currentScore =
                (int64_t) -heightWeight * grid.aggregateHeight() - holesWeight * grid.holes()
                - bumpinessWeight * grid.bumpiness() + emptyLineWeight * grid.sumOfContinuousEmptyLines()
                + backToBackWeight * grid.backToBack + comboWeight * grid.comboCount +
                tetrisWeight * (scoreType == ScoreType::Tetris)
                + tSpinWeight * utility::isTspin(scoreType) + perfectClearWeight * utility::isPerfectClear(scoreType)
                - singleWeight * (scoreType == ScoreType::Single) - doubleWeight * (scoreType == ScoreType::Double)
                + tripleWeight * (scoreType == ScoreType::Triple)
                - highestWeight * grid.highest()
                - movementWeight * grid.lastBlock.totalMovements;
        return currentScore;
    }

    [[nodiscard]] int64_t evaluate(Grid &grid) const {
        int64_t score = evaluateFirst(grid);
        if (!grid.lastBlock.empty())
            score += scoreWeight * utility::scoreForAI(grid, grid.lastBlock);
        return score;
    }

public:
    ///search for the best movement for the block
    ///block is the block to be inserted
    ///candidates is the vector to store the candidates

    void
    search(const Grid &grid, const Block &block, std::vector<Candidate> &candidates,
           const std::vector<Movement> &bestMovement, const Block& _bestBlock,
           bool recordMovement = false, bool recordBestBlock = false) const {
        for (int rotation = 0; rotation < 4; rotation++) {
            std::vector<Movement> movements;
            Block _piece = block;
            if (rotation <= 2) {
                for (int i = 0; i < rotation; i++) {
                    movements.push_back(Movement::Rotate);
                    utility::rotate(grid, _piece);
                }
            } else {
                for (int i = 0; i < 4 - rotation; i++) {
                    movements.push_back(Movement::RotateCounterClockwise);
                    utility::rotateCounterClockwise(grid, _piece);
                }
            }
//            for (int i = 0; i < rotation; i++) {
//                utility::rotate(grid, _piece);
////                movements.push_back(Movement::Rotate);
//            }

            while (utility::moveLeft(grid, _piece)) {
                movements.push_back(Movement::Left);
            };
            do {
                if (isStop) return;
                Block _pieceSet = _piece;
                while (utility::moveDown(grid, _pieceSet));
                if (_pieceSet.getType() != BlockType::O)
                    handleSoftDrop(grid, _pieceSet, candidates, movements, bestMovement, _bestBlock, recordMovement, recordBestBlock);

                //multiple choice
                //1.harddrop
                //2.left && up is not empty
                //3.right && up is not empty
                //4.rotate && srs

                //harddrop
                Grid _grid = grid;
                utility::insertBlock(_grid, _pieceSet);

                int64_t currentScore = evaluate(_grid);
                auto temMovement = movements;
                temMovement.push_back(Movement::HardDrop);
                auto temBestBlock = _pieceSet;
                if (recordBestBlock) temBestBlock = _bestBlock;
                if (recordMovement) candidates.emplace_back(currentScore, _grid, temMovement, temBestBlock);
                else candidates.emplace_back(currentScore, _grid, bestMovement, temBestBlock);
                if (!movements.empty() && movements.back() == Movement::Left) movements.pop_back();
                else movements.push_back(Movement::Right);
            } while (utility::moveRight(grid, _piece));
        }
    }
    //TODO::FIXME soft drop error
    void handleSoftDrop(const Grid& _grid, const Block& _block, std::vector<Candidate>& candidates, const std::vector<Movement>& _movements, const std::vector<Movement>& bestMovements, const Block& bestBlock,bool recordMovement, bool recordBestBlock) const{
        Block temBlockRotate = _block;
        std::vector<Movement> temMovementRotate = _movements;
        temMovementRotate.push_back(Movement::softDrop);
        temBlockRotate.totalMovements += 10;
        int rotateCount = 3;
        while(move(_grid, temBlockRotate, candidates, temMovementRotate, bestMovements, bestBlock, Movement::Rotate, recordMovement, recordBestBlock) && --rotateCount) {
            temMovementRotate.push_back(Movement::Rotate);
            if (!utility::rotate(_grid, temBlockRotate)) break;
        }

        Block temBlockRotateCounterClockwise = _block;
        std::vector<Movement> temMovementRotateCounterClockwise = _movements;
        temMovementRotateCounterClockwise.push_back(Movement::softDrop);
        int rotateCounterClockwiseCount = 3;
        temBlockRotateCounterClockwise.totalMovements += 10;

        while(move(_grid, temBlockRotateCounterClockwise, candidates,  temMovementRotateCounterClockwise, bestMovements, bestBlock,
                   Movement::RotateCounterClockwise, recordMovement, recordBestBlock) && -- rotateCounterClockwiseCount) {
            temMovementRotateCounterClockwise.push_back(Movement::RotateCounterClockwise);
            utility::rotateCounterClockwise(_grid, temBlockRotateCounterClockwise);
        };
    }
    bool move(const Grid &_grid, const Block &_block, std::vector<Candidate> &candidates,
              const std::vector<Movement> &_movements, const std::vector<Movement>& bestMovements, const Block& bestBlock,Movement movement, bool recordMovement, bool recordBestBlock) const {
        if (_block.empty()) return false;
        Block block = _block;
        Grid grid = _grid;
        if (!utility::move(grid, block, movement)) {
            return false;
        }
        if (!utility::touch(grid, block)) {
            return false;
        }
//        if ((movement == Movement::Rotate || movement == Movement::RotateCounterClockwise) && !block.srs) return Block{};
        std::vector<Movement> movements = _movements;
        movements.push_back(movement);
        utility::insertBlock(grid, block);
        int64_t currentScore = evaluate(grid);
        auto temBestBlock = block;
        if (recordBestBlock) temBestBlock = bestBlock;
        if (recordMovement) { candidates.emplace_back(currentScore, grid, movements, temBestBlock); }
        else candidates.emplace_back(currentScore, grid, bestMovements, temBestBlock);
        return true;
    }
    //1.harddrop
    //2.left && up is not empty
    //3.right && up is not empty
    //4.rotate && srs

    std::queue<std::pair<std::vector<Block>, Grid>> blockQueue;
    bool isStop = false;
    const uint32_t beamWidth = 10;
    uint32_t PPS = 1;
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
    int32_t movementWeight;
};
