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

enum class scoreType {
    Single = 100,
    Double = 300,
    Tripe = 500,
    Tetris = 800,
    SoftDrop = 1,
    HardDrop = 2,
    Combo = 50,
    TSpinSingle = 800,
    TSpinDouble = 1200,
    TSpinTriple = 1600,
    SinglePerfectClear = 800,
    DoublePerfectClear = 1200,
    TriplePerfectClear = 1600,
    TetrisPerfectClear = 2000,
};
class Game {
public:
    //every tick render mainWindow and nextWindow and the moving block
    //if timer > fall time then block move down
    double frameTime = 1.0 / 60 * 1000;
    bool firstDraw = false;

    void tick();

    Game() : block(BlockType::O), grid(10, 22), ui(), generator(), gravity() {
        block = generator.nextBlock();
    }
    void insertGridBlock(const Block& block) {
        const auto& shape = block.getShape();
        const auto& color = block.getColor();
        grid.insertBlock(shape, color, block.getPosition().startRow, block.getPosition().startColumn);
    }

    void hold();
    Game(unsigned int windowWidth, unsigned int windowHeight,int gameWidth, int gameHeight):
    block(BlockType::O), grid(gameWidth, gameHeight) {

    }
    Block getHoldBlock() const;
    void insertBlock();
    const unsigned int Height = 10, Width = 20;
    const unsigned int NextWidth = 3, NextHeight = 6, NextCount = 1;
    sf::RenderWindow& getWindow() {
        return ui.getWindow();
    }
    Gravity& getGravity() {
        return gravity;
    }
    Grid& getGrid() {
        return grid;
    }
    Block& getBlock() {
        return block;
    }

    Generator& getGenerator() {
        return generator;
    }
    void stop() {
        isRunning = false;
    }
    bool shouldStop() {
        return !isRunning;
    }

    bool shouldClose() {
        return !ui.getWindow().isOpen();
    }
private:
    //column row
    std::pair<int, int> mousePositionToGridPosition(float x, float y);
    void processEvents();
    bool isTouchedGround = false;
    int movement = 0;
    bool isRunning = true;
    //input the cube data into the render
    enum class BlockMovement {
        left,
        right,
        down,
        rotate
    };
    static BlockMovement blockMovement;
    unsigned int mainWindowWidth, mainWindowHeight;
    unsigned int nextWindowWidth, nextWindowHeight;
//    const unsigned int mainWindowWidth = 1200, mainWindowHeight = 675;
//    const unsigned int nextWindowWidth = 400 , nextWindowHeight = 800;
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
    //map two dimenstions to one dimension
    int index(int x, int y) {
        return x + y * Width;
    }
    Block holdBlock;
    bool isHold = false;
};
