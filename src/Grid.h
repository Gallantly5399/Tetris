//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

//column first vector
class Grid {
public:
    Grid(int columnSize, int rowSize);
    Grid() = delete;
    //resize the grid
    void resize(int columnSize, int rowSize);
    //if (row, column) is occupied
    bool isOccupied(int row, int column) const;

    const std::vector<std::vector<int>> & getGrid() const;

    //fill (x, y) with color
    void fill(int row, int column, sf::Color color);
    void clear(int row, int column);
    sf::Color getColor(int row, int column) const;
    //judge if block has exceeded the boundary
    bool exceed();
    void clear();
    //insert the block into the grid
    void insertBlock(const std::vector<std::vector<int>> &shape, sf::Color color, int startRow, int startColumn);
    std::pair<int, int> getScreenPosition(int row, int column, int blockWidth, int stripeWidth, int screenWidth, int screenHeight, int startPosX, int startPosY, bool reverseY = true, bool startFromLeftTop = true) const;
    int getWidth() const;
    int getHeight() const;
    int clearLines();
    int lines() const;
    int bumpiness() const;
    int holes() const;
    int aggregateHeight() const;
private:
    int columnSize, rowSize;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<sf::Color>> colors;

};
