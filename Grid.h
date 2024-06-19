//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>

class Grid {
public:
    Grid(int width, int height);
    Grid() {};
    void resize(int width, int height);
    bool isOccupied(int row, int column);
    const std::vector<std::vector<int>> & getGrid() {return grid;}
    void fill(int x,int y) {
        grid[x][y] = 1;
    }
private:
    std::vector<std::vector<int>> grid;

    void insertBlock(const std::vector<std::vector<int>> &shape, int startRow, int startColumn);
};
