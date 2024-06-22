//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

class Grid {
public:
    Grid(int columnSize, int rowSize);
    Grid() {};
    //resize the grid
    void resize(int columnSize, int rowSize);
    //if (row, column) is occupied
    bool isOccupied(int row, int column) const;

    const std::vector<std::vector<int>> & getGrid() const{return grid;}

    //fill (x, y) with color
    void fill(int row,int column, sf::Color color) {
        grid[column][row] = 1;
        colors[column][row] = color;
    }

    sf::Color getColor(int row, int column) const{
        return colors[column][row];
    }
    //judge if block has exceeded the boundary
    bool exceed();

    //insert the block into the grid

    //FIXME:: maybe something wrong
    void insertBlock(const std::vector<std::vector<int>> &shape, sf::Color color, int startRow, int startColumn);
    std::pair<int, int> getScreenPosition(int row, int column, int blockWidth, int stripeWidth, int screenWidth, int screenHeight, int startPosX, int startPosY, bool reverseY = true, bool startFromLeftTop = true) const{
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
    int getWidth() const {
        return columnSize;
    }
    int getHeight() const {
        return rowSize;
    }
    int clearLines() {
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

private:
    int columnSize, rowSize;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<sf::Color>> colors;

};
