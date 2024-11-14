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
#include "Ai.h"
#include "Generator.h"
#include <thread>


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
    ~Game();
    void run();
    void close();
    int scoreTypeToInt(ScoreType scoreType);
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
    bool isAiActive = false;

    bool isHardDrop = false;
    sf::Clock clock;
    Block block;
    Grid grid;
    UI ui;
    Generator generator;
    Gravity gravity;
    int score = 0;
    Block holdBlock = Block();
    int currentLogicFrame = 0;

    //if now is available to hold
    bool isHold = false;

    //1.5 times for difficult score
    bool backToBack = false;

    AI ai;
    std::mutex mutex;
    std::condition_variable cv;
    int comboCount = 0;
    ScoreType lastScoreType = ScoreType::None;
    ScoreType addScore();
    bool TSpin() const;
    void restart();
    void draw();
    void insertBlock();
    void stop();
    bool isDifficultScore(const ScoreType& scoreType) const;
    std::queue<Movement> simulateMovement(const Block& aiBlock);
    MovementData aiMovement;
    bool firstBlock = false;
    sf::Clock aiClock;
    long long aiLastMoveTime = 0;
    std::thread aiThread;
    const uint32_t MAX_LOGIC_FRAMES = 20;
    const uint32_t MAX_RENDER_FRAMES = 60;
    const uint32_t MAX_AI_MOVEMENTS_PER_SECOND = 10;

    uint32_t logicFrameCount = 0;
    uint32_t renderFrameCount = 0;
};
