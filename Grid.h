//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include "glm/glm.hpp"

class Grid {
public:
    Grid(int width, int height);
    Grid() {};
    //resize the grid
    void resize(int width, int height);
    //if (row, column) is occupied
    bool isOccupied(int row, int column);

    const std::vector<std::vector<int>> & getGrid() {return grid;}

    //fill (x, y) with color
    void fill(int x,int y, glm::vec3 color) {
        grid[x][y] = 1;
        colors[x][y] = color;
    }

    glm::vec3 getColor(int row, int column) {
        return colors[row][column];
    }
    //judge if block has exceeded the boundary
    bool exceed();

    //insert the block into the grid
    void insertBlock(const std::vector<std::vector<int>> &shape, glm::vec3 color, int startRow, int startColumn);
private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<glm::vec3>> colors;


};
