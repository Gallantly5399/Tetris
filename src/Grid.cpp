//
// Created by amemiya7 on 2024/6/18.
//

#include "Grid.h"
#include <iostream>
bool Grid::isOccupied(int row, int column) const{
    if (row < 0 || row >= rowSize || column < 0 || column >= columnSize) return true;
    return grid[column][row];
}

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

int Grid::getWidth() const {
    return columnSize;
}

int Grid::getHeight() const {
    return rowSize;
}

int Grid::clearLines() {
    int lines = 0;
    for (int row = 0; row < rowSize - 1; row ++) {
        for (int column = 0; column < columnSize; column ++) {
            if (grid[column][row] == 0) {
                break;
            }
            if (column == columnSize - 1) {
                for (int i = row; i < rowSize - 1; i ++) {
                    for (int j = 0; j < columnSize; j ++) {
                        grid[j][i] = grid[j][i + 1];
                        colors[j][i] = colors[j][i + 1];
                    }
                }
                for (int i = 0; i < columnSize; i ++) {
                    grid[i][rowSize - 1] = 0;
                    colors[i][rowSize - 1] = sf::Color{0x000000};
                }
                lines++;
                row --;
            }
        }
    }
    return lines;
}

std::pair<int, int>
Grid::getScreenPosition(int row, int column, int blockWidth, int stripeWidth, int screenWidth, int screenHeight,
                        int startPosX, int startPosY, bool reverseY, bool startFromLeftTop) const {
    int posX = startPosX + column * (blockWidth + stripeWidth);
    int posY = startPosY + row  * (blockWidth + stripeWidth);
    if (startFromLeftTop) {
        posY += blockWidth + stripeWidth;
    }
    if (reverseY) {
        posY = screenHeight - posY;
    }
    return {posX, posY};
}

sf::Color Grid::getColor(int row, int column) const {
    return colors[column][row];
}

void Grid::fill(int row, int column, sf::Color color) {
    if (row < 0 || row > rowSize || column < 0 || column >= columnSize) return;
//    std::cout << "fill" << std::endl;
    grid[column][row] = 1;
    colors[column][row] = color;
}

const std::vector<std::vector<int>> &Grid::getGrid() const {
    return grid;
}

void Grid::clear(int row, int column) {
    if (row < 0 || row > rowSize || column < 0 || column >= columnSize) return;
    grid[column][row] = 0;
    colors[column][row] = sf::Color{0x000000};
}

void Grid::clear() {
    for (auto &i : grid) {
        for (int &j : i) j = 0;
    }
    for (auto &i : colors) {
        for (auto &j : i) j = sf::Color{0x000000};
    }
}
