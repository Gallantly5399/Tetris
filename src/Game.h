//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Block.h"
#include "Grid.h"
#include "UI.h"
#include <chrono>
#include "Gravity.h"
#include "Generator.h"

//TODO:: add perfect clear
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
    Block holdBlock;

    //if now is available to hold
    bool isHold = false;

    //1.5 times for difficult score
    bool backToBack = false;


    int comboCount = 0;
    ScoreType addScore();
    bool TSpin() const;
    void restart();
    void draw();
    void insertBlock();
    void stop();
    bool isDifficultScore(const ScoreType& scoreType) const;
};
