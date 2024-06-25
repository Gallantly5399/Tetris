//
// Created by amemiya7 on 2024/6/24.
//

#pragma once
#include "Grid.h"
#include <random>
#include "Block.h"
#include "Game.h"

class AI {
public:
    AI(double heightWeight, double linesWeight, double holesWeight, double bumpinessWeight) {
        this->heightWeight = heightWeight;
        this->linesWeight = linesWeight;
        this->holesWeight = holesWeight;
        this->bumpinessWeight = bumpinessWeight;
    }
    std::pair<Block, double> best(const Grid& grid, const std::vector<Block>& workingPieces) {
//        for (auto &i : workingPieces) {
//            std::cout << static_cast<int>(i.getType()) << std::endl;
//        }
        return this->best(grid, workingPieces, 0);
    }
private:
    double heightWeight;
    double linesWeight;
    double holesWeight;
    double bumpinessWeight;
    std::pair<Block, double> best(const Grid& grid, const std::vector<Block>& workingPieces, int workingPieceIndex) {
        double bestScore = std::numeric_limits<double>::lowest();
        Block bestBlock{};
        const Block& workingPiece = workingPieces[workingPieceIndex];

        for (int rotation = 0; rotation < 4; rotation++) {
            Block _piece = workingPiece;
            for (int i = 0; i < rotation; i++) {
                _piece.rotate(grid);
            }

            while (_piece.moveLeft(grid)) ;
            do{
                Block _pieceSet = _piece;
                while (_pieceSet.moveDown(grid));

                Grid _grid = grid;
                insertBlock(_grid, _pieceSet);

                double score = 0;
                if (workingPieceIndex == (workingPieces.size() - 1)) {
                    score = -heightWeight * _grid.aggregateHeight() + linesWeight * _grid.lines() -
                            holesWeight * _grid.holes() - bumpinessWeight * _grid.bumpiness();
                } else {
                    score = best(_grid, workingPieces, workingPieceIndex + 1).second;
                }

//                auto [startRow1, startColumn1] = _piece.getPosition();
//                std::cout << "StartRow1:" << startRow1 << " StartColumn1:" << startColumn1 << std::endl;
                if (score > bestScore) {
                    bestScore = score;
                    bestBlock = _piece;
                }
                auto [startRow, startColumn] = _piece.getPosition();
                if (!_piece.isValid(startRow, startColumn + 1, grid)) break;
                _piece.setStartColumn(startColumn + 1);
//                if (!_piece.moveRight(grid)) break;
//                count ++;
//                std::cout << count << std::endl;
            }while(_piece.moveRight(grid));
        }
        return {bestBlock, bestScore};
    }
};
