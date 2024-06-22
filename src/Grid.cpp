//
// Created by amemiya7 on 2024/6/18.
//

#include "Grid.h"
#include <iostream>
bool Grid::isOccupied(int row, int column) const{
    if (row < 0 || row >= rowSize || column < 0 || column >= columnSize) return true;
    return grid[column][row];
}

//TODO:: Column first vector
Grid::Grid(int columnSize_, int rowSize_): columnSize(columnSize_), rowSize(rowSize_){
    grid.resize(columnSize_);
    for(auto &i : grid) {
        i.resize(rowSize_);
        for (int &j : i) j = 0;
    }
    colors.resize(columnSize_);
    for (auto &i : colors) {
        i.resize(rowSize_);
        for (auto &j : i) j = sf::Color{0x000000};
    }
}

void Grid::resize(int columnSize, int rowSize) {
    this->columnSize = columnSize;
    this->rowSize = rowSize;

    grid.resize(columnSize);
    for(auto &i : grid) {
        i.resize(rowSize);
        for (int &j : i) j = 0;
    }
    colors.resize(columnSize);
    for (auto &i : colors) {
        i.resize(rowSize);
        for (auto &j : i) j = sf::Color{0x000000};
    }
}

void Grid::insertBlock(const std::vector<std::vector<int>>& shape, sf::Color color, int startRow, int startColumn) {
    for (int column = 0; column < shape.size(); column++) {
        for (int row = 0; row < shape[column].size(); row++) {
            if (shape[column][row] == 1) {
                grid[startColumn + column][startRow + row] = 1;
                colors[startColumn + column][startRow + row] = color;
            }
        }
    }
}

bool Grid::exceed() {
    for (int column = 0; column < columnSize; column ++) {
        if (grid[column][rowSize - 2] == 1) return true;
    }
    return false;
}
