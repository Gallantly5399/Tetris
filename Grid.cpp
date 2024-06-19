//
// Created by amemiya7 on 2024/6/18.
//

#include "Grid.h"
#include <iostream>
bool Grid::isOccupied(int row, int column) {
    if (row < 0 || row >= grid.size() || column < 0 || column >= grid[0].size()) return true;
    return grid[row][column] == 1;
}

Grid::Grid(int width, int height) {
    grid.resize(width);
    for(auto &i : grid) {
        i.resize(height);
        for (int &j : i) j = 0;
//        std::cout << std::endl;
    }
    colors.resize(width);
    for (auto &i : colors) {
        i.resize(height);
        for (auto &j : i) j = glm::vec3(0.0f);
    }
}
void Grid::resize(int width, int height) {
    grid.resize(width);
    for (auto &i : grid) {
        i.resize(height);
        for (int &j : i) j = 0;
    }
    colors.resize(width);
    for (auto &i : colors) {
        i.resize(height);
        for (auto &j : i) j = glm::vec3(0.0f);
    }
}

void Grid::insertBlock(const std::vector<std::vector<int>>& shape, glm::vec3 color, int startRow, int startColumn) {
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
