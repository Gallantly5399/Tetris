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
#include <filesystem>
//FIXME::AI repress error for the first block
//FIXME::Gravity error
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
private:
    //column row
    std::pair<int, int> mousePositionToGridPosition(float x, float y);
    void processEvents();
    bool isTouchedGround = false;
    bool isRunning = true;
    bool isLockDelay = true;
    double lockDelayTime = 0;
    double time = 0;
    bool isAiActive = false;
    bool isHardDrop = false;
    std::thread aiThread;
    sf::Clock clock;
    Block block;
    Grid grid;
    UI ui;
    Generator generator;
    Gravity gravity;
    int score = 0;
    int currentLogicFrame = 0;
    //1.5 times for difficult score
    bool backToBack = false;
    Movement aiLastMovement = Movement::None;
    AI ai;
    int comboCount = 0;
    ScoreType lastScoreType = ScoreType::None;
    ScoreType addScore();
    bool TSpin() const;
    void restart();
    void draw();
    void insertBlock();
    void stop();
    static bool isDifficultScore(const ScoreType& scoreType);
    void parseConfig();
    void sendBlocks();
    MovementData aiMovement;
    bool firstBlock = false;
    long long aiLastMoveTime = 0;
//    std::thread aiThread;
    //TODO::read from config file
    uint32_t MAX_LOGIC_FRAMES = 20;
    uint32_t MAX_RENDER_FRAMES = 60;
    uint32_t MAX_AI_MOVEMENTS_PER_SECOND = 10;
    uint32_t HORIZONTAL_MOVEMENT = 0;
    uint32_t MOVEMENT_SENSITIVITY = 2;
    uint32_t movementCount = 0;
    uint32_t logicFrameCount = 0;
    uint32_t renderFrameCount = 0;
    std::filesystem::path projectPath = FILE_LOCATION;
};
