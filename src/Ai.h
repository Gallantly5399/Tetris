//
// Created by amemiya7 on 2024/6/24.
//

#pragma once
#include "Grid.h"
#include <random>
#include "Block.h"
#include <queue>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "Utility.h"

struct MovementData{
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
    [[nodiscard]] bool empty() const{
        return readIndex == writeIndex;
    }
};


class AI {
public:
    AI(double heightWeight, double linesWeight, double holesWeight, double bumpinessWeight) {
        this->heightWeight = heightWeight;
        this->linesWeight = linesWeight;
        this->holesWeight = holesWeight;
        this->bumpinessWeight = bumpinessWeight;
    }
    void best(MovementData& movements) {
        while(!isStop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (blockQueue.empty()) continue;
            auto [workingPieces, grid] = blockQueue.front();
            blockQueue.pop();
            Block bestBlock = this->best_(grid, workingPieces, 0).first;
            Block temBlock = workingPieces[0];
            if (bestBlock.getType() == BlockType::None) {
                movements.write(Movement::Down);
                return;
            }
            while (temBlock.getRotation() != bestBlock.getRotation()) {
                movements.write(Movement::Rotate);
                temBlock.rotate(grid);
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
private:
    std::queue<std::pair<std::vector<Block>, Grid>> blockQueue;
    bool isStop = false;
    //millisecond time for per block
    double heightWeight;
    double linesWeight;
    double holesWeight;
    double bumpinessWeight;
    std::pair<Block, double> best_(const Grid& grid, const std::vector<Block>& workingPieces, int workingPieceIndex) {
        //TODO:: beam search instead of dfs
        double bestScore = std::numeric_limits<double>::lowest();
        Block bestBlock(workingPieces[workingPieceIndex].getType());
        const Block& workingPiece = workingPieces[workingPieceIndex];

        for (int rotation = 0; rotation < 4; rotation++) {
            Block _piece = workingPiece;
            for (int i = 0; i < rotation; i++) {
                _piece.rotate(grid);
            }

            while (_piece.moveLeft(grid)) ;
            int count = 0;
            do{
                if (isStop) return {bestBlock, bestScore};
                Block _pieceSet = _piece;
                while (_pieceSet.moveDown(grid));

                Grid _grid = grid;
                insertBlock(_grid, _pieceSet);

                double score = 0;
                if (workingPieceIndex == (workingPieces.size() - 1)) {
                    score = -heightWeight * _grid.aggregateHeight() + linesWeight * _grid.lines() -
                            holesWeight * _grid.holes() - bumpinessWeight * _grid.bumpiness();
                } else {
                    score = best_(_grid, workingPieces, workingPieceIndex + 1).second;
                }

                if (score > bestScore) {
                    bestScore = score;
                    bestBlock = _piece;
                }
                count++;
            }while(_piece.moveRight(grid));
        }
        return {bestBlock, bestScore};
    }
};
