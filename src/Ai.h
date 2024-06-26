//
// Created by amemiya7 on 2024/6/24.
//

#pragma once
#include "Grid.h"
#include <random>
#include "Block.h"
static void insertBlock(Grid& grid, const Block& block) {
    const auto& shape = block.getShape();
    const auto& color = block.getColor();
    int startRow = block.getStartRow();
    int startColumn = block.getStartColumn();
    for (int column = 0; column < shape.size(); column ++) {
        for (int row = 0; row < shape.size(); row ++) {
            if (shape[column][row] == 1) {
                grid.fill(startRow + row, startColumn + column, color);
            }
        }
    }
}
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
                Block _pieceSet = _piece;
                while (_pieceSet.moveDown(grid));

                Grid _grid = grid;
                insertBlock(_grid, _pieceSet);

                double score = 0;
                if (workingPieceIndex == (workingPieces.size() - 1)) {
                    score = -heightWeight * _grid.aggregateHeight() + linesWeight * _grid.lines() -
                            holesWeight * _grid.holes() - bumpinessWeight * _grid.bumpiness();
//                    std::cout << "Rotation: " << rotation << "Count: " << count << ", score: " << score << std::endl;
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
//                if (!_piece.isValid(startRow, startColumn + 1, grid)) break;
//                _piece.setStartColumn(startColumn + 1);
                count++;
            }while(_piece.moveRight(grid));
        }
        return {bestBlock, bestScore};
    }
};
