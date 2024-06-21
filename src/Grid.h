//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

class Grid {
public:
    Grid(int width, int height);
    Grid() {};
    //resize the grid
    void resize(int width, int height);
    //if (row, column) is occupied
    bool isOccupied(int row, int column) const;

    const std::vector<std::vector<int>> & getGrid() const{return grid;}

    //fill (x, y) with color
    void fill(int row,int column, sf::Color color) {
        grid[row][column] = 1;
        colors[row][column] = color;
    }

    sf::Color getColor(int row, int column) const{
        return colors[row][column];
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
        return width;
    }
    int getHeight() const {
        return height;
    }
    int clearLines() {
        int lines = 0;
        for (int row = 0; row < height - 1; row ++) {
            for (int column = 0; column < width; column ++) {
                if (grid[row][column] == 0) {
                    break;
                }
                if (column == width - 1) {
                    for (int i = row; i < height - 1; i ++) {
                        for (int j = 0; j < width; j ++) {
                            grid[i][j] = grid[i + 1][j];
                            colors[i][j] = colors[i + 1][j];
                        }
                    }
                    for (int i = 0; i < width; i ++) {
                        grid[height - 1][i] = 0;
                        colors[height - 1][i] = sf::Color{0x000000};
                    }
                    lines++;
                    row --;
                }
            }
        }
        return lines;
    }

private:
    int width, height;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<sf::Color>> colors;

};
