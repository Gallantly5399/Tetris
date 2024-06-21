//
// Created by amemiya7 on 2024/6/18.
//

#include "Grid.h"
#include <iostream>
bool Grid::isOccupied(int row, int column) const{
    if (row < 0 || row >= grid.size() || column < 0 || column >= grid[0].size()) return true;
    return grid[row][column];
}

Grid::Grid(int width, int height): width(width), height(height){
    grid.resize(height);
    for(auto &i : grid) {
        i.resize(width);
        for (int &j : i) j = 0;
    }
    colors.resize(height);
    for (auto &i : colors) {
        i.resize(width);
        for (auto &j : i) j = sf::Color{0x000000};
    }
}

void Grid::resize(int width, int height) {
    this->width = width;
    this->height = height;

    grid.resize(width);
    for (auto &i : grid) {
        i.resize(height);
        for (int &j : i) j = 0;
    }
    colors.resize(width);
    for (auto &i : colors) {
        i.resize(height);
        for (auto &j : i) j = sf::Color{0x000000};
    }
}

void Grid::insertBlock(const std::vector<std::vector<int>>& shape, sf::Color color, int startRow, int startColumn) {
    for (int i = 0; i < shape.size(); i++) {
        for (int j = 0; j < shape[i].size(); j++) {
            if (shape[i][j] == 1) {
                grid[startRow + i][startColumn + j] = 1;
                colors[startRow + i][startColumn + j] = color;
            }
        }
    }
}

bool Grid::exceed() {
    for (int i = 0;i < grid[0].size(); i ++) {
        if (grid[grid.size() - 2][i] == 1) return true;
    }
    return false;
}
