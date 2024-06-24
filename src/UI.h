//
// Created by amemiya7 on 2024/6/23.
//

#pragma once
#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Block.h"
enum class ScoreType {
    None = 0,
    Single = 100,
    Double = 300,
    Triple = 500,
    Tetris = 800,
    Combo = 50,
    TSpinMiniNoLines = 100,
    TSpinMiniSingle = 200,
    TSpinMiniDouble = 400,
    TSpinNoLines = 400,
    TSpinSingle = 800,
    TSpinDouble = 1200,
    TSpinTriple = 1600,
    SinglePerfectClear = 800,
    DoublePerfectClear = 1200,
    TriplePerfectClear = 1600,
    TetrisPerfectClear = 2000,
    BackToBackTetrisPerfectClear = 3200,
    SoftDrop = 1,
    HardDrop = 2
};

class UI {
public:
    explicit UI();
    UI(const UI& ui) = delete;
    UI& operator=(const UI& ui) = delete;
    UI(UI&& ui) = delete;
    UI& operator=(UI&& ui) = delete;
    void drawLevel(int level);
    void drawScore(int score);
    void drawLines(int lines);
    void clear();
    void display();
    sf::RenderWindow& getWindow();
    void drawGrid(const Grid &grid);
    void drawBlock(const Block &block, int startPosX, int startPosY);
    void drawBackground();
    //draw 4 lines

    void drawHoldBlock(const Block &block);
    void drawNextBlocks(const std::vector<Block> &nextBlocks);
    int getNextCount() const;
    void drawScoreType(ScoreType scoreType, int comboCount, bool backToBack);
private:
    void drawWindowBackground(int startPosX, int startPosY, int width, int height, bool reverseY = true);
    const int SCREEN_WIDTH = 1200, SCREEN_HEIGHT = 675;
    const int BlockWidth = 30, StripeWidth = 1;
    const int GridStartX = 200, GridStartY = 25;
    const int MainWindowStartX = 200, MainWindowStartY = 25;
    const int NextCount = 5, NextWidth = 155, NextHeight = NextCount * 92, NextWindowStartX = 525, NextWindowStartY = 644 - NextHeight;
    const int HoldWindowStartX = 35, HoldWindowStartY = MainWindowStartY + 594 - 92 + 25, HoldWidth = NextWidth, HoldHeight = 92;
    const int HoldBlockStartX = 60, HoldBlockStartY = MainWindowStartY + 594 - 92;
    const int NextBlockStartX = 550, NextBlockStartY = 625 - 91;
    sf::RenderWindow window;
    sf::Font font;
    sf::Text textLevel;
    sf::Text textScore;
    sf::Text textLines;
    sf::Text textScoreType;
};
