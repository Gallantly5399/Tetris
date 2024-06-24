//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include "Block.h"
#include "Grid.h"
#include "UI.h"
#include <chrono>
#include "Gravity.h"
#include "Generator.h"

class Game {
public:
    //delete copy constructor and move constructor
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;
    void tick();

    Game();
    void hold();
    bool shouldStop();
    bool shouldClose();

    void run();
    void close();
private:
    //column row
    std::pair<int, int> mousePositionToGridPosition(float x, float y);
    void processEvents();
    bool isTouchedGround = false;
    int movement = 0;
    bool isRunning = true;
    bool isLockDelay = true;
    double lockDelayTime = 0;
    double time = 0;

    bool isHardDrop = false;
    sf::Clock clock;
    Block block;
    Grid grid;
    UI ui;
    Generator generator;
    Gravity gravity;
    int score = 0;
    Block holdBlock = Block();

    //if now is available to hold
    bool isHold = false;

    //1.5 times for difficult score
    bool backToBack = false;


    int comboCount = 0;
    ScoreType lastScoreType = ScoreType::None;
    ScoreType addScore();
    bool TSpin() const;
    void restart();
    void draw();
    void insertBlock();
    void stop();
    bool isDifficultScore(const ScoreType& scoreType) const;
};
