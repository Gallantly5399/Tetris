//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "Block.h"
//column first vector
class Grid {
public:

    bool backToBack = false;
    bool holdable = true;
    Block holdBlock = Block();
    Block lastBlock = Block();
    int comboCount = 0;

    Grid(int columnSize, int rowSize);
    Grid() = delete;
    //copy constructor
    Grid(const Grid &grid) = default;
    //copy assignment
    Grid &operator=(const Grid &grid) = default;
    //move constructor
    Grid(Grid &&grid) noexcept = default;
    //move assignment
    Grid &operator=(Grid &&grid) noexcept = default;
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
    bool exceed() const;
    void clear();
    bool empty() const;
    //insert the block into the grid
    void insertBlock(const std::vector<std::vector<int>> &shape, sf::Color color, int startRow, int startColumn);
    std::pair<int, int> getScreenPosition(int row, int column, int blockWidth, int stripeWidth, int screenWidth, int screenHeight, int startPosX, int startPosY, bool reverseY = true, bool startFromLeftTop = true) const;
    int getWidth() const;
    int getHeight() const;
    int clearLines();
    //the number of the lines which could be cleared in the grid
    int lines() const;
    //the sum of the difference between the height of each column and the minimum height
    int bumpiness() const;
    //the number of the empty blocks which have at least one block above them
    int holes() const;

    //the sum of the continuous empty lines
    int sumOfContinuousEmptyLines() const;
    int highest() const;
    int aggregateHeight() const;
    bool hold(Block& block);

private:
    int columnSize, rowSize;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<sf::Color>> colors;


};
